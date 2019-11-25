// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourcePanel.h

/** Description:: This class will be used for filling out the main viewport of the source window.
This will manage the UI and all of the interface functionality not directly related to the
window itself. */

#pragma once
#include "SubstanceSourceModule.h"
#include "SubstanceSourceWindow.h"
#include "SubstanceSourceAsset.h"
#include "SubstanceSourceStyle.h"

#include "SEditorViewport.h"
#include "SubstanceToolkit.h"
#include "ISubstanceSource.h"
#include <substance/source/database.h>

// Templated forward declarations
template <typename ItemType>
class STileView;

template< typename OptionType >
class SComboBox;

// Forward Declarations
class SBox;
class SButton;
class SWidget;
class SImage;
class STextBlock;
class SCheckBox;
class SProgressBar;
class SVerticalBox;
class SCircularThrobber;
class SEditableTextBox;
class FSubstanceSourceModule;


class SSubstanceSourcePanel : public SCompoundWidget
{
public:
	/** Slate arguments */
	SLATE_BEGIN_ARGS(SSubstanceSourcePanel) = default;
	SLATE_END_ARGS()

	/** Flags for which view state this panel is in*/
	enum ViewState { eLoadingView, eLoginView, eAssetTileView, eDetailView, eMyAssetView };

	/** SCompoundWidget interface */
	void Construct(const FArguments& InArgs);

	/** Destructor */
	virtual ~SSubstanceSourcePanel() override;

	/** Tick geometry */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Stores a pointer parent window */
	void SetParentWindow(TWeakPtr<FSubstanceSourceWindow> ParentWindow);

	/** Get the substance editor effected by this panel */
	TWeakPtr<FSubstanceSourceWindow> GetParentWindow() const;

	/** TODO:: This should be moved into tiles when confirmed working */
	TSharedRef<ITableRow> CreateContentSourceIconTile(TSharedPtr<SSubstanceSourceAsset> ContentSource, const TSharedRef<STableViewBase>& OwnerTable);

	/** Panel Callbacks */
	void OnFinishedLoadingDatabase();

	/** Stores the selected category label as a class member */
	void SetCurrentSelectedCategory(const FString& CategoryName);

	/** Switches the state of the current panel to a different view */
	FReply SetLayoutDetailView(Alg::Source::AssetPtr Asset);
	FReply SetLayoutLoadingView();
	FReply SetLayoutTilesView();
	FReply SetLayoutLoginView();

	/** Callback for when the details window has been closed through the close button */
	FReply OnCloseDetailsWindow();

private:
	/** Sizes of the thumbnails */
	static const int32 THUMBNAIL_WIDTH;
	static const int32 THUMBNAIL_HEIGHT;
	static const int32 MIN_SOURCEASSET_WIDTH;
	static const int32 BASE_SOURCEASSET_WIDTH;
	static const int32 BASE_SOURCEASSET_HEIGHT;
	static const int32 LOADINGIMAGE_FRAMES_PER_SECOND;
	static const char* CATEGORY_MYASSETS;
	static const char* CATEGORY_TAGORSEARCH;

	static const char* SORT_BY_RECENTLY_UPDATED_FIRST;
	static const char* SORT_BY_DOWNLOAD_DATE;
	static const char* SORT_BY_NAME;
	static const char* MY_ASSETS_SORTS[];

	/** Used to store all of the elements we will need to switch selected button*/
	struct ButtonDetails : public TSharedFromThis<ButtonDetails>
	{
		TSharedPtr<SButton> mButtonWidget;
		TSharedPtr<STextBlock> mCategoryLabelText;
		TSharedPtr<STextBlock> mCategoryCountText;
		bool mCurrentlySelected;

		ButtonDetails() : mCurrentlySelected(false) {};

		void ButtonHovered();
		void ButtonUnHovered();
	};

	/** Builder functions for creating the areas */
	void ConstructCategories();
	void ConstructAssetTiles();
	void ConstructUsersLogin();
	void ConstructDetailArea();
	void ConstructLoadStatus();

	/** UI layouts once the database is loaded */
	void PopulateCategories();
	void PopulateAssetTiles(bool ShouldResetScroll = true);

	/** Removes all of the widgets from the current panel */
	void ClearCurrentView();
	void ClearCurrentCategorySelection();
	void SelectCategoryButton();

	/** Called when the MyAssets page is either displayed or sorted */
	void RegenerateMyAssetList();

	/** Populates the tile view with a new asset vector */
	void SetNewAssetVectorToLayout(const Alg::Source::AssetVector& AssetsToDisplay);

