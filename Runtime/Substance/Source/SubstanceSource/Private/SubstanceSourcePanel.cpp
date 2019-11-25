// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourcePanel.cpp

#include "SubstanceSourcePanel.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceSourceModule.h"

#include "SubstanceSourceAsset.h"
#include "SubstanceSourceDetails.h"
#include "SubstanceScrollBox.h"

#include "Widgets/SWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

#include "Misc/MessageDialog.h"

#include "Types/SlateStructs.h"

#include <substance/source/attachment.h>
#include <substance/source/database.h>
#include <substance/source/asset.h>


/** SSubstanceSourcePanel Constants */
const int SSubstanceSourcePanel::THUMBNAIL_WIDTH = 415;
const int SSubstanceSourcePanel::THUMBNAIL_HEIGHT = 415;
const int SSubstanceSourcePanel::MIN_SOURCEASSET_WIDTH = 200;
const int SSubstanceSourcePanel::BASE_SOURCEASSET_WIDTH = 415;
const int SSubstanceSourcePanel::BASE_SOURCEASSET_HEIGHT = 465;
const int SSubstanceSourcePanel::LOADINGIMAGE_FRAMES_PER_SECOND = 30;
const char* SSubstanceSourcePanel::CATEGORY_MYASSETS = "My Assets - ";
const char* SSubstanceSourcePanel::CATEGORY_TAGORSEARCH = "Tag or Search";

const char* SSubstanceSourcePanel::SORT_BY_RECENTLY_UPDATED_FIRST = "Recently updated first";
const char* SSubstanceSourcePanel::SORT_BY_DOWNLOAD_DATE = "Sort by download date";
const char* SSubstanceSourcePanel::SORT_BY_NAME = "Sort by name";
const char* SSubstanceSourcePanel::MY_ASSETS_SORTS[] =
{
	SORT_BY_RECENTLY_UPDATED_FIRST,
	SORT_BY_DOWNLOAD_DATE,
	SORT_BY_NAME
};

/** SCompoundWidget interface */
void SSubstanceSourcePanel::Construct(const FArguments& InArgs)
{
	//Bind the callback events
	ISubstanceSourceModule::Get()->OnLoggedIn().AddSP(this, &SSubstanceSourcePanel::OnUserHasLoggedIn);
	ISubstanceSourceModule::Get()->OnLoggedOut().AddSP(this, &SSubstanceSourcePanel::OnUserHasLoggedOut);
	ISubstanceSourceModule::Get()->OnLoginFailed().AddSP(this, &SSubstanceSourcePanel::OnUserLoginFailed);
	ISubstanceSourceModule::Get()->OnDatabaseLoaded().AddSP(this, &SSubstanceSourcePanel::OnSourceLoadComplete);
	ISubstanceSourceModule::Get()->OnSearchCompleted().AddSP(this, &SSubstanceSourcePanel::OnSearchedAssetsFound);
	ISubstanceSourceModule::Get()->OnDisplayUsersAssets().AddSP(this, &SSubstanceSourcePanel::OnDisplayMyAssets);
	ISubstanceSourceModule::Get()->OnDisplayCategory().AddSP(this, &SSubstanceSourcePanel::OnDisplayCategory);
	ISubstanceSourceModule::Get()->OnModuleShutdown().AddSP(this, &SSubstanceSourcePanel::OnModuleShutdown);

	//Kick off the load if the database hasn't been loaded
	if (ISubstanceSourceModule::Get() &&
	        (ISubstanceSourceModule::Get()->GetDatabaseLoadingStatus() == ESourceDatabaseLoadingStatus::NotLoaded ||
	         ISubstanceSourceModule::Get()->GetDatabaseLoadingStatus() == ESourceDatabaseLoadingStatus::Error))
	{
		ISubstanceSourceModule::Get()->LoadDatabase();
	}

	//Set the default category name
	CurrentSelectedCategory = Alg::Source::Database::sCategoryAllAssets;

	//Access to database functions (in SubstanceSourceModule)
	bDatabaseLoaded = false;

	LoadingImageDeltaTimeMod = 0.0f;

	//Call the construct functions to create all of the areas we will need
	ConstructLoadStatus();
	ConstructUsersLogin();
	ConstructCategories();
	ConstructAssetTiles();
	ConstructDetailArea();

	//Construct MyAssetMenu widgets
	if (ISubstanceSourceModule::Get()->GetUserLoginStatus() == ESourceUserLoginStatus::LoggedIn)
	{
		ConstructMyAssetsCategoryWidget();
		ConstructMyAssetsSortingWidget();

		CurrentMyAssetsCategoryItem = MyAssetsCategoryItems[0];
		CurrentMyAssetsSortingItem = MyAssetsSortingItems[0];
	}

	//Set the layout based on if the database has been loaded already or not
	ESourceDatabaseLoadingStatus LoadingStatus = ISubstanceSourceModule::Get()->GetDatabaseLoadingStatus();
	if (LoadingStatus != ESourceDatabaseLoadingStatus::Loaded)
	{
		SetLayoutLoadingView();
	}
	else
	{
		PopulateCategories();
		PopulateAssetTiles();
		SetLayoutTilesView();
		SelectCategoryButton();
	}
}

SSubstanceSourcePanel::~SSubstanceSourcePanel()
{
	//Clear hard references to Assets
	CurrentCategoryWidgets.Empty();
	CurrentTileSet.Empty();
}

void SSubstanceSourcePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	ISubstanceSourceModule* SourceModule = ISubstanceSourceModule::Get();

	if (SourceModule->GetUserLoginStatus() == ESourceUserLoginStatus::LoggedIn)
	{
		SourceModule->ManageLoginTimer(InCurrentTime);
	}

	//update loading screen animation image
	if (LoadingImage.IsValid())
	{
		LoadingImageDeltaTimeMod += InDeltaTime * FSubstanceSourceStyle::ANIMLOADING_FRAME_RATE;

		int Frame = (int)FMath::Fmod(LoadingImageDeltaTimeMod, (float)FSubstanceSourceStyle::ANIMLOADING_IMAGE_COUNT) + 1;
		FString Identifier = FString("SourceAnimLoading") + FString::FromInt(Frame);

		LoadingImage.Pin()->SetImage(FSubstanceSourceStyle::Get()->GetBrush(*Identifier));
	}

	//update tile widget dimensions
	if (AssetTileArea.IsValid())
	{
		const FVector2D& LocalSize = AssetTileArea->GetCachedGeometry().GetLocalSize();
		int NumColumns = ((LocalSize.X / 4) >= MIN_SOURCEASSET_WIDTH) ? 4
		                 : ((LocalSize.X / 3) >= MIN_SOURCEASSET_WIDTH) ? 3
		                 : ((LocalSize.X / 2) >= MIN_SOURCEASSET_WIDTH) ? 2 : 1;
		float AssetScale = (LocalSize.X / ((float)NumColumns + 0.25f)) / (float)BASE_SOURCEASSET_WIDTH;

		AssetTileArea->SetItemWidth((float)BASE_SOURCEASSET_WIDTH * AssetScale);
		AssetTileArea->SetItemHeight((float)BASE_SOURCEASSET_HEIGHT * AssetScale);
	}

	//update status bar
	if (StatusText.IsValid() && StatusProgress.IsValid())
	{
		FString StatusTextString;

		int NumDownloads = 0;
		Alg::Source::String AssetTitle;
		float PercentDownloaded = 0.0f;

		if (SourceModule->GetCurrentDownloadInformation(NumDownloads, AssetTitle, PercentDownloaded))
		{
			StatusTextString = TEXT("Downloading \"") + FString(AssetTitle.c_str()) + TEXT("\"");

			if (NumDownloads > 1)
			{
				StatusTextString += TEXT(" (and ") + FString::FromInt(NumDownloads - 1) + TEXT(" more)");
			}
		}
		else
		{
			StatusTextString = TEXT("Ready");
		}

		StatusText->SetText(FText::FromString(StatusTextString));
		StatusProgress->SetPercent(PercentDownloaded);
	}

	//Process thumbnail queue
	if (SourcePanelViewState == eMyAssetView || SourcePanelViewState == eAssetTileView)
	{
		ISubstanceSourceModule::Get()->ProcessThumbnailCacheQueue();
	}
}

