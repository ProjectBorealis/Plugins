// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceAsset.cpp

#include "SubstanceSourceAsset.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceSourcePanel.h"
#include "SubstanceSourceStyle.h"
#include "ObjectTools.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"
#include "HAL/FileManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SFxWidget.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Text/STextBlock.h"

#include <substance/source/attachment.h>


#define MAX_CACHE_DURATION_DAYS 30

/** Init constants */
const float SSubstanceSourceAsset::FullVisible = 0.0f;
const float SSubstanceSourceAsset::FullHidden = 0.51f;
const float SSubstanceSourceAsset::SlideSpeed = 2.5f;

/** SCompound widget override - Called when ever this widget is instantiated */
void SSubstanceSourceAsset::Construct(const FArguments& InArgs)
{
	//Bind Callbacks
	OnModuleShutdownDelegateHandle = ISubstanceSourceModule::Get()->OnModuleShutdown().AddSP(this, &SSubstanceSourceAsset::OnModuleShutdown);
	OnAssetPurchasedDelegateHandle = ISubstanceSourceModule::Get()->OnSubstancePurchased().AddSP(this, &SSubstanceSourceAsset::OnSubstancePurchased);
	LoadCachedThumbnailEvent.AddSP(this, &SSubstanceSourceAsset::LoadThumbnailFromCache);

	//Set args from slate
	Label = InArgs._Label;
	mAsset = InArgs._Asset;
	mParentWindow = InArgs._ParentWindow;

	//Default to a non loaded state
	ThumbnailLoaded = false;

	//Generate the tiles - automagically generates based on state
	RegenerateTileLayout();

	//Default the offset to off screen
	DownloadOverlayOffset = FullHidden;

	//Bind the accessor for our vector
	DownloadOverlayOffsetAttribute.Set(FVector2D(FullHidden, FullHidden));
	DownloadOverlayOffsetAttribute.Bind(this, &SSubstanceSourceAsset::GetDownloadOverlayOffeset);

	//Default overlay update to off
	IsDownloadOverlayInView = false;
	PrioritizedLoad = false;

	//Initialize pointers
	TextureResource = nullptr;

	//As caching is much quicker, we can queue these outside of on paint. #NOTE::EVIL const cast but needed
	if (CanLoadThumbnailFromCache())
	{
		ISubstanceSourceModule::Get()->QueueLoadThumbnailCache(this->AsShared());
	}
}

SSubstanceSourceAsset::~SSubstanceSourceAsset()
{
	if (TextureResource && TextureResource->IsValidLowLevel())
	{
		TextureResource->ClearFlags(RF_Standalone);
	}
}

/** This should only be called when the asset is in view! Handled by UE4 */
int32 SSubstanceSourceAsset::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//Kick off the load if we don't have the thumbnail
	if (!ThumbnailLoaded)
	{
		//EVIL const casts but needed
		if (!CanLoadThumbnailFromCache())
		{
			ISubstanceSourceModule::Get()->LoadThumbnail(ConstCastSharedRef<SWidget>(this->AsShared()));
		}
	}

	//If we are in the Queue and it is in view, TO THE TOP!
	if (!PrioritizedLoad && !ThumbnailLoaded && !ThumbnailBrush.IsValid())
	{
		ISubstanceSourceModule::Get()->PrioritizeThumbnailCacheLoading(ConstCastSharedRef<SWidget>(this->AsShared()));
	}

	//Finalize by calling the parent on paint
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

/** Callback that creates the thumbnail, adds it to the viewport and removes the loading widget */
void SSubstanceSourceAsset::ConstructThumbnail()
{
	//Create the brush
	ThumbnailBrush = ISubstanceSourceModule::Get()->ConstructImageBrush(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL, mAsset);

	if (ThumbnailBrush.IsValid() && ThumbnailBrush->GetResourceObject())
	{
		TextureResource = ThumbnailBrush->GetResourceObject();
	}

	//Regenerates now that the thumbnail brush is valid
	RegenerateTileLayout();
}

/** Flags this class to no longer request the HTTP image data */
void SSubstanceSourceAsset::SetThumbnailLoaded(bool loaded)
{
	ThumbnailLoaded = loaded;
}

void SSubstanceSourceAsset::SetPrioritizedCache()
{
	PrioritizedLoad = true;
}

/** Clears the layout and creates all of the child widgets of the tile */
void SSubstanceSourceAsset::RegenerateTileLayout()
{
	//Clear all widgets from the root
	ChildSlot.DetachWidget();

	//Store the widgets according to the state
	TSharedPtr<SWidget> View       = SNullWidget::NullWidget;
	TSharedPtr<SWidget> Loading    = SNullWidget::NullWidget;
	TSharedPtr<SWidget> NewAsset   = SNullWidget::NullWidget;
	TSharedPtr<SWidget> OwnedAsset = SNullWidget::NullWidget;
	TSharedPtr<SWidget> Download   = SNullWidget::NullWidget;
	TSharedPtr<SWidget> FreeAsset  = SNullWidget::NullWidget;
	TSharedPtr<SWidget> Updated    = SNullWidget::NullWidget;

	bool IsAssetInNewAssetCategory = false;

	Alg::Source::Database* database = ISubstanceSourceModule::Get()->GetDatabase();

	if (!database)
	{
		//Log error that the database could not be found
		UE_LOG(LogSubstanceSource, Error, TEXT("The source database is not valid. Could not populate asset tiles! "))
		return;
	}

	//Fix up category name if all assets or new assets
	const Alg::Source::String& SearchName = Alg::Source::Database::sCategoryAllAssets;
	const Alg::Source::CategoryAssetVector& categoryAssets = database->getCategoryAssetVector();
	auto iter = categoryAssets.begin();

	while (iter != categoryAssets.end())
	{
		if (iter->first == SearchName)
		{
			break;
		}

		iter++;
	}

	if (iter == categoryAssets.end())
	{
		//Log that the category name could not be found!
		UE_LOG(LogSubstanceSource, Error, TEXT("Searched for a category name that could is invalid, no assets found! "))
		check(0);
		return;
	}

	for (auto AssetItr = iter->second.rbegin(); AssetItr != iter->second.rend(); ++AssetItr)
	{
		if (*AssetItr == mAsset)
		{
			IsAssetInNewAssetCategory = true;
			break;
		}
	}

	//Core tile elements
	if (ThumbnailBrush.IsValid())
	{
		View =
		    SNew(SImage)
		    .Image(ThumbnailBrush.Get());

		Download =
		    SNew(SFxWidget)
		    .Clipping(EWidgetClipping::ClipToBoundsAlways)
		    .VisualOffset(DownloadOverlayOffsetAttribute)
		    [
		        SNew(SImage)
		        .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceDownloadIconOverlay"))
		        .OnMouseButtonDown(this, &SSubstanceSourceAsset::OnDownloadOverlayClicked)
		    ];
	}
	else
	{
		Alg::Source::ImageAttachment* image = mAsset->getImageAttachmentByLabel(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL);

		View =
		    SNew(SColorBlock)
		    .Color(FLinearColor(0.2f, 0.2f, 0.2f))
		    .Size(FVector2D(image->getDesiredImageWidth(), image->getDesiredImageHeight()));

		Loading =
		    SNew(SCircularThrobber)
		    .IsEnabled(true)
		    .NumPieces(6)
		    .Period(0.75f)
		    .Radius(16.f);
	}

	//Set the overlay if the asset is owned by the logged in user (if no user -> this will default false)
	//else if the asset is new, add that. (Shouldn't have both tags to reflect website)
	if (ISubstanceSourceModule::Get()->IsAssetOwned(mAsset))
	{
		OwnedAsset =
		    SNew(SImage)
		    .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceOwnedTileIconOverlay"));
	}
	else if (mAsset->isNewAsset())
	{
		NewAsset =
		    SNew(SImage)
		    .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceNewTileIconOverlay"));
	}
	else if (mAsset->getCost() == 0 || mAsset->getIsPromotionallyFree())
	{
		FreeAsset =
		    SNew(SImage)
		    .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceFreeAssetTileIconOverlay"));
	}

	//Set the updated text in the case the sbsar was recently updated
	if (mAsset->isRecentlyUpdatedAsset() && !mAsset->isNewAsset())
	{
		Updated =
		    SNew(STextBlock)
		    .Text(FText::FromString("Updated"))
		    .WrapTextAt(300)
		    .Justification(ETextJustify::Right)
		    .TextStyle(FSubstanceSourceStyle::Get(), "UpdateLabelText");
	}

	//Clear the child slot
	this->ChildSlot
	[
	    SNullWidget::NullWidget
	];

	//Create the layout
	TSharedPtr<SVerticalBox> BaseWidget =
	    SNew(SVerticalBox)

	    //Add the image of the asset
	    + SVerticalBox::Slot()
	    .HAlign(HAlign_Fill)
	    .VAlign(VAlign_Fill)
	    .FillHeight(0.9f)
	    .Padding(FMargin(0.0f, 3.0f, 0.0f, 0.0f))
	    [
	        SNew(SScaleBox)
	        .Stretch(EStretch::ScaleToFit)
	        [
	            //Create an overlay for layering widgets
	            SNew(SOverlay)

	            //Add a slot for the main view
	            + SOverlay::Slot()
	            .HAlign(HAlign_Fill)
	            .VAlign(VAlign_Fill)
	            [
	                SNew(SScaleBox)
	                .HAlign(HAlign_Fill)
	                .VAlign(VAlign_Fill)
	                [
	                    View->AsShared()
	                ]
	            ]

	            //Add a slot for the loading icon
	            + SOverlay::Slot()
	            .HAlign(HAlign_Center)
	            .VAlign(VAlign_Center)
	            [
	                Loading->AsShared()
	            ]

	            //Add a slot for the details panel button
	            + SOverlay::Slot()
	            [
	                SNew(SButton)
	                .OnClicked(this, &SSubstanceSourceAsset::OnOpenDetailsPanel)
	                .ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
	            ]

	            //Add a slot for the icon label overlay for new assets
	            + SOverlay::Slot()
	            .HAlign(HAlign_Right)
	            .VAlign(VAlign_Top)
	            [
	                SNew(SScaleBox)
	                .HAlign(HAlign_Right)
	                .VAlign(VAlign_Top)
	                .Visibility(EVisibility::HitTestInvisible)
	                .Stretch(EStretch::ScaleToFit)
	                [
	                    NewAsset->AsShared()
	                ]
	            ]

	            //Add a slot for the icon overlay for owned assets
	            + SOverlay::Slot()
	            .HAlign(HAlign_Left)
	            .VAlign(VAlign_Top)
	            [
	                SNew(SScaleBox)
	                .HAlign(HAlign_Left)
	                .VAlign(VAlign_Top)
	                .Visibility(EVisibility::HitTestInvisible)
	                .Stretch(EStretch::ScaleToFit)
	                [
	                    OwnedAsset->AsShared()
	                ]
	            ]

	            //Add a slot for the icon overlay for free assets
	            + SOverlay::Slot()
	            .HAlign(HAlign_Left)
	            .VAlign(VAlign_Bottom)
	            [
	                SNew(SScaleBox)
	                .HAlign(HAlign_Left)
	                .VAlign(VAlign_Bottom)
	                .Visibility(EVisibility::HitTestInvisible)
	                .Stretch(EStretch::ScaleToFit)
	                [
	                    FreeAsset->AsShared()
	                ]
	            ]

	            //Test Download Slide in Overlay
	            + SOverlay::Slot()
	            .HAlign(HAlign_Right)
	            .VAlign(VAlign_Bottom)
	            [
	                SNew(SScaleBox)
	                .HAlign(HAlign_Right)
	                .VAlign(VAlign_Bottom)
	                .Stretch(EStretch::ScaleToFit)
	                [
	                    Download->AsShared()
	                ]
	            ]
	        ]
	    ]

	    + SVerticalBox::Slot()
	    .AutoHeight()
	    [
	        SNew(SHorizontalBox)
	        //Slot for the substance tile title
	        + SHorizontalBox::Slot()
	        .HAlign(HAlign_Left)
	        .AutoWidth()
	        .Padding(FMargin(8.0f, 11.0f, 3.0f, 0.0f))
	        [
	            SNew(STextBlock)
	            .Text(FText::FromString(GetLabel()))
	            .WrapTextAt(300)
	            .Justification(ETextJustify::Left)
	            .TextStyle(FSubstanceSourceStyle::Get(), "SourceTileText")
	        ]

	        //Slot for the updated tag
	        + SHorizontalBox::Slot()
	        .HAlign(HAlign_Right)
	        .Padding(FMargin(0.0f, 11.0f, 3.0f, 0.0f))
	        [
	            Updated->AsShared()
	        ]
	    ];

	//Add to the root
	ChildSlot
	[
	    BaseWidget->AsShared()
	];
}