	/** Call backs for when a category button is pressed */
	FReply OnCategorySwitched(FString NewCategoryName);
	FReply OnDisplayMainCategories();
	FReply OnLoginAttempted();
	FReply OnDownloadButton(Alg::Source::AssetPtr Asset);
	FReply OnSearchTag(FString TagToSearch);
	FReply OnOpenHyperlinkButton(FString LinkName);

	/** Authentication callbacks */
	void OnSearchedAssetsFound();
	void OnDisplayMyAssets();
	void OnDisplayCategory(const FString& Category);
	void OnUserHasLoggedIn();
	void OnUserHasLoggedOut();
	void OnUserLoginFailed();
	void OnSourceLoadComplete();
	void OnPasswordSubmitted(const FText& CommentText, ETextCommit::Type CommitInfo);
	void OnOpenHyperlink(FString LinkName);
	void OnCategoryHyperLink(FString NewCategoryName);
	void OnAuthorHyperLink(FString Author);

	/** Handles clearing up the framework memory before our destructors are lost */
	void OnModuleShutdown();

	/** Flags for which state we are currently in */
	ViewState SourcePanelViewState;

	/** Flag for if the database is loaded - TODO:: Is this still needed with the callbacks? */
	bool bDatabaseLoaded;

	/** Delta Time storage for loading spinner graphic */
	float LoadingImageDeltaTimeMod;

	/** Reference to our parent window*/
	TWeakPtr<FSubstanceSourceWindow> ParentWindowPtr;

	/** Pointers to the areas of the main window */
	TSharedPtr<SEditableTextBox> UsernameInput;
	TSharedPtr<SEditableTextBox> PasswordInput;
	TSharedPtr<SCircularThrobber> LoginSpinner;
	TSharedPtr<SVerticalBox> AssetDetailsArea;
	TSharedPtr<SVerticalBox> CategoryArea;
	TSharedPtr<SBox> AssetTileBox;
	TSharedPtr<SBox> AssetTileHeader_MyAssets;
	TSharedPtr<SBox> AssetTileHeader_Tag;
	TSharedPtr<SButton> AssetTileHeader_TagButton;
	TSharedPtr<STextBlock> AssetTileHeader_TagButtonText;
	TSharedPtr<STileView<TSharedPtr<SSubstanceSourceAsset>>> AssetTileArea;
	TSharedPtr<SWidget> DetailsDisplay;
	TSharedPtr<SBox> LoginArea;
	TSharedPtr<SCheckBox> EULAAcceptedCheckbox;
	TWeakPtr<SImage> LoadingImage;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> MyAssetSortingComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> MyAssetCategoryComboBox;
	TArray<TSharedPtr<FString>> MyAssetsSortingItems;
	TSharedPtr<FString> CurrentMyAssetsSortingItem;
	TArray<TSharedPtr<FString>> MyAssetsCategoryItems;
	TSharedPtr<FString> CurrentMyAssetsCategoryItem;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SProgressBar> StatusProgress;
	TWeakPtr<class SSubstanceScrollBox> AssetTileScrollBox;

	/** Array of sources used for asset tiles - Cleared whenever the user switches views. */
	TArray<TSharedPtr<class SSubstanceSourceAsset>> CurrentTileSet;

	/** Attribute value for if user has accepted EULA*/
	TAttribute<ECheckBoxState> EULAAccepted;

	/** Used to store which category is currently selected */
	FString CurrentSelectedCategory;

	/** Container for referencing all of the category buttons */
	TArray<TSharedPtr<ButtonDetails>> CurrentCategoryWidgets;

	/** My Asset's "sort" combobox list */
	void ConstructMyAssetsSortingWidget();
	TSharedRef<SWidget> OnGenerateMyAssetsSortingWidget(TSharedPtr<FString> InItem);
	void OnMyAssetsSortingSelection(TSharedPtr<FString> pSelectedItem, ESelectInfo::Type pSelectInfo);
	FText GetMyAssetsSortingLabel() const;

	/** My Asset's category combobox list */
	void ConstructMyAssetsCategoryWidget();
	TSharedRef<SWidget> OnGenerateMyAssetsCategoryWidget(TSharedPtr<FString> InItem);
	void OnMyAssetsCategorySelection(TSharedPtr<FString> pSelectedItem, ESelectInfo::Type pSelectInfo);
	FText GetMyAssetsCategoryLabel() const;

	// Utility that builds the "MyAsset" category name
	FString AssessMyAssetCategory(const FString& Category);
};