/** Stores a pointer parent window */
void SSubstanceSourcePanel::SetParentWindow(TWeakPtr<FSubstanceSourceWindow> ParentWindow)
{
	ParentWindowPtr = ParentWindow;
}

/** Gets a reference to the parent window this belongs to */
TWeakPtr<FSubstanceSourceWindow> SSubstanceSourcePanel::GetParentWindow() const
{
	return ParentWindowPtr;
}

/** Creates the categories section on the left hand side of the window */
void SSubstanceSourcePanel::ConstructCategories()
{
	CategoryArea =
	    SNew(SVerticalBox);
}

void SSubstanceSourcePanel::PopulateCategories()
{
	//Clear previous list
	CategoryArea->ClearChildren();

	Alg::Source::Database* database = ISubstanceSourceModule::Get()->GetDatabase();
	FString CategoryName = FString("NO_NAME");
	FString numberOfAssets;

	//Clear previous widget reference
	CurrentCategoryWidgets.Empty();

	TSharedPtr<SVerticalBox> ListVerticalBox = SNew(SVerticalBox);

	for (const auto& category : database->getCategoryAssetVector())
	{
		TSharedPtr<ButtonDetails> ButtonWidgetStruct = MakeShared<ButtonDetails>();

		CategoryName = FString(category.first.c_str());

		numberOfAssets = FString::FromInt(category.second.size());

		//Create elements we need to store to add later
		ButtonWidgetStruct->mCategoryLabelText =
		    SNew(STextBlock)
		    .TextStyle(FSubstanceSourceStyle::Get(), "CategoryText")
		    .Text(FText::FromString(CategoryName))
		    .Justification(ETextJustify::Left)
		    .WrapTextAt(128.0f);

		ButtonWidgetStruct->mCategoryCountText =
		    SNew(STextBlock)
		    .TextStyle(FSubstanceSourceStyle::Get(), "CategoryCountText")
		    .Text(FText::FromString(numberOfAssets))
		    .Justification(ETextJustify::Right);

		if (CategoryName == "All Categories")
		{
			ButtonWidgetStruct->mButtonWidget =
			    SNew(SButton)
			    .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceCategoryButton")
			    .OnClicked(this, &SSubstanceSourcePanel::OnCategorySwitched, CategoryName)
			    [
			        SNew(SHorizontalBox)

			        + SHorizontalBox::Slot()
			        .FillWidth(0.8f)
			        .VAlign(VAlign_Center)
			        .Padding(18.0f, 0.0f, 0.0f, 0.0f)
			        [
			            ButtonWidgetStruct->mCategoryLabelText->AsShared()
			        ]
			    ];
		}
		else
		{

			ButtonWidgetStruct->mButtonWidget =
			    SNew(SButton)
			    .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceCategoryButton")
			    .OnClicked(this, &SSubstanceSourcePanel::OnCategorySwitched, CategoryName)
			    [
			        SNew(SHorizontalBox)

			        + SHorizontalBox::Slot()
			        .FillWidth(0.8f)
			        .VAlign(VAlign_Center)
			        .Padding(18.0f, 0.0f, 0.0f, 0.0f)
			        [
			            ButtonWidgetStruct->mCategoryLabelText->AsShared()
			        ]

			        + SHorizontalBox::Slot()
			        .FillWidth(0.2f)
			        .VAlign(VAlign_Center)
			        .Padding(0.0f, 0.0f, 10.0f, 0.0f)
			        [
			            ButtonWidgetStruct->mCategoryCountText->AsShared()
			        ]
			    ];
		}

		ListVerticalBox->AddSlot()
		.Padding(0)
		[
		    SNew(SBox)
		    .MinDesiredHeight(FOptionalSize(45))
		    .Padding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))
		    [
		        ButtonWidgetStruct->mButtonWidget->AsShared()
		    ]
		];

		//Push back the recently created category button details for access later
		CurrentCategoryWidgets.Add(ButtonWidgetStruct);

		//Bind our callbacks to update our styles
		ButtonWidgetStruct->mButtonWidget->SetOnHovered(FSimpleDelegate::CreateSP(ButtonWidgetStruct.Get(), &ButtonDetails::ButtonHovered));
		ButtonWidgetStruct->mButtonWidget->SetOnUnhovered(FSimpleDelegate::CreateSP(ButtonWidgetStruct.Get(), &ButtonDetails::ButtonUnHovered));
	}

	CategoryArea->AddSlot()
	[
	    SNew(SOverlay)
	    //Slot to constrain the area so that it doesn't scale
	    + SOverlay::Slot()
	    [
	        SNew(SConstraintCanvas)
	        + SConstraintCanvas::Slot()
	        .Offset(FMargin(0.0f, 0.0f, 220.0f, 1.0f))
	        .Anchors(FAnchors(220.0f, 1.0f, 220.0f, 1024.0f))
	        .Alignment(FVector2D(0.5f, 0.5f))
	        .AutoSize(false)
	        [
	            SNullWidget::NullWidget
	        ]
	    ]

	    //Slot to fill the area with
	    + SOverlay::Slot()
	    .Padding(FMargin(0.0f, 0.0f, 0.0f, 5.0f))
	    .VAlign(VAlign_Fill)
	    .HAlign(HAlign_Fill)
	    [
	        SNew(SScrollBox)
	        .ScrollBarVisibility(EVisibility::Collapsed)
	        + SScrollBox::Slot()
	        .Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
	        [
	            ListVerticalBox->AsShared()
	        ]
	    ]
	];
}

FReply SSubstanceSourcePanel::OnDownloadButton(Alg::Source::AssetPtr Asset)
{
	ISubstanceSourceModule::Get()->LoadSubstance(Asset);
	return FReply::Handled();
}

/** Callback called when a tag button is clicked from the details panel */
FReply SSubstanceSourcePanel::OnSearchTag(FString TagToSearch)
{
	ISubstanceSourceModule::Get()->SearchStringChanged(TagToSearch);

	Alg::Source::AssetVector SearchedAssets = ISubstanceSourceModule::Get()->GetSearchResults();
	FString SearchCountLabel = " (" + FString::FromInt(SearchedAssets.size()) + ")";
	AssetTileHeader_TagButtonText->SetText(FText::FromString(TagToSearch + SearchCountLabel));
	SetCurrentSelectedCategory(CATEGORY_TAGORSEARCH);

	//Move scroll box to the top
	if (AssetTileScrollBox.IsValid())
	{
		AssetTileScrollBox.Pin()->ScrollToStart();
	}

	return FReply::Handled();
}

FReply SSubstanceSourcePanel::OnCloseDetailsWindow()
{
	SetLayoutTilesView();
	return FReply::Handled();
}

