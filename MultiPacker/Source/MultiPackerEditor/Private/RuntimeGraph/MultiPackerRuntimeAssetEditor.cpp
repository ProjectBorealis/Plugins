/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/MultiPackerRuntimeAssetEditor.h"
#include "RuntimeGraph/MultiPackerRuntimeAssetEditorToolbar.h"
#include "RuntimeGraph/MultiPackerRuntimeAssetGraphSchema.h"
#include "MultiPackerAssetEditor/MultiPackerEditorCommands.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeMaterialNode.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeOutputNode.h"
#include "RuntimeGraph/MultiPackerRuntimeEdGraph.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Engine/Engine.h"
#include "UnrealEd.h"
#include "EdGraph/EdGraphPin.h"
#include "IDetailsView.h"
#include "ScopedTransaction.h"
#include "EdGraphUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyEditorModule.h"
#include "GraphEditorActions.h"
#include "EdGraph/EdGraphNode.h"
#include "Widgets/Docking/SDockTab.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "RHIDefinitions.h"
#include "Layout/SlateRect.h"
#include "Runtime/Launch/Resources/Version.h"
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 18)
#include "HAL/PlatformApplicationMisc.h"
#endif
#include <AssetThumbnail.h>
#include "MultiPackerRuntimeGraph.h"

#define LOCTEXT_NAMESPACE "MultiPackerRuntimeAssetEditor"

const FName GenericGraphEditorRuntimeAppName = FName(TEXT("MultiPackerRuntimeNodeEditorApp"));

struct FMultiPackerRuntimeAssetEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName ContentBrowserID;
};

//////////////////////////////////////////////////////////////////////////

const FName FMultiPackerRuntimeAssetEditorTabs::DetailsID(TEXT("Details"));
const FName FMultiPackerRuntimeAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FMultiPackerRuntimeAssetEditorTabs::ContentBrowserID(TEXT("ContentBrowser"));

//////////////////////////////////////////////////////////////////////////

FMultiPackerRuntimeAssetEditor::FMultiPackerRuntimeAssetEditor()
{
	EditingGraph = nullptr;
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FMultiPackerRuntimeAssetEditor::OnPackageSaved);
}

FMultiPackerRuntimeAssetEditor::~FMultiPackerRuntimeAssetEditor()
{
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
}

void FMultiPackerRuntimeAssetEditor::InitGenericGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UMultiPackerRuntimeGraph* Graph)
{
	EditingGraph = Graph;
	CreateEdGraph();
	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	FMultiPackerEditorCommands::Register();

	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FMultiPackerRuntimeAssetEditorToolbar(SharedThis(this)));
	}

	BindCommands();

	CreateInternalWidgets();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	
	ToolbarBuilder->AddGenericGraphToolbar(ToolbarExtender);

	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("MultiPackerNodeEditor_Graph")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.63f)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.66f)
					->Split				// Graph Widget
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.66f)
						->SetHideTabWell(true)
						->AddTab(FMultiPackerRuntimeAssetEditorTabs::ViewportID, ETabState::OpenedTab)
					)
					->Split				// ContentBrowser
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.33f)
						->AddTab(FMultiPackerRuntimeAssetEditorTabs::ContentBrowserID, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.37f)
					->AddTab(FMultiPackerRuntimeAssetEditorTabs::DetailsID, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, GenericGraphEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingGraph, false);

	RegenerateMenusAndToolbars();

	RebuildGenericGraph();
}

void FMultiPackerRuntimeAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MultiPacker", "MultiPacker Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FMultiPackerRuntimeAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FMultiPackerRuntimeAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FMultiPackerRuntimeAssetEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FMultiPackerRuntimeAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FMultiPackerRuntimeAssetEditorTabs::ContentBrowserID, FOnSpawnTab::CreateSP(this, &FMultiPackerRuntimeAssetEditor::SpawnTab_ContentBrowser))
		.SetDisplayName(LOCTEXT("ContentBrowserLabel", "ContentBrowser"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.ContentBrowser"));
}

void FMultiPackerRuntimeAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FMultiPackerRuntimeAssetEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FMultiPackerRuntimeAssetEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FMultiPackerRuntimeAssetEditorTabs::ContentBrowserID);
}

FName FMultiPackerRuntimeAssetEditor::GetToolkitFName() const
{
	return FName("FMultiPackerEditor");
}

FText FMultiPackerRuntimeAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("MultiPackerAppLabel", "MultiPacker Editor");
}

FText FMultiPackerRuntimeAssetEditor::GetToolkitName() const
{
	const bool bDirtyState = false; EditingGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("MultiPackerName"), FText::FromString(EditingGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("MultiPacker", "{MultiPackerName}{DirtyState}"), Args);
}

FText FMultiPackerRuntimeAssetEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingGraph);
}

FLinearColor FMultiPackerRuntimeAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FMultiPackerRuntimeAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("MultiPackerEditor");
}

FString FMultiPackerRuntimeAssetEditor::GetDocumentationLink() const
{
	return TEXT("");
}

void FMultiPackerRuntimeAssetEditor::SaveAsset_Execute()
{
	UMultiPackerRuntimeEdGraph* EdGraph = Cast<UMultiPackerRuntimeEdGraph>(EditingGraph->EdGraph);
	if (EdGraph != nullptr)
	{
		while (EdGraph->IsRebuilding)
		{
		}
		FAssetEditorToolkit::SaveAsset_Execute();
	}
	RebuildGenericGraph();
}

void FMultiPackerRuntimeAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EditingGraph);
	Collector.AddReferencedObject(EditingGraph->EdGraph);
}


TSharedRef<SDockTab> FMultiPackerRuntimeAssetEditor::SpawnTab_ContentBrowser(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerRuntimeAssetEditorTabs::ContentBrowserID);

	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("ContentBrowserKey", "ContentBrowser"))
		.TabColorScale(GetTabColorScale())
		[
			SNullWidget::NullWidget
		];

	IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();

	//FName ContentBrowserID = *("MultiPacker_ContentBrowser_" + FGuid::NewGuid().ToString());
	FContentBrowserConfig ContentBrowserConfig;
	TSharedRef<SWidget, ESPMode::NotThreadSafe> ContentBrowser = ContentBrowserSingleton.CreateContentBrowser(FMultiPackerRuntimeAssetEditorTabs::ContentBrowserID, SpawnedTab, &ContentBrowserConfig);
	SpawnedTab->SetContent(ContentBrowser);

	return SpawnedTab;
}

TSharedRef<SDockTab> FMultiPackerRuntimeAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerRuntimeAssetEditorTabs::ViewportID);

	AssetDropTarget = SNew(SMultiPackerAssetEditorDrop)
		.OnAssetDropped(this, &FMultiPackerRuntimeAssetEditor::HandleAssetDropped)
		.OnIsAssetAcceptableForDrop(this, &FMultiPackerRuntimeAssetEditor::IsAssetAcceptableForDrop)
		.Visibility(EVisibility::HitTestInvisible);

	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
				[
					ViewportWidget.ToSharedRef()
				]
			+ SOverlay::Slot()
				[
					AssetDropTarget.ToSharedRef()
				]
		];
}

TSharedRef<SDockTab> FMultiPackerRuntimeAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerRuntimeAssetEditorTabs::DetailsID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Details"))
		[
			PropertyWidget.ToSharedRef()
		];
}


void FMultiPackerRuntimeAssetEditor::HandleAssetDropped(UObject* AssetObject) 
{
	if (ViewportWidget.IsValid())
	{
		UMultiPackerRuntimeOutputNode* EdNode = NULL;
		UMultiPackerRuntimeEdGraph* EdGraph = Cast<UMultiPackerRuntimeEdGraph>(EditingGraph->EdGraph);
		for (int i = 0; i < EdGraph->Nodes.Num(); ++i)
		{
			EdNode = Cast<UMultiPackerRuntimeOutputNode>(EdGraph->Nodes[i]);
			if (EdNode != nullptr)
			{
				break;
			}
		}
		FVector2D Location = GetAssetDropGridLocation();
		EditingGraph->Modify();
		if (AssetObject->GetClass()->IsChildOf<UMaterial>())
		{
			UMultiPackerRuntimeMatNode* NewNode = NewObject<UMultiPackerRuntimeMatNode>(EditingGraph, EditingGraph->MatNodeType);

			EditingGraph->MatNodes.Add(NewNode);

			FGraphNodeCreator<UMultiPackerRuntimeMaterialNode> NodeCreator(*EditingGraph->EdGraph);
			UMultiPackerRuntimeMaterialNode* GraphNode = NodeCreator.CreateNode(true);
			GraphNode->SetGenericGraphNode(NewNode);
			NodeCreator.Finalize();

			GraphNode->SetMaterialInput(Cast<UMaterial>(AssetObject));
			GraphNode->NodePosX = Location.X;
			GraphNode->NodePosY = Location.Y;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
			EdNode->FindPin("Output", EGPD_Input)->MakeLinkTo(GraphNode->FindPin("Out", EGPD_Output));
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 19)
			EdNode->FindPin(FName("Output"), EGPD_Input)->MakeLinkTo(GraphNode->FindPin(FName("Out"), EGPD_Output));
#endif
		}
		else
		{
			if (AssetObject->GetClass()->IsChildOf<UMaterialInstance>())
			{
				UMultiPackerRuntimeMatNode* NewNode = NewObject<UMultiPackerRuntimeMatNode>(EditingGraph, EditingGraph->MatNodeType);

				EditingGraph->MatNodes.Add(NewNode);

				FGraphNodeCreator<UMultiPackerRuntimeMaterialNode> NodeCreator(*EditingGraph->EdGraph);
				UMultiPackerRuntimeMaterialNode* GraphNode = NodeCreator.CreateNode(true);
				GraphNode->SetGenericGraphNode(NewNode);
				NodeCreator.Finalize();

				GraphNode->SetMaterialInput(Cast<UMaterialInstance>(AssetObject));
				GraphNode->NodePosX = Location.X;
				GraphNode->NodePosY = Location.Y;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
				EdNode->FindPin("Output", EGPD_Input)->MakeLinkTo(GraphNode->FindPin("Out", EGPD_Output));
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 19)
				EdNode->FindPin(FName("Output"), EGPD_Input)->MakeLinkTo(GraphNode->FindPin(FName("Out"), EGPD_Output));
#endif
			}
		}
		EditingGraph->PostEditChange();
		EditingGraph->MarkPackageDirty();
	}
}