/** Accessor for passing the asset associated with this widget to the factory */
Alg::Source::AssetPtr SSubstanceSourceAsset::GetAsset() const
{
	return mAsset;
}

FVector2D SSubstanceSourceAsset::GetDownloadOverlayOffeset() const
{
	return FVector2D(DownloadOverlayOffset, DownloadOverlayOffset);
}

FReply SSubstanceSourceAsset::OnOpenDetailsPanel()
{
	if (mParentWindow.IsValid())
	{
		mParentWindow.Pin()->SetLayoutDetailView(mAsset);
	}
	else
	{
		UE_LOG(LogSubstanceSource, Error, TEXT("Asset attempted to set source panels detail view but parent is invalid"))
	}

	//Return handled
	return FReply::Handled();
}

void SSubstanceSourceAsset::OnSubstancePurchased(Alg::Source::AssetPtr PurchasedAsset)
{
	if (PurchasedAsset == mAsset)
	{
		RegenerateTileLayout();
	}
}

void SSubstanceSourceAsset::OnModuleShutdown()
{
	ISubstanceSourceModule::Get()->OnModuleShutdown().Remove(OnModuleShutdownDelegateHandle);
	ISubstanceSourceModule::Get()->OnSubstancePurchased().Remove(OnAssetPurchasedDelegateHandle);
	mAsset.reset();
}