void SSubstanceSourcePanel::PopulateAssetTiles(bool ShouldResetScroll)
{
	Alg::Source::Database* database = ISubstanceSourceModule::Get()->GetDatabase();

	if (!database)
	{
		//Log error that the database could not be found
		UE_LOG(LogSubstanceSource, Error, TEXT("The source database is not valid. Could not populate asset tiles! "))
		return;
	}

	// Make sure the current category is valid (in this function: CATEGORY_MYASSETS or CATEGORY_TAGORSEARCH are not permitted!)
	if (CurrentSelectedCategory.Contains(CATEGORY_MYASSETS) || CurrentSelectedCategory.Contains(CATEGORY_TAGORSEARCH))
	{
		SetCurrentSelectedCategory(FString(Alg::Source::Database::sCategoryAllAssets));
	}

	//Fix up category name if all assets or new assets
	Alg::Source::String currentSelectedCategory = TCHAR_TO_ANSI(*CurrentSelectedCategory);

	//Search for the category
	const Alg::Source::CategoryAssetVector& categoryAssets = database->getCategoryAssetVector();
	auto iter = categoryAssets.begin();

	while (iter != categoryAssets.end())
	{
		if (iter->first == currentSelectedCategory)
		{
			break;
		}

		iter++;
	}

	if (iter == categoryAssets.end())
	{

		if (CurrentSelectedCategory != Alg::Source::Database::sCategoryAllAssets)
		{
			SetCurrentSelectedCategory(Alg::Source::Database::sCategoryAllAssets);
			return;
		}
		//Log that the category name could not be found!
		UE_LOG(LogSubstanceSource, Error, TEXT("Searched for a category name that could be invalid as no assets were found! "))
		check(0);
		return;
	}

	if (ShouldResetScroll)
	{
		//Clear the old tile set
		CurrentTileSet.Empty();

		for (const auto& AssetItr : iter->second)
		{
			if (Alg::Source::ImageAttachment* attachment = AssetItr->getImageAttachmentByLabel(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL))
			{
				attachment->setDesiredImageHeight(THUMBNAIL_HEIGHT);
				attachment->setDesiredImageWidth(THUMBNAIL_WIDTH);
			}

			//Create a new custom tile widget
			TSharedPtr<SSubstanceSourceAsset> SourceAsset =
			    SNew(SSubstanceSourceAsset)
			    .Label(AssetItr->getTitle().c_str())
			    .Asset(AssetItr)
			    .ParentWindow(SharedThis(this));

			CurrentTileSet.Add(SourceAsset);
		}

		//Recreate the menu with the new assets
		AssetTileArea->RebuildList();
		AssetTileArea->ScrollToTop();

		//Move scroll box to the top
		if (AssetTileScrollBox.IsValid())
		{
			AssetTileScrollBox.Pin()->ScrollToStart();
		}
	}

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

void SSubstanceSourcePanel::SetCurrentSelectedCategory(const FString& CategoryName)
{
	// Function used to store the new category name into the 'CurrentSelectedCategory'
	// variable, and also enable/disable the tile area header (tag or MyAssets widgets)
	CurrentSelectedCategory = CategoryName;

	AssetTileHeader_MyAssets->SetVisibility(CurrentSelectedCategory.Contains(CATEGORY_MYASSETS) ? EVisibility::All : EVisibility::Collapsed);
	AssetTileHeader_Tag->SetVisibility(CurrentSelectedCategory == FString(CATEGORY_TAGORSEARCH) ? EVisibility::All : EVisibility::Collapsed);
}

FReply SSubstanceSourcePanel::OnCategorySwitched(FString NewCategoryName)
{
	FSlateApplication::Get().FlushRenderState();

	//Make sure stale queue has been cleared
	ISubstanceSourceModule::Get()->ClearThumbnailCacheQueue();

	//Fire an event that lets the front end know that the category has just changed
	ISubstanceSourceModule::Get()->OnCategorySwitched();

	//Switch back to tiles view if we aren't currently there
	if (SourcePanelViewState != eAssetTileView)
	{
		SetLayoutTilesView();
	}

	//Clear the previously selected button
	ClearCurrentCategorySelection();

	//If the category hasn't changed, don't reset scroll
	bool ResetScroll = true;
	if (*CurrentSelectedCategory == NewCategoryName)
	{
		PopulateAssetTiles(false);
	}
	else
	{
		//Set the newly selected category
		CurrentSelectedCategory = NewCategoryName;
		SetCurrentSelectedCategory(CurrentSelectedCategory);

		//Rebuild the asset tile view to show the assets in the new category
		PopulateAssetTiles();
	}

	//Switch the style of the clicked button to the new style
	SelectCategoryButton();

	//Return handled event
	return FReply::Handled();
}

FReply SSubstanceSourcePanel::OnDisplayMainCategories()
{
	OnCategorySwitched(Alg::Source::Database::sCategoryAllAssets);

	//Return handled event
	return FReply::Handled();
}

FReply SSubstanceSourcePanel::OnLoginAttempted()
{
	if (!EULAAcceptedCheckbox->IsChecked())
	{
		//Alert the user they didn't attempt to login because they have not accepted
		FText Title = FText::FromString("Login Error");
		FText Message = FText::FromString("EULA must be accepted for login");
		FMessageDialog::Open(EAppMsgType::Ok, Message, &Title);
		return FReply::Handled();
	}

	//Send the inputs to the framework
	ISubstanceSourceModule::Get()->UserLogin(UsernameInput->GetText().ToString(), PasswordInput->GetText().ToString());

	//Loading icon visible while log in process is running
	LoginSpinner->SetVisibility(EVisibility::Visible);

	//Return handled event
	return FReply::Handled();
}


/** "My Assets" ComboBox supports: */
void SSubstanceSourcePanel::ConstructMyAssetsSortingWidget()
{
	MyAssetsSortingItems.Empty();
	for (const auto& SortItr : MY_ASSETS_SORTS)
	{
		MyAssetsSortingItems.Add(MakeShared<FString>(SortItr));
	}
}

TSharedRef<SWidget> SSubstanceSourcePanel::OnGenerateMyAssetsSortingWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
	       .TextStyle(FSubstanceSourceStyle::Get(), "SourceComboBox.OptionButtonsText")
	       .Text(FText::FromString(*InItem));
}
void SSubstanceSourcePanel::OnMyAssetsSortingSelection(TSharedPtr<FString> pSelectedItem, ESelectInfo::Type pSelectInfo)
{
	for (const auto& AssetItr : MyAssetsSortingItems)
	{
		if (pSelectedItem == AssetItr)
		{
			CurrentMyAssetsSortingItem = pSelectedItem;

			//Regenerate with the newly selected filter
			RegenerateMyAssetList();
			break;
		}
	}
}
FText SSubstanceSourcePanel::GetMyAssetsSortingLabel() const
{
	return FText::FromString(*CurrentMyAssetsSortingItem);
}

FString SSubstanceSourcePanel::AssessMyAssetCategory(const FString& Category)
{
	// The MyAssets category name (stored in variable 'CurrentSelectedCategory' ) is of the form:
	//			"MyAssets - xxx"
	// where "xxx" is the actual category

	return FString(CATEGORY_MYASSETS) + Category;

}
void SSubstanceSourcePanel::ConstructMyAssetsCategoryWidget()
{
	// Note (#TODO ?): Should perhaps be built when receiving the login's userInfo...
	Alg::Source::AssetVector OwnedAssets = ISubstanceSourceModule::Get()->GetUserOwnedAssets();

	TArray<Alg::Source::String> Categories;

	// Parse all owned assets and keep matching categories:
	Categories.Add(Alg::Source::Database::sCategoryAllAssets);

	for (const auto& AssetItr : OwnedAssets)
	{
		Categories.AddUnique(AssetItr->getCategory());
	}

	Categories.Sort();

	// Populate the category combobox:
	MyAssetsCategoryItems.Empty();
	for (const auto& CategoryItr : Categories)
	{
		MyAssetsCategoryItems.Add(MakeShared<FString>(UTF8_TO_TCHAR(CategoryItr.c_str())));
	}
}

TSharedRef<SWidget> SSubstanceSourcePanel::OnGenerateMyAssetsCategoryWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
	       .TextStyle(FSubstanceSourceStyle::Get(), "SourceComboBox.OptionButtonsText")
	       .Text(FText::FromString(*InItem));
}
void SSubstanceSourcePanel::OnMyAssetsCategorySelection(TSharedPtr<FString> pSelectedItem, ESelectInfo::Type pSelectInfo)
{
	if (pSelectedItem.IsValid())
	{
		for (const auto& CategoryItr : MyAssetsCategoryItems)
		{
			if (*pSelectedItem.Get() == *CategoryItr.Get())
			{
				CurrentMyAssetsCategoryItem = pSelectedItem;

				SetCurrentSelectedCategory(AssessMyAssetCategory(*pSelectedItem.Get()));

				//Regenerate with the newly selected filter
				RegenerateMyAssetList();
				break;
			}
		}
	}
}
FText SSubstanceSourcePanel::GetMyAssetsCategoryLabel() const
{
	return FText::FromString(*CurrentMyAssetsCategoryItem);

}

