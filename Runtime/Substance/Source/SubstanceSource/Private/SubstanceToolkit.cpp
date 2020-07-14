// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceToolkit.cpp

#include "SubstanceToolkit.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceToolkitHost.h"
#include "SubstanceSourceStyle.h"

#include "Toolkits/ToolkitManager.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "Interfaces/IMainFrameModule.h"
#include "EditorStyleSet.h"
#include "WorkspaceMenuStructureModule.h"
#include "Styling/SlateIconFinder.h"
#include "CollectionManagerModule.h"
#include "IIntroTutorials.h"
#include "AssetEditorModeManager.h"
#include "SubstanceSourceModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "SubstanceSourceModule"

const FName FSubstanceToolkit::ToolbarTabId(TEXT("SubstanceSourceToolkit_Toolbar"));

/** Constructs all of the widgets the window will use and launches the window */
void FSubstanceToolkit::InitAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const FName AppIdentifier, const TSharedRef<FTabManager::FLayout>& StandaloneDefaultLayout, const bool bCreateDefaultStandaloneMenu, const bool bCreateDefaultToolbar, const bool bInIsToolbarFocusable /*= false*/)
{
	//Bind the callback events
	ISubstanceSourceModule::Get()->OnLoggedIn().AddSP(this, &FSubstanceToolkit::OnUserHasLoggedIn);
	ISubstanceSourceModule::Get()->OnLoggedOut().AddSP(this, &FSubstanceToolkit::OnUserHasLoggedOut);
	ISubstanceSourceModule::Get()->OnLoginFailed().AddSP(this, &FSubstanceToolkit::OnSourceLoadComplete);
	ISubstanceSourceModule::Get()->OnDatabaseLoaded().AddSP(this, &FSubstanceToolkit::OnSourceLoadError);
	ISubstanceSourceModule::Get()->OnCategoryChanged().AddSP(this, &FSubstanceToolkit::OnCategorySwitched);
	ISubstanceSourceModule::Get()->OnSubstancePurchased().AddSP(this, &FSubstanceToolkit::OnAssetPurchased);

	//Create the widgets for this class
	ConstructBaseWidgets();

	// Open a standalone app to edit this asset.
	check(AppIdentifier != NAME_None);

	//Save local reference of our manager
	FToolkitManager& ToolkitManager = FToolkitManager::Get();

	//Store input parameters into members
	bIsToolbarFocusable = bInIsToolbarFocusable;
	ToolkitMode = Mode;

	//Set up smart pointer to new objects
	TSharedPtr<SSubstanceSourceToolkitHost> NewSubstanceHost;
	TSharedPtr<SDockTab> NewMajorTab;

	//Create the label and the link for the toolkit documentation.
	TAttribute<FText> Label = TAttribute<FText>(this, &FSubstanceToolkit::GetToolkitName);
	TAttribute<FText> ToolTipText = TAttribute<FText>(this, &FSubstanceToolkit::GetToolkitToolTipText);

	//Create the tab
	FName CurID = TEXT("StandaloneToolkit");

	NewMajorTab = SNew(SDockTab)
	              .ContentPadding(0.0f)
	              .TabRole(ETabRole::MajorTab)
	              .Label(Label);

	const TSharedRef<FTabManager> NewTabManager = FGlobalTabmanager::Get()->NewTabManager(NewMajorTab.ToSharedRef());
	NewTabManager->SetOnPersistLayout(FTabManager::FOnPersistLayout::CreateRaw(this, &FSubstanceToolkit::HandleTabManagerPersistLayout));
	this->TabManager = NewTabManager;

	NewMajorTab->SetContent
	(
	    SAssignNew(NewSubstanceHost, SSubstanceSourceToolkitHost, NewTabManager, AppIdentifier)
	    .OnRequestClose(this, &FSubstanceToolkit::OnRequestClose)
	);

	ToolkitHost = NewSubstanceHost;
	ToolkitManager.RegisterNewToolkit(SharedThis(this));

	if (ToolkitMode == EToolkitMode::Standalone)
	{
		TSharedRef<FTabManager::FLayout> LayoutToUse = StandaloneDefaultLayout; // FLayoutSaveRestore::LoadFromConfig(GEditorLayoutIni, StandaloneDefaultLayout);

		// Actually create the widget content
		NewSubstanceHost->SetupInitialContent(LayoutToUse, NewMajorTab, bCreateDefaultStandaloneMenu);
	}

	FName PlaceholderId(TEXT("StandaloneToolkit"));
	FTabManager::FSearchPreference* SearchPreference = new FTabManager::FRequireClosedTab();
	FGlobalTabmanager::Get()->InsertNewDocumentTab(PlaceholderId, *SearchPreference, NewMajorTab.ToSharedRef());
	delete SearchPreference;

	SubstanceHost = NewSubstanceHost;

	if (bCreateDefaultToolbar)
	{
		GenerateToolbar();
	}
}