FVector2D FMultiPackerRuntimeAssetEditor::GetAssetDropGridLocation() const
{
	if (!AssetDropTarget.IsValid()) return FVector2D::ZeroVector;

	FVector2D PanelCoord = AssetDropTarget->GetPanelCoordDropPosition();
	FVector2D ViewLocation = FVector2D::ZeroVector;
	float ZoomAmount = 1.0f;
	ViewportWidget->GetViewLocation(ViewLocation, ZoomAmount);
	FVector2D GridLocation = PanelCoord / ZoomAmount + ViewLocation;

	GridLocation.Y = GridLocation.Y - (AssetDropTarget->GetRow() * 200);
	GridLocation.X = GridLocation.X - (AssetDropTarget->GetColumn() * 250);
	return GridLocation;
}

bool FMultiPackerRuntimeAssetEditor::IsAssetAcceptableForDrop(const UObject* AssetObject) const 
{
	if (!AssetObject) return false;
	return AssetObject->GetClass()->IsChildOf<UMaterial>() | AssetObject->GetClass()->IsChildOf<UMaterialInstance>();
}

void FMultiPackerRuntimeAssetEditor::CreateInternalWidgets()
{
	ViewportWidget = CreateViewportWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(EditingGraph);

	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FMultiPackerRuntimeAssetEditor::OnFinishedChangingProperties);
}

TSharedRef<SGraphEditor> FMultiPackerRuntimeAssetEditor::CreateViewportWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_GenericGraph", "MultiPackerRuntime Graph");

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FMultiPackerRuntimeAssetEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FMultiPackerRuntimeAssetEditor::OnNodeDoubleClicked);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FMultiPackerRuntimeAssetEditor::OnNodeTitleCommitted);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditingGraph->EdGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}


void FMultiPackerRuntimeAssetEditor::BindCommands()
{
	ToolkitCommands->MapAction(FMultiPackerEditorCommands::Get().GraphSettings,
		FExecuteAction::CreateSP(this, &FMultiPackerRuntimeAssetEditor::GraphSettings),
		FCanExecuteAction::CreateSP(this, &FMultiPackerRuntimeAssetEditor::CanGraphSettings)
	);
	ToolkitCommands->MapAction(FMultiPackerEditorCommands::Get().CompileGraphNodes,
		FExecuteAction::CreateSP(this, &FMultiPackerRuntimeAssetEditor::ProcessGraph),
		FCanExecuteAction::CreateSP(this, &FMultiPackerRuntimeAssetEditor::CanProcessGraph)
	);
}

void FMultiPackerRuntimeAssetEditor::CreateEdGraph()
{
	if (EditingGraph->EdGraph == nullptr)
	{
		EditingGraph->EdGraph = CastChecked<UMultiPackerRuntimeEdGraph>(FBlueprintEditorUtils::CreateNewGraph(EditingGraph, NAME_None, UMultiPackerRuntimeEdGraph::StaticClass(), UMultiPackerRuntimeAssetGraphSchema::StaticClass()));
		EditingGraph->EdGraph->bAllowDeletion = false;

		FGraphNodeCreator<UMultiPackerRuntimeOutputNode> NodeCreator(*EditingGraph->EdGraph);
		UMultiPackerRuntimeOutputNode* GraphMultiNodeOutput = NodeCreator.CreateNode(true);
		NodeCreator.Finalize();
		// Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = EditingGraph->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EditingGraph->EdGraph);
	}
}

void FMultiPackerRuntimeAssetEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);

	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FMultiPackerEditorCommands::Get().GraphSettings,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::GraphSettings),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanGraphSettings)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerRuntimeAssetEditor::CanDuplicateNodes)
	);
	// Graph Editor Commands
	GraphEditorCommands->MapAction(FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateSP(this, &FMultiPackerRuntimeAssetEditor::OnCreateComment)
	);
}