/** Creates the main tile view that will display all of the asset tiles */
void SSubstanceSourcePanel::ConstructAssetTiles()
{
	AssetTileArea =
	    SNew(STileView<TSharedPtr<SSubstanceSourceAsset>>)
	    .ItemWidth(BASE_SOURCEASSET_WIDTH)
	    .ItemHeight(BASE_SOURCEASSET_HEIGHT)
	    .ItemAlignment(EListItemAlignment::EvenlyDistributed)
	    .ScrollbarVisibility(EVisibility::Hidden)
	    .WheelScrollMultiplier(0.0f)
	    .AllowOverscroll(EAllowOverscroll::No)
	    .ConsumeMouseWheel(EConsumeMouseWheel::Never)
	    .ListItemsSource(&CurrentTileSet)
	    .OnGenerateTile(this, &SSubstanceSourcePanel::CreateContentSourceIconTile);

	//Top of tile area when a tag / author / search has been selected:
	AssetTileHeader_Tag =
	    SNew(SBox)
	    .Visibility(CurrentSelectedCategory.Contains(CATEGORY_TAGORSEARCH) ? EVisibility::All : EVisibility::Collapsed)

	    .Padding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
	    [
	        SNew(SBox)
	        .HeightOverride(20.0f)
	        .VAlign(VAlign_Center)
	        [
	            SAssignNew(AssetTileHeader_TagButton, SButton)
	            .VAlign(VAlign_Center)
	            .ButtonStyle(FSubstanceSourceStyle::Get(), "TagSearchButton")
	            .OnClicked(this, &SSubstanceSourcePanel::OnDisplayMainCategories)
	            [
	                SNew(SHorizontalBox)

	                + SHorizontalBox::Slot()
	                [
	                    SAssignNew(AssetTileHeader_TagButtonText, STextBlock)
	                    .TextStyle(FSubstanceSourceStyle::Get(), "TagSearchText")
	                    .Text(FText::FromString(""))
	                ]
	            ]
	        ]
	    ];

	// Top of tile area when logged in and in "My Assets" mode:
	AssetTileHeader_MyAssets =
	    SNew(SBox)
	    .Visibility(CurrentSelectedCategory.Contains(CATEGORY_MYASSETS) ? EVisibility::All : EVisibility::Collapsed)
	    .Padding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
	    [
	        SNew(SVerticalBox)

	        + SVerticalBox::Slot()
	        [
	            SNew(SBox)
	            .HeightOverride(30.0f)
	            .HAlign(HAlign_Center)
	            [
	                SNew(STextBlock)
	                .TextStyle(FSubstanceSourceStyle::Get(), "TextTitle14")
	                .Text(FText::FromString("My Assets"))
	            ]
	        ]

	        + SVerticalBox::Slot()
	        [
	            SNew(SHorizontalBox)

	            + SHorizontalBox::Slot()
	            .Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
	            [
	                SAssignNew(MyAssetSortingComboBox, SComboBox<TSharedPtr<FString>>)
	                .ComboBoxStyle(FSubstanceSourceStyle::Get(), "SourceComboBox")
	                .OptionsSource(&MyAssetsSortingItems)
	                .OnGenerateWidget(this, &SSubstanceSourcePanel::OnGenerateMyAssetsSortingWidget)
	                .OnSelectionChanged(this, &SSubstanceSourcePanel::OnMyAssetsSortingSelection)
	                .InitiallySelectedItem(CurrentMyAssetsSortingItem)
	                [
	                    SNew(SBox)
	                    .WidthOverride(200.0)
	                    [
	                        SNew(STextBlock)
	                        .TextStyle(FSubstanceSourceStyle::Get(), "SourceComboBox.ToolbarButtonText")
	                        .Text(this, &SSubstanceSourcePanel::GetMyAssetsSortingLabel)
	                    ]
	                ]
	            ]

	            + SHorizontalBox::Slot()
	            .Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
	            [
	                SAssignNew(MyAssetCategoryComboBox, SComboBox<TSharedPtr<FString>>)
	                .ComboBoxStyle(FSubstanceSourceStyle::Get(), "SourceComboBox")
	                .OptionsSource(&MyAssetsCategoryItems)
	                .OnGenerateWidget(this, &SSubstanceSourcePanel::OnGenerateMyAssetsCategoryWidget)
	                .OnSelectionChanged(this, &SSubstanceSourcePanel::OnMyAssetsCategorySelection)
	                .InitiallySelectedItem(CurrentMyAssetsCategoryItem)
	                [
	                    SNew(SBox)
	                    .WidthOverride(200.0)
	                    [
	                        SNew(STextBlock)
	                        .TextStyle(FSubstanceSourceStyle::Get(), "SourceComboBox.ToolbarButtonText")
	                        .Text(this, &SSubstanceSourcePanel::GetMyAssetsCategoryLabel)
	                    ]
	                ]
	            ]
	        ]
	    ];

	TSharedRef<SSubstanceScrollBox> AssetTileScrollBoxRef =
	    SNew(SSubstanceScrollBox)
	    .AllowOverscroll(EAllowOverscroll::Yes)
	    .ScrollBarAlwaysVisible(true)

	    + SSubstanceScrollBox::Slot()
	    .Padding(0.0f)
	    [
	        SNew(SVerticalBox)

	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .HAlign(HAlign_Left)
	        [
	            AssetTileHeader_Tag->AsShared()
	        ]

	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .HAlign(HAlign_Center)
	        [
	            AssetTileHeader_MyAssets->AsShared()
	        ]

	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(5.0f, 0.0f, 5.0f, 0.0f)
	        [
	            AssetTileArea->AsShared()
	        ]
	    ];

	//Set the scroll speed
	AssetTileScrollBoxRef->SetScrollSpeedMultiplier(6.0f);

	AssetTileScrollBox = AssetTileScrollBoxRef;

	AssetTileBox =
	    SNew(SBox)
	    .Padding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
	    [
	        AssetTileScrollBoxRef
	    ];
}

