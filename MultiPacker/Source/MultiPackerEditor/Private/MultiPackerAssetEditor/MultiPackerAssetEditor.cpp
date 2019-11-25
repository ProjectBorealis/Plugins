/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/MultiPackerAssetEditor.h"
#include "MultiPackerAssetEditor/MultiPackerAssetEditorToolbar.h"
#include "MultiPackerAssetEditor/MultiPackerAssetGraphSchema.h"
#include "MultiPackerAssetEditor/MultiPackerEditorCommands.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerTextureEdNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerMaterialNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"
#include "Graph/MultiPackerMatNode.h"
#include "Graph/MultiPackerTextureNode.h"
#include "MultiPackerAssetEditor/MultiPackerProcessCore.h"
#include "MultiPackerAssetEditor/MultiPackerEdGraph.h"
#include "Engine/Engine.h"
#include "UnrealEd.h"
#include "GraphEditor.h"
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
#include "MultiPackerAssetEditor/SMultiPackerAssetEditorDrop.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "MultiPackerAssetEditor"

const FName GenericGraphEditorAppName = FName(TEXT("MultiPackerNodeEditorApp"));

struct FMultiPackerAssetEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName ContentBrowserID;
};

//////////////////////////////////////////////////////////////////////////

const FName FMultiPackerAssetEditorTabs::DetailsID(TEXT("Details"));
const FName FMultiPackerAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FMultiPackerAssetEditorTabs::ContentBrowserID(TEXT("ContentBrowser"));

//////////////////////////////////////////////////////////////////////////

FMultiPackerAssetEditor::FMultiPackerAssetEditor()
{
	EditingGraph = nullptr;
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FMultiPackerAssetEditor::OnPackageSaved);
}

FMultiPackerAssetEditor::~FMultiPackerAssetEditor()
{
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
}

void FMultiPackerAssetEditor::InitGenericGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UMultiPacker* Graph)
{
	EditingGraph = Graph;
	CreateEdGraph();
	MPCore = NewObject<UMultiPackerProcessCore>(UMultiPackerProcessCore::StaticClass());
	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	FMultiPackerEditorCommands::Register();

	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FMultiPackerAssetEditorToolbar(SharedThis(this)));
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
						->AddTab(FMultiPackerAssetEditorTabs::ViewportID, ETabState::OpenedTab)
					)
					->Split				// ContentBrowser
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.33f)
						->AddTab(FMultiPackerAssetEditorTabs::ContentBrowserID, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.37f)
					->AddTab(FMultiPackerAssetEditorTabs::DetailsID, ETabState::OpenedTab)
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

void FMultiPackerAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MultiPacker", "MultiPacker Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FMultiPackerAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FMultiPackerAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FMultiPackerAssetEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FMultiPackerAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FMultiPackerAssetEditorTabs::ContentBrowserID, FOnSpawnTab::CreateSP(this, &FMultiPackerAssetEditor::SpawnTab_ContentBrowser))
		.SetDisplayName(LOCTEXT("ContentBrowserLabel", "ContentBrowser"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.Tabs.ContentBrowser"));
}

void FMultiPackerAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FMultiPackerAssetEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FMultiPackerAssetEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FMultiPackerAssetEditorTabs::ContentBrowserID);
}

FName FMultiPackerAssetEditor::GetToolkitFName() const
{
	return FName("FMultiPackerEditor");
}

FText FMultiPackerAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("MultiPackerAppLabel", "MultiPacker Editor");
}

FText FMultiPackerAssetEditor::GetToolkitName() const
{
	const bool bDirtyState = false; EditingGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("MultiPackerName"), FText::FromString(EditingGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("MultiPacker", "{MultiPackerName}{DirtyState}"), Args);
}

FText FMultiPackerAssetEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingGraph);
}

FLinearColor FMultiPackerAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FMultiPackerAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("MultiPackerEditor");
}

FString FMultiPackerAssetEditor::GetDocumentationLink() const
{
	return TEXT("");
}

void FMultiPackerAssetEditor::SaveAsset_Execute()
{
	UMultiPackerEdGraph* EdGraph = Cast<UMultiPackerEdGraph>(EditingGraph->EdGraph);
	if (EdGraph != nullptr)
	{
		while (EdGraph->IsRebuilding)
		{
		}
		FAssetEditorToolkit::SaveAsset_Execute();
	}
	RebuildGenericGraph();
}

void FMultiPackerAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EditingGraph);
	Collector.AddReferencedObject(EditingGraph->EdGraph);
}


TSharedRef<SDockTab> FMultiPackerAssetEditor::SpawnTab_ContentBrowser(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerAssetEditorTabs::ContentBrowserID);

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
	TSharedRef<SWidget, ESPMode::NotThreadSafe> ContentBrowser = ContentBrowserSingleton.CreateContentBrowser(FMultiPackerAssetEditorTabs::ContentBrowserID, SpawnedTab, &ContentBrowserConfig);
	SpawnedTab->SetContent(ContentBrowser);

	return SpawnedTab;
}

TSharedRef<SDockTab> FMultiPackerAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerAssetEditorTabs::ViewportID);

	AssetDropTarget = SNew(SMultiPackerAssetEditorDrop)
		.OnAssetDropped(this, &FMultiPackerAssetEditor::HandleAssetDropped)
		.OnIsAssetAcceptableForDrop(this, &FMultiPackerAssetEditor::IsAssetAcceptableForDrop)
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

TSharedRef<SDockTab> FMultiPackerAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMultiPackerAssetEditorTabs::DetailsID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Details"))
		[
			PropertyWidget.ToSharedRef()
		];
}


void FMultiPackerAssetEditor::HandleAssetDropped(UObject* AssetObject) 
{
	if (ViewportWidget.IsValid())
	{
		UMultiPackerOutputNode* EdNode = NULL;
		UMultiPackerEdGraph* EdGraph = Cast<UMultiPackerEdGraph>(EditingGraph->EdGraph);
		for (int i = 0; i < EdGraph->Nodes.Num(); ++i)
		{
			EdNode = Cast<UMultiPackerOutputNode>(EdGraph->Nodes[i]);
			if (EdNode != nullptr)
			{
				break;
			}
		}
		FVector2D Location = GetAssetDropGridLocation();
		EditingGraph->Modify();
		if (AssetObject->GetClass()->IsChildOf<UTexture>())
		{
			UMultiPackerTextureNode* NewNode = NewObject<UMultiPackerTextureNode>(EditingGraph, EditingGraph->NodeType);
			NewNode->CompareSizes(UMultiPackerBaseEnums::GetTextureSizeOutputEnum(EditingGraph->OutputSizeY), UMultiPackerBaseEnums::GetTextureSizeOutputEnum(EditingGraph->OutputSizeX));
			EditingGraph->TexNodes.Add(NewNode);

			FGraphNodeCreator<UMultiPackerTextureEdNode> NodeCreator(*EditingGraph->EdGraph);
			UMultiPackerTextureEdNode* GraphNode = NodeCreator.CreateNode(true);
			GraphNode->SetGenericGraphNode(NewNode);
			NodeCreator.Finalize();

			GraphNode->SetTextureInput(Cast<UTexture>(AssetObject));
			GraphNode->NodePosX = Location.X;
			GraphNode->NodePosY = Location.Y;
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
			EdNode->FindPin("Output", EGPD_Input)->MakeLinkTo(GraphNode->FindPin("RGB", EGPD_Output));
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 19)
			EdNode->FindPin(FName("Output"), EGPD_Input)->MakeLinkTo(GraphNode->FindPin(FName("RGB"), EGPD_Output));
#endif
			if (EditingGraph->RectangleTiles)
			{
				GraphNode->GenericGraphNode->RectangleSize = true;
				GraphNode->GenericGraphNode->SizeVertical = GraphNode->GenericGraphNode->TextureInput->GetSurfaceHeight();
				GraphNode->GenericGraphNode->SizeHorizontal = GraphNode->GenericGraphNode->TextureInput->GetSurfaceWidth();
			}
		}
		else
		{
			if (AssetObject->GetClass()->IsChildOf<UMaterial>())
			{
				UMultiPackerMatNode* NewNode = NewObject<UMultiPackerMatNode>(EditingGraph, EditingGraph->MatNodeType);
				NewNode->CompareSizes(UMultiPackerBaseEnums::GetTextureSizeOutputEnum(EditingGraph->OutputSizeY), UMultiPackerBaseEnums::GetTextureSizeOutputEnum(EditingGraph->OutputSizeX));
				EditingGraph->MatNodes.Add(NewNode);

				FGraphNodeCreator<UMultiPackerMaterialNode> NodeCreator(*EditingGraph->EdGraph);
				UMultiPackerMaterialNode* GraphNode = NodeCreator.CreateNode(true);
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
				if (EditingGraph->RectangleTiles)
				{
					GraphNode->MultiPackerMatNode->RectangleSize = true;
					GraphNode->MultiPackerMatNode->SizeVertical = 32;
					GraphNode->MultiPackerMatNode->SizeHorizontal = 32;
				}
			}
			else
			{
				if (AssetObject->GetClass()->IsChildOf<UMaterialInstance>())
				{
					UMultiPackerMatNode* NewNode = NewObject<UMultiPackerMatNode>(EditingGraph, EditingGraph->MatNodeType);

					EditingGraph->MatNodes.Add(NewNode);

					FGraphNodeCreator<UMultiPackerMaterialNode> NodeCreator(*EditingGraph->EdGraph);
					UMultiPackerMaterialNode* GraphNode = NodeCreator.CreateNode(true);
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
					if (EditingGraph->RectangleTiles)
					{
						GraphNode->MultiPackerMatNode->RectangleSize = true;
						GraphNode->MultiPackerMatNode->SizeVertical = 32;
						GraphNode->MultiPackerMatNode->SizeHorizontal = 32;
					}
				}
			}
		}
		EditingGraph->PostEditChange();
		EditingGraph->MarkPackageDirty();
	}
}