void FMultiPackerRuntimeAssetEditor::OnCreateComment()
{
	FMultiPackerRuntimeAssetSchemaAction_NewNodeComment CommentAction;
	CommentAction.Graph = ViewportWidget;
	CommentAction.PerformAction(EditingGraph->EdGraph, NULL, ViewportWidget->GetPasteLocation());
}

TSharedPtr<SGraphEditor> FMultiPackerRuntimeAssetEditor::GetCurrGraphEditor()
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FMultiPackerRuntimeAssetEditor::GetSelectedNodes()
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FMultiPackerRuntimeAssetEditor::RebuildGenericGraph()
{
	if (EditingGraph == nullptr)
	{
		return;
	}

	UMultiPackerRuntimeEdGraph* EdGraph = Cast<UMultiPackerRuntimeEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	EdGraph->RebuildGenericGraph();
	/*if (CanProcessGraph())
	{
		ToolbarBuilder->GraphSettingsIconOutput = FSlateIcon(FEditorStyle::GetStyleSetName(), "MaterialEditor.Apply"); //Kismet.Status.Good");
	}
	else
	{
		ToolbarBuilder->GraphSettingsIconOutput = FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Status.Error");
	}*/
}

void FMultiPackerRuntimeAssetEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}

bool FMultiPackerRuntimeAssetEditor::CanSelectAllNodes()
{
	return true;
}

void FMultiPackerRuntimeAssetEditor::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());
	CurrentGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
		{
			if (Node->CanUserDeleteNode())
			{
				Node->Modify();
				Node->DestroyNode();
			}
		}
	}
	RebuildGenericGraph();
}

bool FMultiPackerRuntimeAssetEditor::CanDeleteNodes()
{
	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}
	return false;
}

void FMultiPackerRuntimeAssetEditor::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FMultiPackerRuntimeAssetEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FMultiPackerRuntimeAssetEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FMultiPackerRuntimeAssetEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 17)
	FPlatformMisc::ClipboardCopy(*ExportedText);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 18)
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
#endif
}

bool FMultiPackerRuntimeAssetEditor::CanCopyNodes()
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FMultiPackerRuntimeAssetEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetPasteLocation());
	}
}

void FMultiPackerRuntimeAssetEditor::PasteNodesHere(const FVector2D& Location)
{
}

bool FMultiPackerRuntimeAssetEditor::CanPasteNodes()
{
	return false;
}

void FMultiPackerRuntimeAssetEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FMultiPackerRuntimeAssetEditor::CanDuplicateNodes()
{
	return false;
}

void FMultiPackerRuntimeAssetEditor::GraphSettings()
{
	PropertyWidget->SetObject(EditingGraph);
}

bool FMultiPackerRuntimeAssetEditor::CanGraphSettings() const
{
	return true;
}

void FMultiPackerRuntimeAssetEditor::ProcessGraph()
{
	EditingGraph->ProcessNodes(GEditor->GetEditorWorldContext().World());
}

bool FMultiPackerRuntimeAssetEditor::CanProcessGraph()
{
	return true;
}

void FMultiPackerRuntimeAssetEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : NewSelection)
	{
		Selection.Add(SelectionEntry);
	}

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (Selection.Num() == 0) 
	{
		PropertyWidget->SetObject(EditingGraph);
	}
	else if (Selection.Num() == 1)
	{
		for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
		{
			UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
			UMultiPackerRuntimeOutputNode* EdNode = Cast<UMultiPackerRuntimeOutputNode>(Node);
			if (EdNode != nullptr)
			{
				PropertyWidget->SetObject(EditingGraph);
			}
			else
			{
				PropertyWidget->SetObject(Selection[0]);
			}
		}
	}
	else
	{
		bool TexNode = false;
		bool MatNode = false;
		bool Mixcle = false;
		for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
		{
			UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
				UMultiPackerRuntimeMaterialNode* EdNode = Cast<UMultiPackerRuntimeMaterialNode>(Node);
				if (EdNode != nullptr)
				{
					MatNode = true;
					Mixcle = TexNode ? true : Mixcle;
				}
				else
				{
					Mixcle = true;
				}
			
		}
		if (!Mixcle)
		{
			PropertyWidget->SetObjects(Selection);
		}
		else
		{
			PropertyWidget->SetObject(nullptr);
		}
	}
	NodesChanged = true;
	RebuildGenericGraph();
}

void FMultiPackerRuntimeAssetEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
}

void FMultiPackerRuntimeAssetEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingGraph == nullptr)
		return;
	EditingGraph->EdGraph->GetSchema()->ForceVisualizationCacheClear();
}

void FMultiPackerRuntimeAssetEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildGenericGraph(); 
}

void FMultiPackerRuntimeAssetEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager) 
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FMultiPackerRuntimeAssetEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

#undef LOCTEXT_NAMESPACE