/** Creates the login menu that will allow users to login to their source accounts */
void SSubstanceSourcePanel::ConstructUsersLogin()
{
	UsernameInput =
	    SNew(SEditableTextBox)
	    .Style(FSubstanceSourceStyle::Get(), "SourceLoginInput")
	    .HintText(FText::FromString("EMAIL ADDRESS"));

	PasswordInput =
	    SNew(SEditableTextBox)
	    .HintText(FText::FromString("PASSWORD"))
	    .Style(FSubstanceSourceStyle::Get(), "SourceLoginInput")
	    .IsPassword(true)
	    .OnTextCommitted(this, &SSubstanceSourcePanel::OnPasswordSubmitted);

	EULAAcceptedCheckbox =
	    SNew(SCheckBox)
	    .IsChecked(EULAAccepted);

	LoginSpinner =
	    SNew(SCircularThrobber)
	    .IsEnabled(false)
	    .Radius(15.0f)
	    .NumPieces(10)
	    .Period(.5f);

	LoginArea =
	    SNew(SBox)
	    .VAlign(VAlign_Top)
	    .HAlign(HAlign_Center)
	    .MinDesiredWidth(FOptionalSize(500.0f))
	    .MinDesiredHeight(FOptionalSize(1400.0f))
	    [
	        SNew(SVerticalBox)

	        //Add a slot for the Text header label
	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 15.0f, 0.0f, 15.0f))
	        [
	            SNew(STextBlock)
	            .TextStyle(FSubstanceSourceStyle::Get(), "LoginHeader")
	            .Text(FText::FromString("Log in"))
	        ]

	        //Add a slot for the user name
	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 0.0f, 0.0f, 20.0f))
	        [
	            SNew(SBox)
	            .WidthOverride(430.0f)	// also make corresponding SEditableTextBox widget not expandable
	            .MinDesiredHeight(40.0f)
	            .MaxDesiredHeight(40.0f)
	            [
	                UsernameInput->AsShared()
	            ]
	        ]

	        //Add a slot for the user password
	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
	        [
	            SNew(SBox)
	            .WidthOverride(430.0f)	// also make corresponding SEditableTextBox widget not expandable
	            .MinDesiredHeight(40.0f)
	            .MaxDesiredHeight(40.0f)
	            [
	                PasswordInput->AsShared()
	            ]
	        ]

	        //Add a radio button and a text block for the EULA
	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 10.0f, 0.0f, 20.0f))
	        [
	            SNew(SHorizontalBox)
	            //EULA Check box
	            + SHorizontalBox::Slot()
	            .HAlign(HAlign_Left)
	            .AutoWidth()
	            .Padding(FMargin(0.0f, 0.0f, 5.0f, 0.0f))
	            [
	                EULAAcceptedCheckbox->AsShared()
	            ]

	            //EULA Text
	            + SHorizontalBox::Slot()
	            .HAlign(HAlign_Left)
	            .VAlign(VAlign_Center)
	            .AutoWidth()
	            [
	                SNew(STextBlock)
	                .TextStyle(FSubstanceSourceStyle::Get(), "LoginNormalText")
	                .Text(FText::FromString("I agree with the "))
	            ]

	            //EULA Hyper link
	            + SHorizontalBox::Slot()
	            .HAlign(HAlign_Left)
	            .VAlign(VAlign_Center)
	            .AutoWidth()
	            [
	                SNew(SHyperlink)
	                .TextStyle(FSubstanceSourceStyle::Get(), "LoginHyperlinkText")
	                .Text(FText::FromString("EULA"))
	                .OnNavigate(this, &SSubstanceSourcePanel::OnOpenHyperlink, FString("https://www.allegorithmic.com/legal/general-terms-and-conditions"))
	            ]
	        ]

	        //Add a slot for the login button
	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 0.0f, 0.0f, 40.0f))
	        [
	            SNew(SOverlay)

	            //Login Button
	            + SOverlay::Slot()
	            .HAlign(HAlign_Center)
	            .VAlign(VAlign_Center)
	            [
	                SNew(SButton)
	                .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceLoginSubmitButton")
	                .OnClicked(this, &SSubstanceSourcePanel::OnLoginAttempted)
	            ]

	            //Loading Spinner
	            + SOverlay::Slot()
	            .HAlign(HAlign_Center)
	            .VAlign(VAlign_Center)
	            .Padding(FMargin(200.0f, 0.0f, 0.0f, 0.0f))
	            [
	                LoginSpinner->AsShared()
	            ]
	        ]

	        //Forgot information text
	        + SVerticalBox::Slot()
	        .HAlign(HAlign_Center)
	        .VAlign(VAlign_Center)
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 0.0f, 0.0f, 20.0f))
	        [
	            SNew(SHyperlink)
	            .TextStyle(FSubstanceSourceStyle::Get(), "LoginHyperlinkText")
	            .Text(FText::FromString("Forgot your username or password?"))
	            .OnNavigate(this, &SSubstanceSourcePanel::OnOpenHyperlink, FString("https://www.allegorithmic.com/user/password"))
	        ]

	        //Sign up text
	        + SVerticalBox::Slot()
	        .HAlign(HAlign_Center)
	        .VAlign(VAlign_Center)
	        .AutoHeight()
	        .Padding(FMargin(0.0f, 0.0f, 0.0f, 20.0f))
	        [
	            SNew(SHorizontalBox)
	            //Slot for hyperlink label
	            + SHorizontalBox::Slot()
	            .AutoWidth()
	            [
	                SNew(STextBlock)
	                .TextStyle(FSubstanceSourceStyle::Get(), "LoginNormalText")
	                .Text(FText::FromString("Don't have an account? "))
	            ]
	            //Slot for register hyperlink
	            + SHorizontalBox::Slot()
	            .AutoWidth()

	            [
	                SNew(SHyperlink)
	                .TextStyle(FSubstanceSourceStyle::Get(), "LoginHyperlinkText")
	                .Text(FText::FromString("Sign Up"))
	                .OnNavigate(this, &SSubstanceSourcePanel::OnOpenHyperlink, FString("https://www.allegorithmic.com/user/login"))
	            ]

	        ]

	        //Substance Source Button
	        + SVerticalBox::Slot()
	        .HAlign(HAlign_Center)
	        .VAlign(VAlign_Center)
	        .AutoHeight()
	        [
	            SNew(SButton)
	            .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceFooterLoginButton")
	            [
	                SNew(SBox)
	                .MaxDesiredWidth(430.0f)
	                .MaxDesiredHeight(300.0f)
	                [
	                    SNew(SScaleBox)
	                    .Stretch(EStretch::ScaleToFit)
	                    [
	                        SNew(SImage)
	                        .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceLoginIcon"))
	                    ]
	                ]
	            ]
	        ]

	        //Substance Source Hyper Link
	        + SVerticalBox::Slot()
	        .HAlign(HAlign_Center)
	        .VAlign(VAlign_Center)
	        .AutoHeight()
	        [
	            SNew(SBox)
	            .MaxDesiredWidth(430.0f)
	            .MaxDesiredHeight(300.0f)
	            [
	                SNew(SHyperlink)
	                .TextStyle(FSubstanceSourceStyle::Get(), "LoginHyperlinkText")
	                .Text(FText::FromString("Subscribe to Substance Source from $19.90/month"))
	                .OnNavigate(this, &SSubstanceSourcePanel::OnOpenHyperlink, FString("https://www.allegorithmic.com/checkout/software/ue4/1/1"))
	            ]
	        ]
	    ];
}

/** Creates the widget that will be used to display the details of an asset when it is selected. */
void SSubstanceSourcePanel::ConstructDetailArea()
{
	DetailsDisplay = SNullWidget::NullWidget;

	AssetDetailsArea =
	    SNew(SVerticalBox)

	    //Main details area
	    + SVerticalBox::Slot()
	    [
	        SNew(SHorizontalBox)

	        //Create a slot for the categories
	        + SHorizontalBox::Slot()
	        .FillWidth(0.15f)
	        [
	            CategoryArea->AsShared()
	        ]

	        //Create a slot for details menu
	        + SHorizontalBox::Slot()
	        .FillWidth(0.85f)
	        [
	            DetailsDisplay->AsShared()
	        ]
	    ]

	    //Slot for the download status bar
	    + SVerticalBox::Slot()
	    .MaxHeight(25.0f)
	    .FillHeight(0.1f)
	    .AutoHeight()
	    .Padding(0.0f, 2.0f)
	    [
	        SNew(SHorizontalBox)
	        + SHorizontalBox::Slot()
	        .Padding(2.0f, 0.0f)
	        [
	            StatusText->AsShared()
	        ]
	        + SHorizontalBox::Slot()
	        .FillWidth(0.4f)
	        .Padding(2.0f, 0.0f)
	        [
	            StatusProgress->AsShared()
	        ]
	    ];
}

void SSubstanceSourcePanel::ConstructLoadStatus()
{
	StatusText =
	    SNew(STextBlock)
	    .TextStyle(FSubstanceSourceStyle::Get(), "StatusBarText")
	    .Justification(ETextJustify::Left);

	StatusProgress =
	    SNew(SProgressBar)
	    .BarFillType(EProgressBarFillType::LeftToRight);
}

void SSubstanceSourcePanel::OnModuleShutdown()
{
	AssetDetailsArea->ClearChildren();
	CurrentTileSet.Empty();
	CurrentCategoryWidgets.Empty();
}