/** Call back to spawn the toolbar when a tab is opened */
TSharedRef<SDockTab> FSubstanceToolkit::SpawnTab_Toolbar(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ToolbarTabId);

	TSharedRef<SDockTab> DockTab =
	    SNew(SDockTab)
	    .Label(NSLOCTEXT("Toolbar", "Toolbar_TabTitle", "Toolbar"))
	    .Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Toolbar"))
	    .ShouldAutosize(true)
	    [
	        SAssignNew(ToolbarWidgetContent, SVerticalBox)
	    ];

	if (Toolbar.IsValid())
	{
		ToolbarWidgetContent->AddSlot()
		[
		    Toolbar.ToSharedRef()
		];
	}

	return DockTab;
}

/** Register the spawner that will be used to create the tabs */
void FSubstanceToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& TM)
{
	//Use the first child category of the local workspace root if there is one, otherwise use the root itself
	const auto& LocalCategories = TM->GetLocalWorkspaceMenuRoot()->GetChildItems();
	TSharedRef<FWorkspaceItem> ToolbarSpawnerCategory = LocalCategories.Num() > 0 ? LocalCategories[0] : TM->GetLocalWorkspaceMenuRoot();

	TM->RegisterTabSpawner(ToolbarTabId, FOnSpawnTab::CreateSP(this, &FSubstanceToolkit::SpawnTab_Toolbar))
	.SetDisplayName(LOCTEXT("ToolbarTab", "Toolbar"))
	.SetGroup(ToolbarSpawnerCategory)
	.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Toolbar.Icon"));
}

/** Clean up the spawner when no longer needed */
void FSubstanceToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& TM)
{
	TM->UnregisterTabSpawner(ToolbarTabId);
	TM->ClearLocalWorkspaceMenuCategories();
}

FText FSubstanceToolkit::GetToolkitName() const
{
	return FText::FromString("Substance Source");
}

FText FSubstanceToolkit::GetToolkitToolTipText() const
{
	return FText::FromString("Substance Source");
}

class FEdMode* FSubstanceToolkit::GetEditorMode() const
{
	return nullptr;
}

FName FSubstanceToolkit::GetEditorName() const
{
	return FName("SubstanceToolkit");
}

void FSubstanceToolkit::FocusWindow(UObject* ObjectToFocusOn /*= nullptr*/)
{
	BringToolkitToFront();
}

bool FSubstanceToolkit::CloseWindow()
{
	if (OnRequestClose())
	{
		FToolkitManager::Get().CloseToolkit(AsShared());
	}
	return true;
}

void FSubstanceToolkit::InvokeTab(const FTabId& TabId)
{
	GetTabManager()->InvokeTab(TabId);
}

TSharedPtr<FTabManager> FSubstanceToolkit::GetAssociatedTabManager()
{
	return TSharedPtr<FTabManager>();
}

void FSubstanceToolkit::FillDefaultFileMenuCommands(FMenuBuilder& MenuBuilder)
{
}

void FSubstanceToolkit::FillDefaultAssetMenuCommands(FMenuBuilder& MenuBuilder)
{
}

void FSubstanceToolkit::FillDefaultHelpMenuCommands(FMenuBuilder& MenuBuilder)
{
}

bool FSubstanceToolkit::IsAssetEditor() const
{
	return false;
}

/** This is override that we will never use as we are not using the toolkit for asset editing purposes */
const TArray< UObject* >* FSubstanceToolkit::GetObjectsCurrentlyBeingEdited() const
{
	return nullptr;
}

/** A call back for when authentication passes and we can create the main view */
void FSubstanceToolkit::OnUserHasLoggedIn()
{
	GenerateToolbar();
}

/** Callback for when the user has successfully logged out */
void FSubstanceToolkit::OnUserHasLoggedOut()
{
	GenerateToolbar();
}

