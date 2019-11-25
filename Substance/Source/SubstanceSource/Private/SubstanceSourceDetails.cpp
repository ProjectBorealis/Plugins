// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceDetails.cpp

#include "SubstanceSourceDetails.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceSourcePanel.h"
#include "SubstanceSourceStyle.h"

#include "IImageWrapper.h"
#include "ImageUtils.h"
#include "Styling/SlateBrush.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"

#include <substance/source/attachment.h>

#define DEFAULT_DETAIL_SIZE_X 1920
#define DEFAULT_DETAIL_SIZE_Y 1080

/** SCompound widget override - Called when ever this widget is instantiated */
void SSubstanceSourceDetails::Construct(const FArguments& InArgs)
{
	//Bind Callbacks
	ISubstanceSourceModule::Get()->OnModuleShutdown().AddSP(this, &SSubstanceSourceDetails::OnModuleShutdown);

	//Set args from slate
	mAsset = InArgs._Asset;
	mParentWindow = InArgs._ParentWindow;

	mImagesLoaded = false;

	//Default timer
	mFlipTimer = 5.0f;

	//Generate the tiles - automagically generates based on state
	RegeneratePanelLayout();
}

SSubstanceSourceDetails::~SSubstanceSourceDetails()
{
	DetailsSlicesImageBrush.Reset();
	DetailsMapsImageBrush.Reset();

	for (const auto& ResourceItr : mTextureResources)
	{
		ResourceItr->ClearFlags(RF_Standalone);
	}

	mTextureResources.Empty();
}

/** Callback that creates the slices image, adds it to the viewport and removes the loading widget */
void SSubstanceSourceDetails::ConstructDetailsSlicesImage()
{
	DetailsSlicesImageBrush = ISubstanceSourceModule::Get()->ConstructImageBrush(SUBSTANCE_SOURCE_PREVIEW_IMAGE_SLICES, mAsset);

	//Store the texture resource for proper cleanup
	if (DetailsSlicesImageBrush.IsValid() && DetailsSlicesImageBrush->GetResourceObject())
	{
		mTextureResources.Add(DetailsSlicesImageBrush->GetResourceObject());
	}

	//Regenerates now that the panel brush is valid
	RegeneratePanelLayout();
}

void SSubstanceSourceDetails::ConstructDetailsMapsImage()
{
	DetailsMapsImageBrush = ISubstanceSourceModule::Get()->ConstructImageBrush(SUBSTANCE_SOURCE_PREVIEW_IMAGE_OUTPUTS, mAsset);

	//Store the texture resource for proper cleanup
	if (DetailsMapsImageBrush.IsValid() && DetailsMapsImageBrush->GetResourceObject())
	{
		mTextureResources.Add(DetailsMapsImageBrush->GetResourceObject());
	}

	//Using Details map as initial brush
	CurrentSlateBrush = DetailsMapsImageBrush;

	//Regenerates now that the panel brush is valid
	RegeneratePanelLayout();
}

/** Clears the layout and creates all of the child widgets of the tile */
void SSubstanceSourceDetails::RegeneratePanelLayout()
{
	//Clear all widgets from the root
	ChildSlot.DetachWidget();

	//Store the widgets according to the state
	TSharedPtr<SWidget> View = SNullWidget::NullWidget;
	TSharedPtr<SWidget> Loading = SNullWidget::NullWidget;

	//NOTE:: We are defaulting to the slices image brush and to switch it quickly without resetting up
	//the entire menu post creation, we can store the view widget as a member of this class and swap the image brush!
	if (DetailsMapsImageBrush.IsValid() && DetailsSlicesImageBrush.IsValid())
	{
		View =
		    //Slot for the image
		    SNew(SOverlay)
		    + SOverlay::Slot()
		    [
		        SNew(SScaleBox)
		        //.VAlign(VAlign_Bottom)
		        //.HAlign(HAlign_Left)
		        .Stretch(EStretch::ScaleToFit)
		        [
		            SNew(SHorizontalBox)
		            + SHorizontalBox::Slot()
		            .FillWidth(1.0f)
		            [
		                SNew(SOverlay)

		                + SOverlay::Slot()
		                [
		                    SNew(SImage)
		                    .Image(CurrentSlateBrush.Get())
		                ]

		                //Slot for the loop left button
		                + SOverlay::Slot()
		                .HAlign(HAlign_Left)
		                .VAlign(VAlign_Center)
		                [
		                    SNew(SScaleBox)
		                    .Stretch(EStretch::None)
		                    .IgnoreInheritedScale(true)
		                    [
		                        SNew(SBox)
		                        .MinDesiredWidth(32.0f)
		                        .MinDesiredHeight(43.0f)
		                        .MaxDesiredWidth(32.0f)
		                        .MaxDesiredHeight(43.0f)
		                        [
		                            SNew(SButton)
		                            .ButtonStyle(FSubstanceSourceStyle::Get(), "DetailsArrowLeft")
		                            .OnClicked(this, &SSubstanceSourceDetails::SwitchNextImage)
		                        ]
		                    ]
		                ]

		                //Slot for the loop right button
		                + SOverlay::Slot()
		                .HAlign(HAlign_Right)
		                .VAlign(VAlign_Center)
		                [
		                    SNew(SScaleBox)
		                    .Stretch(EStretch::None)
		                    .IgnoreInheritedScale(true)
		                    [
		                        SNew(SBox)
		                        .MinDesiredWidth(32.0f)
		                        .MinDesiredHeight(43.0f)
		                        .MaxDesiredWidth(32.0f)
		                        .MaxDesiredHeight(43.0f)
		                        [
		                            SNew(SButton)
		                            .ButtonStyle(FSubstanceSourceStyle::Get(), "DetailsArrowRight")
		                            .OnClicked(this, &SSubstanceSourceDetails::SwitchNextImage)
		                        ]
		                    ]
		                ]

		                //Slot for the close button
		                + SOverlay::Slot()
		                .VAlign(VAlign_Top)
		                .HAlign(HAlign_Right)
		                .Padding(FMargin(0.0f, 8.0f, 8.0f, 0.0f))
		                [
		                    SNew(SScaleBox)
		                    .Stretch(EStretch::None)
		                    .IgnoreInheritedScale(true)
		                    [
		                        SNew(SButton)
		                        .OnClicked(this, &SSubstanceSourceDetails::OnDetailWindowCloseButton)
		                        .ButtonStyle(FSubstanceSourceStyle::Get(), "CloseDetailsPanelButton")
		                    ]
		                ]
		            ]

		        ]
		    ];

	}
	else
	{
		View =
		    SNew(SBox)
		    [
		        SNew(SButton)
		        .ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
		    ];

		Loading =
		    SNew(SCircularThrobber)
		    .IsEnabled(true)
		    .Radius(200.0f)
		    .NumPieces(50)
		    .Period(2.5f);
	}

	//Create the layout
	TSharedPtr<SVerticalBox> BaseWidget =
	    SNew(SVerticalBox)

	    //Add the image of the asset
	    + SVerticalBox::Slot()
	    .HAlign(EHorizontalAlignment::HAlign_Fill)
	    .FillHeight(0.8f)
	    .Padding(FMargin(0, 0, 0, 0))
	    [
	        //Create an overlay for layering widgets
	        SNew(SOverlay)

	        //Add a slot for the main view
	        + SOverlay::Slot()
	        .HAlign(HAlign_Fill)
	        .VAlign(VAlign_Fill)
	        [
	            View->AsShared()
	        ]

	        //Add a slot for the loading icon
	        + SOverlay::Slot()
	        .HAlign(HAlign_Center)
	        .VAlign(VAlign_Center)
	        [
	            Loading->AsShared()
	        ]
	    ];

	//Add to the root
	ChildSlot
	[
	    BaseWidget->AsShared()
	];
}

/** Accessor for passing the asset associated with this widget to the factory */
Alg::Source::AssetPtr SSubstanceSourceDetails::GetAsset() const
{
	return mAsset;
}

/** Heart beat that should only ever be used for updating and checking the timer */
void SSubstanceSourceDetails::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	mFlipTimer -= InDeltaTime;
	if (mFlipTimer <= 0.0f)
	{
		SwitchNextImage();
	}

	//Load images
	if (!mImagesLoaded)
	{
		FVector2D defaultSize(DEFAULT_DETAIL_SIZE_X, DEFAULT_DETAIL_SIZE_Y);
		ISubstanceSourceModule::Get()->LoadDetailsMaps(AsShared(), defaultSize);
		ISubstanceSourceModule::Get()->LoadDetailsSliced(AsShared(), defaultSize);
		mImagesLoaded = true;
	}
}

FReply SSubstanceSourceDetails::SwitchNextImage()
{
	//Reset timer
	mFlipTimer = 5.0f;

	//Make sure both brushes are valid
	if (!DetailsSlicesImageBrush.IsValid() || !DetailsMapsImageBrush.IsValid())
	{
		UE_LOG(LogSubstanceSource, Error, TEXT("Not all details images have been loaded when attempting to switch image in the detail panel "))
		return FReply::Handled();
	}

	//Set the brush based on what it currently is.
	//NOTE:: If more images are added in the future, store them in an array and save the index rather than a brush reference to
	//quickly switch.
	if (CurrentSlateBrush == DetailsSlicesImageBrush)
	{
		CurrentSlateBrush = DetailsMapsImageBrush;
	}
	else
	{
		CurrentSlateBrush = DetailsSlicesImageBrush;
	}

	//This may be overkill - Maybe we can switch the Image widget rather than the brush!
	RegeneratePanelLayout();

	//Return handled
	return FReply::Handled();

}

FReply SSubstanceSourceDetails::OnDetailWindowCloseButton()
{
	if (mParentWindow.IsValid())
	{
		return mParentWindow.Pin()->OnCloseDetailsWindow();
	}

	return FReply::Handled();
}

void SSubstanceSourceDetails::OnModuleShutdown()
{
	mAsset.reset();
}