/** Create the view which will display the details for the selected substance */
FReply SSubstanceSourcePanel::SetLayoutDetailView(Alg::Source::AssetPtr Asset)
{
	//Clear any previous version of this display
	DetailsDisplay.Reset();

	DetailsDisplay =
	    SNew(SSubstanceSourceDetails)
	    .ParentWindow(SharedThis(this))
	    .Asset(Asset);

	//Create the area for the tags now to add later
	TSharedPtr<SHorizontalBox> Tags = SNew(SHorizontalBox);

	for (const auto& TagsItr : Asset->getTags())
	{
		FString TagLabel = FString(TagsItr.c_str());

		Tags->AddSlot()
		.Padding(FMargin(4.0f))
		.AutoWidth()
		[
		    SNew(SBox)
		    .MinDesiredWidth(75.0f)
		    .MinDesiredHeight(30.0f)
		    .MaxDesiredWidth(125.0f)
		    .MaxDesiredHeight(50.0f)
		    [
		        SNew(SButton)
		        .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceDetailTagButton")
		        .OnClicked(this, &SSubstanceSourcePanel::OnSearchTag, TagLabel)
		        [
		            SNew(SHorizontalBox)
		            + SHorizontalBox::Slot()
		            .VAlign(VAlign_Center)
		            [
		                SNew(STextBlock)
		                .Text(FText::FromString(TagLabel))
		                .Justification(ETextJustify::Center)
		                .TextStyle(FSubstanceSourceStyle::Get(), "DetailsHeaderTags")
		            ]
		        ]
		    ]
		];
	}

	//Sbsar attachment field - Used to get the sbsar file name for display
	Alg::Source::String SbsarFilename;

	if (Alg::Source::DownloadAttachment* downloadAttachment = Asset->getDownloadAttachmentByMimeType(SUBSTANCE_SOURCE_MIMETYPE_SBSAR))
	{
		SbsarFilename = downloadAttachment->getFilename();
	}


	FString CategoryString = FString(Asset->getCategory().c_str());

	//Create the string for the author and category display in the panel
	FString CenterString = "Category: ";
	CenterString += CategoryString;
	CenterString += FString(" - Author: ");
	CenterString += FString(Asset->getAuthor().c_str());

	AssetDetailsArea =
	    SNew(SVerticalBox)
	    + SVerticalBox::Slot()
	    [
	        SNew(SHorizontalBox)

	        //Create a slot for the categories
	        + SHorizontalBox::Slot()
	        .AutoWidth()
	        //.FillWidth(categoryWidth)
	        [
	            SNew(SBox)
	            .MinDesiredWidth(220.0f)
	            [
	                CategoryArea->AsShared()
	            ]
	        ]

	        //Create a slot for details menu
	        + SHorizontalBox::Slot()
	        .FillWidth(1.0f)
	        [
	            SNew(SVerticalBox)

	            //Create a slot for detail images
	            + SVerticalBox::Slot()
	            .FillHeight(0.85f)
	            .Padding(FMargin(6.0f, 6.0f, 6.0f, 0.0f))
	            [

	                SNew(SOverlay)
	                //Slot for the main details image slider
	                + SOverlay::Slot()
	                [
	                    DetailsDisplay->AsShared()
	                ]
	            ]

	            //Create a slot for detail footer
	            + SVerticalBox::Slot()
	            .AutoHeight()
	            .Padding(FMargin(7.0f, 0.0f, 6.0f, 6.0f))
	            [
	                SNew(SBox)
	                .MinDesiredHeight(150.0f)
	                [
	                    SNew(SBorder)
	                    //.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f))
	                    .BorderImage(FSubstanceSourceStyle::Get()->GetBrush("SourceDetailsBackground"))
	                    [
	                        SNew(SOverlay)
	                        + SOverlay::Slot()
	                        [
	                            SNew(SImage)
	                            .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceDetailsBackground"))
	                        ]
	                        + SOverlay::Slot()
	                        [
	                            SNew(SHorizontalBox)

	                            //Create a slot for the tags section
	                            + SHorizontalBox::Slot()
	                            .VAlign(VAlign_Bottom)
	                            .HAlign(HAlign_Left)
	                            .Padding(FMargin(10.0f, 0.0f, 0.0f, 10.0f))
	                            [
	                                SNew(SVerticalBox)
	                                .Clipping(EWidgetClipping::OnDemand)
	                                //Slot for title
	                                + SVerticalBox::Slot()
	                                .Padding(FMargin(6.0f, 10.0f, 0.0f, 0.0f))
	                                .VAlign(VAlign_Center)
	                                [
	                                    SNew(STextBlock)
	                                    .Text(FText::FromString(Asset->getTitle().c_str()))
	                                    .TextStyle(FSubstanceSourceStyle::Get(), "DetailsHeaderTop")
	                                ]
	                                //Slot for author and category
	                                + SVerticalBox::Slot()
	                                .Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
	                                .VAlign(VAlign_Center)
	                                [
	                                    SNew(SHorizontalBox)
	                                    + SHorizontalBox::Slot()
	                                    .HAlign(HAlign_Left)
	                                    .VAlign(VAlign_Center)
	                                    .AutoWidth()
	                                    [
	                                        SNew(STextBlock)
	                                        .Text(FText::FromString("Category: "))
	                                        .TextStyle(FSubstanceSourceStyle::Get(), "DetailsHeaderMiddle")
	                                    ]

	                                    //Slot for Category Hyperlink
	                                    + SHorizontalBox::Slot()
	                                    .HAlign(HAlign_Left)
	                                    .VAlign(VAlign_Center)
	                                    .AutoWidth()
	                                    [
	                                        SNew(SHyperlink)
	                                        .TextStyle(FSubstanceSourceStyle::Get(), "DetailsHeaderMiddle")
	                                        .Text(FText::FromString(FString(Asset->getCategory().c_str())))
	                                        .OnNavigate(this, &SSubstanceSourcePanel::OnCategoryHyperLink, CategoryString)
	                                    ]

	                                    + SHorizontalBox::Slot()
	                                    .HAlign(HAlign_Left)
	                                    .VAlign(VAlign_Center)
	                                    .AutoWidth()
	                                    [
	                                        SNew(STextBlock)
	                                        .Text(FText::FromString(" - Author: "))
	                                        .TextStyle(FSubstanceSourceStyle::Get(), "DetailsHeaderMiddle")
	                                    ]

	                                    //Slot for Author Hyperlink
	                                    + SHorizontalBox::Slot()
	                                    .HAlign(HAlign_Left)
	                                    .VAlign(VAlign_Center)
	                                    .AutoWidth()
	                                    [
	                                        SNew(SHyperlink)
	                                        .TextStyle(FSubstanceSourceStyle::Get(), "DetailsHeaderMiddle")
	                                        .Text(FText::FromString(FString(Asset->getAuthor().c_str())))
	                                        .OnNavigate(this, &SSubstanceSourcePanel::OnAuthorHyperLink, FString(Asset->getAuthor().c_str()))
	                                    ]

	                                ]
	                                //Slot for tags
	                                + SVerticalBox::Slot()
	                                [
	                                    SNew(SBox)
	                                    .MinDesiredHeight(75.0f)
	                                    .MaxDesiredHeight(75.0f)
	                                    [
	                                        Tags->AsShared()
	                                    ]
	                                ]
	                            ]

	                            //Create the area for the download button
	                            + SHorizontalBox::Slot()
	                            .HAlign(HAlign_Right)
	                            .AutoWidth()
	                            .Padding(FMargin(0.0f, 0.0f, 15.0f, 15.0f))
	                            [
	                                SNew(SOverlay)
	                                .Clipping(EWidgetClipping::ClipToBoundsAlways)
	                                + SOverlay::Slot()
	                                .VAlign(VAlign_Bottom)
	                                .HAlign(HAlign_Right)
	                                .Padding(FMargin(0.0f, 0.0f, 0.0f, 20.0f))
	                                [
	                                    SNew(SBox)
	                                    .MinDesiredWidth(160.0f)
	                                    .MinDesiredHeight(41.0f)
	                                    .MaxDesiredWidth(160.0f)
	                                    .MaxDesiredHeight(41.0f)
	                                    [
	                                        SNew(SButton)
	                                        .OnClicked(this, &SSubstanceSourcePanel::OnDownloadButton, Asset)
	                                        .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceDownloadButton")
	                                    ]
	                                ]
	                                + SOverlay::Slot()
	                                .VAlign(VAlign_Bottom)
	                                .Padding(FMargin(0.0f, 8.0f, 0.0f, 2.0f))
	                                [
	                                    SNew(STextBlock)
	                                    .Text(FText::FromString(SbsarFilename.c_str()))
	                                    .Justification(ETextJustify::Center)
	                                    .TextStyle(FSubstanceSourceStyle::Get(), "CategoryCountText")
	                                ]
	                            ]
	                        ]
	                    ]
	                ]
	            ]
	        ]
	    ]

	    //Slot for the download status bar
	    + SVerticalBox::Slot()
	    .MaxHeight(25.0f)
	    .FillHeight(0.1f)
	    .AutoHeight()
	    .Padding(0.0f, 2.0f)
	    [
	        SNew(SHorizontalBox)
	        + SHorizontalBox::Slot()
	        .Padding(2.0f, 0.0f)
	        [
	            StatusText->AsShared()
	        ]
	        + SHorizontalBox::Slot()
	        .FillWidth(0.4f)
	        .Padding(2.0f, 0.0f)
	        [
	            StatusProgress->AsShared()
	        ]

	    ];

	//Set the state flag
	SourcePanelViewState = eDetailView;

	//Remove all widgets from the panel
	ClearCurrentView();


	//Add the details area to the current view
	ChildSlot
	[
	    SNew(SOverlay)
	    + SOverlay::Slot()
	    [
	        SNew(SImage)
	        .Image(FSubstanceSourceStyle::Get()->GetBrush("SourcePanelBackground"))
	    ]
	    + SOverlay::Slot()
	    [
	        AssetDetailsArea.ToSharedRef()
	    ]
	];

	return FReply::Handled();
}

/** Creates all of the widgets for viewing assets (main view) */
FReply SSubstanceSourcePanel::SetLayoutTilesView()
{
	//Set the state flag
	SourcePanelViewState = eAssetTileView;

	//Remove the previous view
	ClearCurrentView();

	//Create the our base child widget to add widgets to
	TSharedPtr<SVerticalBox> ChildWidget =
	    SNew(SVerticalBox)
	    + SVerticalBox::Slot()
	    .FillHeight(0.9f)
	    [
	        SNew(SHorizontalBox)
	        + SHorizontalBox::Slot()
	        .AutoWidth()
	        .Padding(0.0f, 0.0f)
	        .MaxWidth(220.0f)
	        [
	            CategoryArea.ToSharedRef()
	        ]
	        + SHorizontalBox::Slot()
	        .FillWidth(1.0f)
	        .Padding(0.0f, 0.0f)
	        [
	            AssetTileBox.ToSharedRef()
	        ]
	    ]
	    + SVerticalBox::Slot()
	    .MaxHeight(25.0f)
	    .FillHeight(0.1f)
	    .AutoHeight()
	    .Padding(0.0f, 2.0f)
	    [
	        SNew(SHorizontalBox)
	        + SHorizontalBox::Slot()
	        .Padding(2.0f, 0.0f)
	        [
	            StatusText->AsShared()
	        ]
	        + SHorizontalBox::Slot()
	        .FillWidth(0.4f)
	        .Padding(2.0f, 0.0f)
	        [
	            StatusProgress->AsShared()
	        ]
	    ];

	//Add the root panel widget to this panels child slot
	ChildSlot
	[
	    SNew(SOverlay)
	    + SOverlay::Slot()
	    [
	        SNew(SImage)
	        .Image(FSubstanceSourceStyle::Get()->GetBrush("SourcePanelBackground"))
	    ]
	    + SOverlay::Slot()
	    [
	        ChildWidget.ToSharedRef()
	    ]
	];

	return FReply::Handled();
}

FReply SSubstanceSourcePanel::SetLayoutLoadingView()
{
	//Set the state flag
	SourcePanelViewState = eLoadingView;

	//Remove all widgets from the panel - There shouldn't be any initially but doing this as a precaution
	ClearCurrentView();

	TSharedPtr<SImage> StrongLoadingImage =
	    SNew(SImage)
	    .Image(FSubstanceSourceStyle::Get()->GetBrush("SourceAnimLoading1"));

	//Store a weak reference so we can animate the loading graphic
	LoadingImage = StrongLoadingImage;
	LoadingImageDeltaTimeMod = 0.0f;

	//Create the our base child widget to add widgets to
	TSharedPtr<SBox> ChildWidget =
	    SNew(SBox)
	    .VAlign(VAlign_Center)
	    .HAlign(HAlign_Center)
	    .MinDesiredWidth(FOptionalSize(500))
	    .MinDesiredHeight(FOptionalSize(500))
	    [
	        SNew(SVerticalBox)
	        + SVerticalBox::Slot()
	        [
	            StrongLoadingImage->AsShared()
	        ]
	    ];

	ChildSlot
	[
	    SNew(SOverlay)
	    + SOverlay::Slot()
	    [
	        SNew(SImage)
	        .Image(FSubstanceSourceStyle::Get()->GetBrush("SourcePanelBackground"))
	    ]
	    + SOverlay::Slot()
	    [
	        ChildWidget.ToSharedRef()
	    ]
	];

	return FReply::Handled();
}

/**Creates all of the widgets for the login screen view */
FReply SSubstanceSourcePanel::SetLayoutLoginView()
{
	//Set the state flag
	SourcePanelViewState = eLoginView;

	//Remove all widgets from the panel
	ClearCurrentView();

	//Create the our base child widget to add widgets to
	TSharedPtr<SHorizontalBox> ChildWidget =
	    SNew(SHorizontalBox)
	    + SHorizontalBox::Slot()
	    .VAlign(VAlign_Top)
	    .HAlign(HAlign_Center)
	    .Padding(0.0f, 60.0f, 0.0f, 0.0f)
	    [
	        SNew(SBox)
	        .HAlign(HAlign_Fill)
	        .VAlign(VAlign_Fill)
	        [
	            LoginArea->AsShared()
	        ]
	    ];

	ChildSlot
	[
	    SNew(SOverlay)
	    + SOverlay::Slot()
	    [
	        SNew(SImage)
	        .Image(FSubstanceSourceStyle::Get()->GetBrush("SourcePanelBackground"))
	    ]
	    + SOverlay::Slot()
	    [
	        ChildWidget.ToSharedRef()
	    ]
	];

	LoginSpinner->SetVisibility(EVisibility::Hidden);

	return FReply::Handled();
}

/** Clears the current view before adding a new view */
void SSubstanceSourcePanel::ClearCurrentView()
{
	//Detaches the widget from the parent. If the widget was valid, the shared pointer should lose all hard
	//references here and the smart pointer should be cleaned up / destroyed
	ChildSlot.DetachWidget();
}

/** Loops through all of the current category buttons and sets their styles to default */
void SSubstanceSourcePanel::ClearCurrentCategorySelection()
{
	for (const auto& CatWidgetItr : CurrentCategoryWidgets)
	{
		CatWidgetItr->mButtonWidget->SetButtonStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FButtonStyle>("SubstanceCategoryButton"));
		CatWidgetItr->mCategoryLabelText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryText"));
		CatWidgetItr->mCategoryCountText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryCountText"));
		CatWidgetItr->mCurrentlySelected = false;
	}
}

void SSubstanceSourcePanel::SelectCategoryButton()
{
	for (const auto& CatWidgetItr : CurrentCategoryWidgets)
	{
		if (CatWidgetItr->mCategoryLabelText->GetText().ToString() == CurrentSelectedCategory)
		{
			CatWidgetItr->mButtonWidget->SetButtonStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FButtonStyle>("SubstanceCategorySelectedButton"));
			CatWidgetItr->mCategoryLabelText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryTextSelected"));
			CatWidgetItr->mCategoryCountText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryCountTextSelected"));
			CatWidgetItr->mCurrentlySelected = true;
			break;
		}
	}
}

void SSubstanceSourcePanel::SetNewAssetVectorToLayout(const Alg::Source::AssetVector& AssetsToDisplay)
{
	//Switch back to tiles view if we aren't currently there
	if (SourcePanelViewState != eAssetTileView)
	{
		SetLayoutTilesView();
	}

	//Clear the selected category as we are no longer displaying its assets
	ClearCurrentCategorySelection();

	//Clear the old tile set
	CurrentTileSet.Empty();

	for (const auto& AssetItr : AssetsToDisplay)
	{
		//set desired image size
		if (Alg::Source::ImageAttachment* image = AssetItr->getImageAttachmentByLabel(SUBSTANCE_SOURCE_PREVIEW_IMAGE_THUMBNAIL))
		{
			image->setDesiredImageHeight(THUMBNAIL_HEIGHT);
			image->setDesiredImageWidth(THUMBNAIL_WIDTH);
		}

		//Create a new custom tile widget
		TSharedPtr<SSubstanceSourceAsset> SourceAsset =
		    SNew(SSubstanceSourceAsset)
		    .Label(AssetItr->getTitle().c_str())
		    .Asset(AssetItr)
		    .ParentWindow(SharedThis(this));

		CurrentTileSet.Add(SourceAsset);
	}

	//Recreate the menu with the new assets
	AssetTileArea->RebuildList();

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

}

/** Callback for when new assets are found from a search */
void SSubstanceSourcePanel::OnSearchedAssetsFound()
{
	Alg::Source::AssetVector SearchedAssets = ISubstanceSourceModule::Get()->GetSearchResults();

	FString SearchLabel = "Search (" + FString::FromInt(SearchedAssets.size()) + ")";
	AssetTileHeader_TagButtonText->SetText(FText::FromString(SearchLabel));
	SetCurrentSelectedCategory(FString(CATEGORY_TAGORSEARCH));

	SetNewAssetVectorToLayout(SearchedAssets);
}

