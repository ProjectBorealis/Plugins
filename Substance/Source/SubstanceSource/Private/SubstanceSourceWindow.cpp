// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceWindow.cpp

#include "SubstanceSourceWindow.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceSourceModule.h"
#include "SubstanceSourcePanel.h"

#include "EditorStyleSet.h"
#include "ScopedTransaction.h"
#include "Interfaces/IMainFrameModule.h"
#include "DesktopPlatformModule.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/SlateIconFinder.h"
#include "Templates/SharedPointer.h"
#include "Editor/EditorEngine.h"

#include "Widgets/Docking/SDockableTab.h"
#include "Widgets/Docking/SDockTab.h"

#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "SubstanceSourceWindow"

/** ID for this tab */
const FName FSubstanceSourceWindow::SubstanceSourceTabId(TEXT("SubstanceSourceWindowID"));

/** Substance Source Commands*/
class FSubstanceSourceCommands : public TCommands<FSubstanceSourceCommands>
{
public:

	/** Constructor */
	FSubstanceSourceCommands()
		: TCommands<FSubstanceSourceCommands>(
		      "SubstanceSource",
		      NSLOCTEXT("Contexts", "SubstanceSource", "Substance Source Browser"),
		      NAME_None,
		      FEditorStyle::GetStyleSetName())
	{
	}

	TSharedPtr<FUICommandInfo> LoginButton;
	TSharedPtr<FUICommandInfo> LogoutButton;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};

void FSubstanceSourceCommands::RegisterCommands()
{

}

/** Registers the window tab within the editor window */
void FSubstanceSourceWindow::RegisterTabSpawners(const TSharedRef<class FTabManager>& TM)
{
	FSubstanceToolkit::RegisterTabSpawners(TM);
	const IWorkspaceMenuStructure& MenuStructure = WorkspaceMenu::GetMenuStructure();

	TM->RegisterTabSpawner(
	    SubstanceSourceTabId,
	    FOnSpawnTab::CreateSP(this, &FSubstanceSourceWindow::SpawnTab_SubstanceSourceWindow))
	.SetDisplayName(LOCTEXT("SubstanceSource", "SubstanceSource"))
	.SetGroup(MenuStructure.GetStructureRoot());
}

/** Unregisters the window tab within the editor window */
void FSubstanceSourceWindow::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TM)
{
	FSubstanceToolkit::UnregisterTabSpawners(TM);
	TM->UnregisterTabSpawner(SubstanceSourceTabId);
}

/** Window destructor */
FSubstanceSourceWindow::~FSubstanceSourceWindow()
{
	//Cleanup the source panel
	SubstanceSourcePanel.Reset();

	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	if (Editor != nullptr)
	{
		Editor->UnregisterForUndo(this);
	}
}

/** Call back to spawn a tab window */
TSharedRef<SDockTab> FSubstanceSourceWindow::SpawnTab_SubstanceSourceWindow(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == SubstanceSourceTabId);

	TSharedRef<SDockTab> SpawnedTab =
	    SNew(SDockTab)
	    .TabRole(ETabRole::NomadTab)
	    .Label(FText::FromString(GetTabPrefix() + TEXT("Substance Source")))
	    [
	        SubstanceSourcePanel.ToSharedRef()
	    ];

	AddToSpawnedToolPanels(Args.GetTabId().TabType, SpawnedTab);
	return SpawnedTab;
}

void FSubstanceSourceWindow::AddToSpawnedToolPanels(const FName& TabIdentifier, const TSharedRef<SDockTab>& SpawnedTab)
{
	TWeakPtr<SDockTab>* TabSpot = SpawnedToolPanels.Find(TabIdentifier);

	if (!TabSpot)
	{
		SpawnedToolPanels.Add(TabIdentifier, SpawnedTab);
	}
	else
	{
		check(!TabSpot->IsValid());
		*TabSpot = SpawnedTab;
	}
}

/** Entry point for initializing this window */
void FSubstanceSourceWindow::InitSubstanceSourceWindow(const TSharedPtr<class IToolkitHost>& InitToolkitHost)
{
	//Creates the substance source internal tab panel
	CreateInternalWidgets();

	//Create the default window layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_SubstanceSource_Layout")
	        ->AddArea
	        (
	            FTabManager::NewPrimaryArea()
	            ->SetOrientation(Orient_Vertical)
	            ->Split
	            (
	                FTabManager::NewSplitter()
	                ->SetOrientation(Orient_Vertical)
	                ->Split
	                (
	                    FTabManager::NewStack()
	                    ->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
	                    ->SetHideTabWell(true)
	                )
	                ->Split
	                (
	                    FTabManager::NewStack()
	                    ->AddTab(SubstanceSourceTabId, ETabState::OpenedTab)
	                    ->SetHideTabWell(true)
	                )
	            )
	        );

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	InitAssetEditor(EToolkitMode::Standalone,
	                InitToolkitHost,
	                SubstanceSourceModule::SubstanceSourceAppIdentifier,
	                StandaloneDefaultLayout,
	                true,
	                true);

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	//TODO:: if we want to restore the window, we will have to set up saving and loading from config
	//TSharedRef<FTabManager::FLayout> LayoutToUse = FLayoutSaveRestore::LoadFromConfig(GEditorLayoutIni, StandaloneDefaultLayout);

	//TODO:: Menus need to be adjusted to the substance source window.
	//And example is the edit drop down menu which is currently empty as we do not have an asset or assets to edit within this window.
}

FName FSubstanceSourceWindow::GetToolkitFName() const
{
	return FName("Substance Source Toolkit");
}

FText FSubstanceSourceWindow::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Substance Source");
}

FString FSubstanceSourceWindow::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Font ").ToString();
}

FLinearColor FSubstanceSourceWindow::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

void FSubstanceSourceWindow::AddReferencedObjects(FReferenceCollector& Collector)
{
}

void FSubstanceSourceWindow::CreateInternalWidgets()
{
	SubstanceSourcePanel = SNew(SSubstanceSourcePanel);
}

FReply FSubstanceSourceWindow::OnLoginPressed()
{
	SubstanceSourcePanel->SetLayoutLoginView();
	FSubstanceToolkit::OnLoginPressed();
	return FReply::Handled();
}

FReply FSubstanceSourceWindow::OnLogoutPressed()
{
	SubstanceSourcePanel->SetLayoutTilesView();
	FSubstanceToolkit::OnLogoutPressed();

	return FReply::Handled();
}

FReply FSubstanceSourceWindow::OnSubstancePressed()
{
	if (ISubstanceSourceModule::Get()->GetDatabaseLoadingStatus() == ESourceDatabaseLoadingStatus::Loaded)
	{
		SubstanceSourcePanel->SetLayoutTilesView();
		ISubstanceSourceModule::Get()->DisplayAssetCategory(Alg::Source::Database::sCategoryNewAssets);
	}

	return FSubstanceToolkit::OnSubstancePressed();
}

void FSubstanceSourceWindow::ExtendToolbar()
{

}

void FSubstanceSourceWindow::BindCommands()
{
	const FSubstanceSourceCommands& Commands = FSubstanceSourceCommands::Get();
}

#undef LOCTEXT_NAMESPACE