FVector2D FMultiPackerAssetEditor::GetAssetDropGridLocation() const
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

bool FMultiPackerAssetEditor::IsAssetAcceptableForDrop(const UObject* AssetObject) const 
{
	if (!AssetObject) return false;
	return AssetObject->GetClass()->IsChildOf<UTexture>() | AssetObject->GetClass()->IsChildOf<UMaterial>() | AssetObject->GetClass()->IsChildOf<UMaterialInstance>();
}

void FMultiPackerAssetEditor::CreateInternalWidgets()
{
	ViewportWidget = CreateViewportWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(EditingGraph);

	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FMultiPackerAssetEditor::OnFinishedChangingProperties);
}

TSharedRef<SGraphEditor> FMultiPackerAssetEditor::CreateViewportWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_GenericGraph", "MultiPacker Graph");

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FMultiPackerAssetEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FMultiPackerAssetEditor::OnNodeDoubleClicked);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FMultiPackerAssetEditor::OnNodeTitleCommitted);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditingGraph->EdGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}


void FMultiPackerAssetEditor::BindCommands()
{
	ToolkitCommands->MapAction(FMultiPackerEditorCommands::Get().GraphSettings,
		FExecuteAction::CreateSP(this, &FMultiPackerAssetEditor::GraphSettings),
		FCanExecuteAction::CreateSP(this, &FMultiPackerAssetEditor::CanGraphSettings)
	);
	ToolkitCommands->MapAction(FMultiPackerEditorCommands::Get().CompileGraphNodes,
		FExecuteAction::CreateSP(this, &FMultiPackerAssetEditor::ProcessGraph),
		FCanExecuteAction::CreateSP(this, &FMultiPackerAssetEditor::CanProcessGraph)
	);
}

void FMultiPackerAssetEditor::CreateEdGraph()
{
	if (EditingGraph->EdGraph == nullptr)
	{
		EditingGraph->EdGraph = CastChecked<UMultiPackerEdGraph>(FBlueprintEditorUtils::CreateNewGraph(EditingGraph, NAME_None, UMultiPackerEdGraph::StaticClass(), UMultiPackerAssetGraphSchema::StaticClass()));
		EditingGraph->EdGraph->bAllowDeletion = false;
		FGraphNodeCreator<UMultiPackerOutputNode> NodeCreator(*EditingGraph->EdGraph);
		UMultiPackerOutputNode* GraphMultiNodeOutput = NodeCreator.CreateNode(true);
		NodeCreator.Finalize();
		// Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = EditingGraph->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EditingGraph->EdGraph);
	}
}

void FMultiPackerAssetEditor::CreateCommandList()
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
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::GraphSettings),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanGraphSettings)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FMultiPackerAssetEditor::CanDuplicateNodes)
	);
	// Graph Editor Commands
	GraphEditorCommands->MapAction(FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateSP(this, &FMultiPackerAssetEditor::OnCreateComment)
	);
}