/** Called to regenerate tile layout for MyAssetsPage */
void SSubstanceSourcePanel::RegenerateMyAssetList()
{
	Alg::Source::AssetVector CurrentAssets = ISubstanceSourceModule::Get()->GetUserOwnedAssets();

	// ...........................................................................................
	//The following code could be eliminated if the framework function getOwnedAssets(), called
	//within ISubstanceSourceModule::GetUserOwnedAssets(), could accept a "categoryName" argument
	//onto which all assets from other categories would be filtered out from the assets vector.

	Alg::Source::AssetVector MyAssetsVector;

	//If a specific My Assets category is selected; filter out all other categories:
	//At this point, 'CurrentSelectedCategory' is of the form "MyAssets - ..." where "..." is the actual category

	FString TargetCategoryName = CurrentSelectedCategory;
	TargetCategoryName.RemoveFromStart(FString(CATEGORY_MYASSETS));

	if (TargetCategoryName != FString(Alg::Source::Database::sCategoryAllAssets))
	{
		//Filter out all but one category:
		FString AssetCategoryName;

		for (auto AssetItr = CurrentAssets.rbegin(); AssetItr != CurrentAssets.rend(); ++AssetItr)
		{
			AssetCategoryName = FString((*AssetItr)->getCategory().c_str());

			if (AssetCategoryName == TargetCategoryName)
			{
				MyAssetsVector.push_back(*AssetItr);
			}
		}
	}
	else
	{
		MyAssetsVector = CurrentAssets;
		std::reverse(MyAssetsVector.begin(), MyAssetsVector.end());
	}

	//At this point; the assets are sorted by download date (youngest first)
	//Proceed to user selected sort:
	if (CurrentMyAssetsSortingItem->Equals(SORT_BY_RECENTLY_UPDATED_FIRST))
	{
		std::sort(MyAssetsVector.begin(), MyAssetsVector.end(), [](Alg::Source::AssetPtr const & A, Alg::Source::AssetPtr const & B)
		{
			const std::tm& a = A->getUpdatedAt();
			FDateTime dt1(a.tm_year + 1900, a.tm_mon + 1, a.tm_mday, a.tm_hour, a.tm_min, a.tm_sec, 0);

			const std::tm& b = B->getUpdatedAt();
			FDateTime dt2(b.tm_year + 1900, b.tm_mon + 1, b.tm_mday, b.tm_hour, b.tm_min, b.tm_sec, 0);

			return dt1.ToUnixTimestamp() < dt2.ToUnixTimestamp();
		});
	}
	else if (CurrentMyAssetsSortingItem->Equals(SORT_BY_NAME))
	{
		std::sort(MyAssetsVector.begin(), MyAssetsVector.end(), [](Alg::Source::AssetPtr const & A, Alg::Source::AssetPtr const & B)
		{
			return A->getTitle() < B->getTitle();
		});
	}

	SetNewAssetVectorToLayout(MyAssetsVector);

	if (AssetTileScrollBox.IsValid())
	{
		AssetTileScrollBox.Pin()->ScrollToStart();
	}
}

/** Callback to populate the asset tile view with the users assets */
void SSubstanceSourcePanel::OnDisplayMyAssets()
{
	//In the case we are already in my assets, scroll to top and don't regenerate
	if (SourcePanelViewState == eMyAssetView)
	{
		if (AssetTileScrollBox.IsValid())
		{
			AssetTileScrollBox.Pin()->ScrollToStart();
		}

		return;
	}

	if (!CurrentSelectedCategory.Contains(CATEGORY_MYASSETS))
	{
		//If not already in "My Assets" display: set "My Assets" default...
		MyAssetCategoryComboBox->ClearSelection();
		FString DefaultMyAssetsCategory = AssessMyAssetCategory(Alg::Source::Database::sCategoryAllAssets);
		CurrentMyAssetsCategoryItem = MyAssetsCategoryItems[0];
		SetCurrentSelectedCategory(DefaultMyAssetsCategory);
	}

	//Generate the asset list
	RegenerateMyAssetList();

	SourcePanelViewState = eMyAssetView;
}

/** A call back for when authentication passes and we can create the main view */
void SSubstanceSourcePanel::OnUserHasLoggedIn()
{
	//Event received from framework that we have logged in - Switch to "My Assets" tile view
	ConstructMyAssetsCategoryWidget();
	ConstructMyAssetsSortingWidget();

	CurrentMyAssetsCategoryItem = MyAssetsCategoryItems[0];
	CurrentMyAssetsSortingItem = MyAssetsSortingItems[0];

	ISubstanceSourceModule::Get()->DisplayUsersOwnedAssets();
}

/** Callback for when back-end wishes to display a specific category */
void SSubstanceSourcePanel::OnDisplayCategory(const FString& Category)
{
	OnCategorySwitched(Category);
}

/** Callback for when the user has successfully logged out */
void SSubstanceSourcePanel::OnUserHasLoggedOut()
{
	if (CurrentSelectedCategory.Contains(CATEGORY_MYASSETS))
	{
		OnCategorySwitched(Alg::Source::Database::sCategoryAllAssets);
	}
	else
	{
		//Refresh display
		PopulateAssetTiles();
	}
}

/** Callback for when a login attempt failed */
void SSubstanceSourcePanel::OnUserLoginFailed()
{
	//Hide loading icon
	LoginSpinner->SetVisibility(EVisibility::Hidden);
}

/** Callback for when the database has finished loading */
void SSubstanceSourcePanel::OnSourceLoadComplete()
{
	//Setup!
	PopulateCategories();
	PopulateAssetTiles();
	SetLayoutTilesView();
	SelectCategoryButton();
}

/** Called when the user submits the text in the password input */
void SSubstanceSourcePanel::OnPasswordSubmitted(const FText& CommitText, ETextCommit::Type CommitType)
{
	switch (CommitType)
	{
	//Don't commit text of mouse focus leave or when the text is cleared
	case ETextCommit::OnUserMovedFocus:
	case ETextCommit::OnCleared:
	case ETextCommit::Default:
		break;
	//Attempt login on all other commits, including: ETextCommit::OnEnter
	default:
		OnLoginAttempted();
		break;
	}
}

/** Opens a hyperlink to the given address */
void SSubstanceSourcePanel::OnOpenHyperlink(FString LinkName)
{
	FPlatformProcess::LaunchURL(*LinkName, nullptr, nullptr);
}

FReply SSubstanceSourcePanel::OnOpenHyperlinkButton(FString LinkName)
{
	FPlatformProcess::LaunchURL(*LinkName, nullptr, nullptr);
	return FReply::Handled();
}


/** Called to switch categories when the hyperlink is pressed from the details panel */
void SSubstanceSourcePanel::OnCategoryHyperLink(FString NewCategoryName)
{
	OnCategorySwitched(NewCategoryName);
}

/** Called when the author hyper link is pressed from the details panel */
void SSubstanceSourcePanel::OnAuthorHyperLink(FString Author)
{
	ISubstanceSourceModule::Get()->SearchStringChanged(TCHAR_TO_ANSI(*Author));

	Alg::Source::AssetVector SearchedAssets = ISubstanceSourceModule::Get()->GetSearchResults();
	FString SearchCountLabel = " (" + FString::FromInt(SearchedAssets.size()) + ")";
	AssetTileHeader_TagButtonText->SetText(FText::FromString(Author + SearchCountLabel));
	SetCurrentSelectedCategory(FString(CATEGORY_TAGORSEARCH));
}

/** Callback when load is complete and we can switch to the main initial view */
void SSubstanceSourcePanel::OnFinishedLoadingDatabase()
{
	//Remove all widgets from the panel
	ClearCurrentView();

	//Set to the default tile view.
	SetLayoutTilesView();
}

/** Adds the tiles in to a uniform gird */
TSharedRef<ITableRow> SSubstanceSourcePanel::CreateContentSourceIconTile(TSharedPtr<SSubstanceSourceAsset> ContentSource, const TSharedRef<STableViewBase>& OwnerTable)
{
	Alg::Source::Asset* CurrentSourceAsset = ContentSource->GetAsset().get();
	return SNew(STableRow< TSharedPtr<FString>>, OwnerTable)
	       .Style(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTableRowStyle>("SubstanceTableViewRow"))
	       .Padding(FMargin(2.0f, 0.0f, 2.0f, 25.0f))
	       [
	           ContentSource->AsShared()
	       ];
}

void SSubstanceSourcePanel::ButtonDetails::ButtonHovered()
{
	if (mCategoryCountText.IsValid() && mCategoryLabelText.IsValid())
	{
		mCategoryCountText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryCountTextHovered"));
		mCategoryLabelText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryTextHovered"));
	}
}

void SSubstanceSourcePanel::ButtonDetails::ButtonUnHovered()
{
	if (mCategoryCountText.IsValid() && mCategoryLabelText.IsValid() && !mCurrentlySelected)
	{
		mCategoryCountText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryCountText"));
		mCategoryLabelText->SetTextStyle(&FSubstanceSourceStyle::Get()->GetWidgetStyle<FTextBlockStyle>("CategoryText"));
	}
}