/** Callback for when the database has finished loading */
void FSubstanceToolkit::OnSourceLoadComplete()
{
	//TODO:: Call generate toolbar here once we clear the toolbar pre load.
	//Currently the toolbar is filled out by default and should only add widgets post load to prevent users
	//Clicking buttons and causing actions we don't want to occur until everything is completely loaded
}

/** Callback for when the source database load has failed! */
void FSubstanceToolkit::OnSourceLoadError()
{
	//TODO:: Shutdown the window host gracefully
}

void FSubstanceToolkit::OnCategorySwitched()
{
	SearchBar->SetText(FText::GetEmpty());
}

/** Called to regenerate the toolbar so that the new available download count is refreshed and displayed */
void FSubstanceToolkit::OnAssetPurchased(Alg::Source::AssetPtr PurchasedAsset)
{
	RegenerateToolBar();
}

/** Generate the toolbar according to state of the framework and the main panel */
void FSubstanceToolkit::GenerateToolbar()
{
	FToolBarBuilder ToolbarBuilder(GetToolkitCommands(), FMultiBoxCustomization::AllowCustomization(GetToolkitFName()));
	ToolbarBuilder.SetIsFocusable(bIsToolbarFocusable);
	ToolbarBuilder.BeginSection("Asset");
	{
		ToolbarBuilder.AddToolBarButton(FGlobalEditorCommonCommands::Get().FindInContentBrowser, NAME_None, LOCTEXT("FindInContentBrowserButton", "Find in CB"));
	}
	ToolbarBuilder.EndSection();

	//Check with the database if the user is authenticated
	bool UserIsLoggedIn = false;
	if (ISubstanceSourceModule::Get()->GetUserLoginStatus() == ESourceUserLoginStatus::LoggedIn)
	{
		UserIsLoggedIn = true;
	}

	if (UserIsLoggedIn)
	{
		FString DownloadCount = FString::FromInt(ISubstanceSourceModule::Get()->GetUserAvailableDownloads());
		TSharedPtr<SButton> CurrentLogButton = LogoutButton;

		Toolbar =
		    SNew(SHorizontalBox)
		    + SHorizontalBox::Slot()
		    [
		        SNew(SGridPanel)
		        .FillColumn(1, 1.0f)

		        //Add a slot for home
		        + SGridPanel::Slot(0, 0)
		        .HAlign(HAlign_Left)
		        [
		            SNew(SOverlay)
		            + SOverlay::Slot()
		            .Padding(FMargin(0.0f, 0.0f, 20.0f, 0.0f))
		            [
		                SNew(SBox)
		                .MinDesiredWidth(152.0f)
		                .MinDesiredHeight(60.0f)
		                .MaxDesiredWidth(152.0f)
		                .MaxDesiredHeight(60.0f)
		                [
		                    SubstanceButton->AsShared()
		                ]
		            ]

		            + SOverlay::Slot()
		            .HAlign(HAlign_Right)
		            [
		                SNew(SColorBlock)
		                .Color(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		                .Size(FVector2D(2.0f, 60.0f))
		            ]
		        ]

		        //Add a slot for search
		        + SGridPanel::Slot(1, 0)
		        [
		            SNew(SOverlay)
		            + SOverlay::Slot()
		            [
		                SearchBar->AsShared()
		            ]

		            + SOverlay::Slot()
		            .HAlign(HAlign_Right)
		            .Padding(FMargin(0.0f, 0.0f, 13.0f, 0.0f))
		            [
		                SNew(SOverlay)

		                //Slot for the download number
		                + SOverlay::Slot()
		                .VAlign(VAlign_Center)
		                .Padding(FMargin(0.0f, 1.0f, 5.0f, 0.0f))
		                [
		                    SNew(STextBlock)
		                    .Text(FText::FromString(DownloadCount))
		                    .TextStyle(FSubstanceSourceStyle::Get(), "CategoryText")
		                ]

		                //Slot for available download label
		                + SOverlay::Slot()
		                .Padding(FMargin(35.0f, 0.0f, 0.0f, 0.0f))
		                .VAlign(VAlign_Center)
		                [
		                    SNew(SBox)
		                    .VAlign(VAlign_Center)
		                    .MinDesiredWidth(173.0f)
		                    .MinDesiredHeight(51.0f)
		                    .MaxDesiredWidth(173.0f)
		                    .MaxDesiredHeight(51.0f)
		                    [
		                        SNew(SImage)
		                        .Image(FSubstanceSourceStyle::Get()->GetBrush("SubstanceAvailableDownloads"))
		                    ]
		                ]
		            ]

		            + SOverlay::Slot()
		            .HAlign(HAlign_Right)
		            [
		                SNew(SColorBlock)
		                .Color(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		                .Size(FVector2D(2.0f, 60.0f))
		            ]
		        ]

		        //Add a slot for my assets
		        + SGridPanel::Slot(2, 0)
		        [
		            SNew(SOverlay)
		            + SOverlay::Slot()
		            [
		                SNew(SBox)
		                .MinDesiredWidth(111.0f)
		                .MinDesiredHeight(60.0f)
		                .MaxDesiredWidth(111.0f)
		                .MaxDesiredHeight(60.0f)
		                [
		                    MyAssetsButton->AsShared()
		                ]
		            ]

		            + SOverlay::Slot()
		            .HAlign(HAlign_Right)
		            [
		                SNew(SColorBlock)
		                .Color(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		                .Size(FVector2D(2.0f, 60.0f))
		            ]
		        ]

		        //Add a slot for the log in/out button
		        + SGridPanel::Slot(3, 0)
		        .HAlign(HAlign_Right)
		        [
		            SNew(SBox)
		            .HAlign(HAlign_Center)
		            .MinDesiredWidth(140.0f)
		            .MinDesiredHeight(60.0f)
		            .MaxDesiredWidth(140.0f)
		            .MaxDesiredHeight(60.0f)
		            [
		                CurrentLogButton->AsShared()
		            ]
		        ]
		    ];
	}
	else
	{
		//The user is logged in an we need to create the proper bar
		TSharedPtr<SButton> CurrentLogButton = LoginButton;

		Toolbar =
		    SNew(SHorizontalBox)
		    + SHorizontalBox::Slot()
		    [
		        SNew(SGridPanel)
		        .FillColumn(1, 1.0f)

		        //Add a slot for home
		        + SGridPanel::Slot(0, 0)
		        .HAlign(HAlign_Left)
		        [
		            SNew(SOverlay)
		            + SOverlay::Slot()
		            .Padding(FMargin(0.0f, 0.0f, 20.0f, 0.0f))
		            [
		                SNew(SBox)
		                .MinDesiredWidth(152.0f)
		                .MinDesiredHeight(60.0f)
		                .MaxDesiredWidth(152.0f)
		                .MaxDesiredHeight(60.0f)
		                [
		                    SubstanceButton->AsShared()
		                ]
		            ]

		            + SOverlay::Slot()
		            .HAlign(HAlign_Right)
		            [
		                SNew(SColorBlock)
		                .Color(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		                .Size(FVector2D(2.0f, 60.0f))
		            ]
		        ]

		        //Add a slot for search
		        + SGridPanel::Slot(1, 0)
		        [
		            SNew(SOverlay)
		            + SOverlay::Slot()
		            [
		                SearchBar->AsShared()
		            ]

		            + SOverlay::Slot()
		            .HAlign(HAlign_Right)
		            [
		                SNew(SColorBlock)
		                .Color(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		                .Size(FVector2D(2.0f, 60.0f))
		            ]
		        ]

		        //Add a slot for the log in/out button
		        + SGridPanel::Slot(2, 0)
		        .HAlign(HAlign_Right)
		        [
		            SNew(SBox)
		            .HAlign(HAlign_Center)
		            .MinDesiredWidth(140.0f)
		            .MinDesiredHeight(60.0f)
		            .MaxDesiredWidth(140.0f)
		            .MaxDesiredHeight(60.0f)
		            [
		                CurrentLogButton->AsShared()
		            ]
		        ]
		    ];
	}

	if (ToolbarWidgetContent.IsValid())
	{
		ToolbarWidgetContent->ClearChildren();
		ToolbarWidgetContent->AddSlot()
		[
		    SNew(SOverlay)
		    + SOverlay::Slot()
		    [
		        SNew(SImage)
		        .Image(FSubstanceSourceStyle::Get()->GetBrush("SubstanceHeaderbarBackground"))
		    ]
		    + SOverlay::Slot()
		    [
		        Toolbar.ToSharedRef()
		    ]
		];
	}
}

/** Callback for when the menu bar login button has been pressed */
FReply FSubstanceToolkit::OnLoginPressed()
{
	return FReply::Handled();
}

/** Callback for when the user has pressed the logout button. */
FReply FSubstanceToolkit::OnLogoutPressed()
{
	ISubstanceSourceModule::Get()->UserLogout();
	return FReply::Handled();
}

/** Callback for when the home button is pressed to return to the tile asset view */
FReply FSubstanceToolkit::OnSubstancePressed()
{
	return FReply::Handled();
}

FReply FSubstanceToolkit::OnMyAssetsPressed()
{
	ISubstanceSourceModule::Get()->DisplayUsersOwnedAssets();
	return FReply::Handled();
}

void FSubstanceToolkit::EnableLoginButton()
{
	LoginButton->SetEnabled(true);
	SearchBar->SetEnabled(true);
}

/** Create the persistent / core widgets for the base window */
void FSubstanceToolkit::ConstructBaseWidgets()
{
	//Create Login Button (will be enabled once the database is loaded)
	LoginButton =
	    SNew(SButton)
	    .IsEnabled(ISubstanceSourceModule::Get()->GetDatabaseLoadingStatus() == ESourceDatabaseLoadingStatus::Loaded)
	    .OnClicked(this, &FSubstanceToolkit::OnLoginPressed)
	    .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceLoginButton");

	//Create Logout Button
	LogoutButton =
	    SNew(SButton)
	    .VAlign(VAlign_Center)
	    .HAlign(HAlign_Center)
	    .OnClicked(this, &FSubstanceToolkit::OnLogoutPressed)
	    .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceLogoutButton");

	//Create Substance Home Button
	SubstanceButton =
	    SNew(SButton)
	    .OnClicked(this, &FSubstanceToolkit::OnSubstancePressed)
	    .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceHomeButton");


	//Create the search bar
	SearchBar =
	    SNew(SEditableTextBox)
	    .IsEnabled(ISubstanceSourceModule::Get()->GetDatabaseLoadingStatus() == ESourceDatabaseLoadingStatus::Loaded)
	    .Style(FSubstanceSourceStyle::Get(), "SourceSearch")
	    //.MinDesiredWidth(100.0f)
	    //TODO:: Padding is for text and will allow us to add magnifying glass
	    .HintText(FText::FromString("SEARCH"))
	    .OnTextChanged(this, &FSubstanceToolkit::OnSearchTextChanged);

	//Create the my assets button
	MyAssetsButton =
	    SNew(SButton)
	    .OnClicked(this, &FSubstanceToolkit::OnMyAssetsPressed)
	    .ButtonStyle(FSubstanceSourceStyle::Get(), "SubstanceMyAssetsButton");
}

void FSubstanceToolkit::OnSearchTextChanged(const FText& NewSearchValue)
{
	//Get proper string format and let the module know we are searching
	ISubstanceSourceModule::Get()->SearchStringChanged(NewSearchValue.ToString());
}

/** Regenerate the tool bar and menu to add different widgets based on if the user has logged in or is logged out */
void FSubstanceToolkit::RegenerateMenusAndToolbars()
{
	SubstanceHost.Pin()->GenerateMenus(false);

	if (Toolbar != SNullWidget::NullWidget)
	{
		GenerateToolbar();
	}

	PostRegenerateMenusAndToolbars();
}

/** Regenerate the tool bar to add different widgets based on if the user has logged in or is logged out */
void FSubstanceToolkit::RegenerateToolBar()
{
	if (Toolbar != SNullWidget::NullWidget)
	{
		GenerateToolbar();
	}
}

/** Assigns the menu bar widget to the toolkit window */
void FSubstanceToolkit::SetMenuOverlay(TSharedRef<SWidget> Widget)
{
	SubstanceHost.Pin()->SetMenuOverlay(Widget);
}

UEdMode* FSubstanceToolkit::GetScriptableEditorMode() const
{
	return nullptr;
}

FText FSubstanceToolkit::GetEditorModeDisplayName() const
{
	return FText::GetEmpty();
}

FSlateIcon FSubstanceToolkit::GetEditorModeIcon() const
{
	return FSlateIcon();
}

/** Returns the icon that will be displayed on the index tab */
const FSlateBrush* FSubstanceToolkit::GetDefaultTabIcon() const
{
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