void FMultiPackerAssetEditor::OnCreateComment()
{
	FMultiPackerAssetSchemaAction_NewNodeComment CommentAction;
	CommentAction.Graph = ViewportWidget;
	CommentAction.PerformAction(EditingGraph->EdGraph, NULL, ViewportWidget->GetPasteLocation());
}

TSharedPtr<SGraphEditor> FMultiPackerAssetEditor::GetCurrGraphEditor()
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FMultiPackerAssetEditor::GetSelectedNodes()
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FMultiPackerAssetEditor::RebuildGenericGraph()
{
	if (EditingGraph == nullptr)
	{
		return;
	}

	UMultiPackerEdGraph* EdGraph = Cast<UMultiPackerEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	EdGraph->RebuildGenericGraph();
	if (CanProcessGraph())
	{
		ToolbarBuilder->GraphSettingsIconOutput = FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Status.Good");
	}
	else
	{
		ToolbarBuilder->GraphSettingsIconOutput = FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Status.Error");
	}
}

void FMultiPackerAssetEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}

bool FMultiPackerAssetEditor::CanSelectAllNodes()
{
	return true;
}

void FMultiPackerAssetEditor::DeleteSelectedNodes()
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

bool FMultiPackerAssetEditor::CanDeleteNodes()
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

void FMultiPackerAssetEditor::DeleteSelectedDuplicatableNodes()
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

void FMultiPackerAssetEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FMultiPackerAssetEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FMultiPackerAssetEditor::CopySelectedNodes()
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

bool FMultiPackerAssetEditor::CanCopyNodes()
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

void FMultiPackerAssetEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetPasteLocation());
	}
}

void FMultiPackerAssetEditor::PasteNodesHere(const FVector2D& Location)
{
}

bool FMultiPackerAssetEditor::CanPasteNodes()
{
	return false;
}

void FMultiPackerAssetEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FMultiPackerAssetEditor::CanDuplicateNodes()
{
	return false;
}

void FMultiPackerAssetEditor::GraphSettings()
{
	PropertyWidget->SetObject(EditingGraph);
}

bool FMultiPackerAssetEditor::CanGraphSettings()
{
	return CanProcessGraph();
}

void FMultiPackerAssetEditor::ProcessGraph()
{
	MPCore = NewObject<UMultiPackerProcessCore>(UMultiPackerProcessCore::StaticClass());
	MPCore->ProcessNodes(EditingGraph);
	EditingGraph->Modify();

	FGraphNodeCreator<UMultiPackerTextureEdNode> NodeCreator(*EditingGraph->EdGraph);
	UMultiPackerTextureEdNode* GraphNode = NodeCreator.CreateNode(true);
 	NodeCreator.Finalize();
	GraphNode->Modify();
	GraphNode->DestroyNode();
	RebuildGenericGraph();
}

bool FMultiPackerAssetEditor::CanProcessGraph()
{
	return true;
}

void FMultiPackerAssetEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	MPCore->SetGraph(EditingGraph);
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
			UMultiPackerOutputNode* EdNode = Cast<UMultiPackerOutputNode>(Node);
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
			UMultiPackerTextureEdNode* EdNodeTex = Cast<UMultiPackerTextureEdNode>(Node);
			if (EdNodeTex != nullptr && !Mixcle)
			{
				Mixcle = MatNode ? true : Mixcle;
				TexNode = true;
			}
			else
			if(!Mixcle)
			{
				UMultiPackerMaterialNode* EdNodeMat = Cast<UMultiPackerMaterialNode>(Node);
				if (EdNodeMat != nullptr)
				{
					MatNode = true;
					Mixcle = TexNode ? true : Mixcle;
				}
				else
				{
					Mixcle = true;
				}
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

void FMultiPackerAssetEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
}

void FMultiPackerAssetEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingGraph == nullptr)
		return;
	EditingGraph->EdGraph->GetSchema()->ForceVisualizationCacheClear();
}

void FMultiPackerAssetEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildGenericGraph(); 
}

void FMultiPackerAssetEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager) 
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FMultiPackerAssetEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

#undef LOCTEXT_NAMESPACE