void SSubstanceSourceAsset::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	IsDownloadOverlayInView = true;
	Invalidate(EInvalidateWidget::Layout);
}

void SSubstanceSourceAsset::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	//Call parent implementation
	SWidget::OnMouseLeave(MouseEvent);
	IsDownloadOverlayInView = false;
	Invalidate(EInvalidateWidget::Layout);
}

/** Download the asset and add to project */
FReply SSubstanceSourceAsset::OnDownloadOverlayClicked(const FGeometry& Geom, const FPointerEvent& PointerEvent)
{
	FVector2D ClickLocalPos = Geom.AbsoluteToLocal(PointerEvent.GetScreenSpacePosition());
	if (ClickLocalPos.X + ClickLocalPos.Y >	Geom.GetLocalSize().X)
	{
		ISubstanceSourceModule::Get()->LoadSubstance(mAsset);
	}
	else
	{
		return OnOpenDetailsPanel();
	}

	return FReply::Handled();
}

bool SSubstanceSourceAsset::CanLoadThumbnailFromCache() const
{
	FString AssetPath = FString(mAsset->getID().c_str() + FString("_") + FString(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL));

	//Check to see if the file exists on dis
	if (!IFileManager::Get().FileExists(*ISubstanceSourceModule::Get()->GetImageCachePath(AssetPath)))
	{
		return false;
	}

	//Convert seconds to days
	float SecondsFractionToDays = 0.0000115741f;
	double Days = IFileManager::Get().GetFileAgeSeconds(*ISubstanceSourceModule::Get()->GetImageCachePath(AssetPath)) * SecondsFractionToDays;
	if (SecondsFractionToDays > MAX_CACHE_DURATION_DAYS)
	{
		return false;
	}

	return true;
}

void SSubstanceSourceAsset::LoadThumbnailFromCache()
{
	FString AssetPath = FString(mAsset->getID().c_str() + FString("_") + FString(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL));
	ThumbnailBrush = ISubstanceSourceModule::Get()->LoadImageFromCache(AssetPath);
	if (ThumbnailBrush.IsValid())
	{
		TextureResource = ThumbnailBrush->GetResourceObject();
	}
	RegenerateTileLayout();
}

/** Only used to update slider positions - It would be nice to drive this from events rather than tick similar to a blueprint time line */
void SSubstanceSourceAsset::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Update sliders if they are not in place
	if (IsDownloadOverlayInView && DownloadOverlayOffset > FullVisible)
	{
		DownloadOverlayOffset -= InDeltaTime * SlideSpeed;
		Invalidate(EInvalidateWidget::Layout);
	}
	else if (!IsDownloadOverlayInView && DownloadOverlayOffset < FullHidden)
	{
		DownloadOverlayOffset += InDeltaTime * SlideSpeed;
		Invalidate(EInvalidateWidget::Layout);
	}

	//Clamp the offset in the case it goes out of expected bounds.
	if (IsDownloadOverlayInView && DownloadOverlayOffset < FullVisible)
	{
		DownloadOverlayOffset = FullVisible;
		Invalidate(EInvalidateWidget::Layout);
	}
	else if (!IsDownloadOverlayInView && DownloadOverlayOffset > FullHidden)
	{
		DownloadOverlayOffset = FullHidden;
		Invalidate(EInvalidateWidget::Layout);
	}
}
