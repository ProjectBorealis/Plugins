/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "LayerDatabase/LayerStyleEditorToolkit.h"
#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
#include "LayerDatabase/SLayerStyleEditor.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"
#include <PropertyEditorModule.h>
#include <Modules/ModuleManager.h>
#include <Misc/AssertionMacros.h>
#include <Materials/Material.h>
#include "Materials/MaterialInterface.h"
#include <Materials/MaterialInstanceDynamic.h>
#include "MultiPackerImageCore.h"
#include "MultiPackerLayerDatabase.h"

#define LOCTEXT_NAMESPACE "FLayerStyleEditorToolkit"

/* Local constants
*****************************************************************************/

const FName FLayerStyleEditorToolkit::AppId(TEXT("StyleAssetEditorApp"));
const FName FLayerStyleEditorToolkit::PreviewTabId(TEXT("MPStyle_Preview"));
const FName FLayerStyleEditorToolkit::DetailsTabId(TEXT("MPStyle_Details"));

/* FLayerStyleEditorToolkit structors
*****************************************************************************/

FLayerStyleEditorToolkit::FLayerStyleEditorToolkit()
{ }

FLayerStyleEditorToolkit::~FLayerStyleEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}

void FLayerStyleEditorToolkit::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged)
{
	StyleAsset->LayerBase.ColorInterior = StyleAsset->ColorInterior;
	StyleAsset->LayerBase.ColorOutline = StyleAsset->ColorOutline;
	StyleAsset->LayerBase.OutlineThresold = StyleAsset->OutlineThresold;
	StyleAsset->LayerBase.SDFThresold = StyleAsset->SDFThresold;
	StyleAsset->Material = UMultiPackerImageCore::SetDataBaseLayer(StyleAsset->Material, StyleAsset->LayerBase, "");
}

/* FLayerStyleEditorToolkit interface
*****************************************************************************/

void FLayerStyleEditorToolkit::Initialize(UMultiPackerLayerDatabase* InStyleAsset, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost)
{
	StyleAsset = InStyleAsset;

	CreateInternalWidgets();
	// Support undo/redo
	StyleAsset->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_StyleAssetEditor")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			->SetHideTabWell(true)
			->SetSizeCoefficient(0.1f)
		)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(DetailsTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.37f)
			)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(PreviewTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.9f)
			)
		)
	);

	FAssetEditorToolkit::InitAssetEditor(
		InMode,
		InToolkitHost,
		AppId,
		Layout,
		true /*bCreateDefaultStandaloneMenu*/,
		true /*bCreateDefaultToolbar*/,
		InStyleAsset
	);

	RegenerateMenusAndToolbars();
}

void FLayerStyleEditorToolkit::CreateInternalWidgets()
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	StyleAsset->Material = UMultiPackerImageCore::CreateMaterial(EMultiPackerImageLayer::EMCE_Option1, NULL, StyleAsset->LayerBase, StyleAsset->LayerBase, StyleAsset->LayerBase);
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(StyleAsset);

	PreviewUIViewport = SNew(SLayerStyleEditor, StyleAsset->Material);
}

/* FAssetEditorToolkit interface
*****************************************************************************/

FString FLayerStyleEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("https://github.com/ue4plugins/StyleAsset"));
}


void FLayerStyleEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_StyleAssetEditor", "Text Asset Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	
	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FLayerStyleEditorToolkit::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PreviewTabId, FOnSpawnTab::CreateSP(this, &FLayerStyleEditorToolkit::SpawnTab_Material))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}


void FLayerStyleEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(AppId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(PreviewTabId);
}

/* IToolkit interface
*****************************************************************************/

FText FLayerStyleEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Text Asset Editor");
}


FName FLayerStyleEditorToolkit::GetToolkitFName() const
{
	return FName("StyleAssetEditor");
}


FLinearColor FLayerStyleEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FLayerStyleEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "StyleAsset ").ToString();
}


/* FGCObject interface
*****************************************************************************/

void FLayerStyleEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(StyleAsset);
}


/* FEditorUndoClient interface
*****************************************************************************/

void FLayerStyleEditorToolkit::PostUndo(bool bSuccess)
{ }


void FLayerStyleEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}


/* FLayerStyleEditorToolkit callbacks
*****************************************************************************/

TSharedRef<SDockTab> FLayerStyleEditorToolkit::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	//check(Args.GetTabId() == TEXT("Details"));

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Details"))
		[
			PropertyWidget.ToSharedRef()
		];
}


TSharedRef<SDockTab> FLayerStyleEditorToolkit::SpawnTab_Material(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("ViewportTabTitle", "Viewport"))
		[
			PreviewUIViewport.ToSharedRef()
		];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE
