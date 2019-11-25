/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/MultiPackerAssetGraphSchema.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerTextureEdNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerMaterialNode.h"
#include "ScopedTransaction.h"
#include "Graph/MultiPacker.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "EdGraphNode_Comment.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MultiPackerEditorTypes.h"

#define LOCTEXT_NAMESPACE "MultiPackerAssetSchema"

int32 UMultiPackerAssetGraphSchema::CurrentCacheRefreshID = 0;

class FNodeVisitorCycleChecker
{
public:
	/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
	bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
	{
		VisitedNodes.Add(StartNode);

		return TraverseInputNodesToRoot(EndNode);
	}

private:
	bool TraverseInputNodesToRoot(UEdGraphNode* Node)
	{
		VisitedNodes.Add(Node);

		for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
		{
			UEdGraphPin* MyPin = Node->Pins[PinIndex];

			if (MyPin->Direction == EGPD_Output)
			{
				for (int32 LinkedPinIndex = 0; LinkedPinIndex < MyPin->LinkedTo.Num(); ++LinkedPinIndex)
				{
					UEdGraphPin* OtherPin = MyPin->LinkedTo[LinkedPinIndex];
					if (OtherPin)
					{
						UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
						if (VisitedNodes.Contains(OtherNode))
						{
							return false;
						}
						else
						{
							return TraverseInputNodesToRoot(OtherNode);
						}
					}
				}
			}
		}

		return true;
	}

	TSet<UEdGraphNode*> VisitedNodes;
};

UEdGraphNode* FMultiPackerAssetSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UMultiPacker* Graph = CastChecked<UMultiPacker>(ParentGraph->GetOuter());

	const FScopedTransaction Transaction(LOCTEXT("MultiPackerEditorNewNode", "MultiPacker Editor: New Texture Node"));
	ParentGraph->Modify();
	Graph->Modify();

	UMultiPackerTextureNode* NewNode = NewObject<UMultiPackerTextureNode>(Graph, Graph->NodeType);

	Graph->TexNodes.Add(NewNode);

	FGraphNodeCreator<UMultiPackerTextureEdNode> NodeCreator(*Graph->EdGraph);
	UMultiPackerTextureEdNode* GraphNode = NodeCreator.CreateNode(true);
	GraphNode->SetGenericGraphNode(NewNode);
	NodeCreator.Finalize();

	GraphNode->NodePosX = Location.X;
	GraphNode->NodePosY = Location.Y;

	GraphNode->AutowireNewNode(FromPin);

	Graph->PostEditChange();
	Graph->MarkPackageDirty();

	return GraphNode;
}

UEdGraphNode* FMultiPackerAssetSchemaAction_NewNodeMat::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UMultiPacker* Graph = CastChecked<UMultiPacker>(ParentGraph->GetOuter());

	const FScopedTransaction Transaction(LOCTEXT("MultiPackerMaterialEditorNewNode", "MultiPacker Editor: New Material Node"));
	ParentGraph->Modify();
	Graph->Modify();

	UMultiPackerMatNode* NewNode = NewObject<UMultiPackerMatNode>(Graph, Graph->MatNodeType);
	Graph->MatNodes.Add(NewNode);

	FGraphNodeCreator<UMultiPackerMaterialNode> NodeCreator(*Graph->EdGraph);
	UMultiPackerMaterialNode* GraphNode = NodeCreator.CreateNode(true);
	GraphNode->SetGenericGraphNode(NewNode);
	NodeCreator.Finalize();

	GraphNode->NodePosX = Location.X;
	GraphNode->NodePosY = Location.Y;

	GraphNode->AutowireNewNode(FromPin);

	Graph->PostEditChange();
	Graph->MarkPackageDirty();

	return GraphNode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FNewComment_DialogueGraphSchemaAction
UEdGraphNode* FMultiPackerAssetSchemaAction_NewNodeComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode )
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();
	// Wrap comment around other nodes, this makes it possible to select other nodes and press the "C" key on the keyboard.
	FVector2D SpawnLocation = Location;
	FSlateRect Bounds;
	if (Graph.IsValid())
	{
		if (Graph->GetBoundsForSelectedNodes(Bounds, 50.0f))
		{
			CommentTemplate->SetBounds(Bounds);
			SpawnLocation.X = CommentTemplate->NodePosX;
			SpawnLocation.Y = CommentTemplate->NodePosY;
		}
	}
	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation);
}

void UMultiPackerAssetGraphSchema::GetBreakLinkToSubMenuActions(class FMenuBuilder& MenuBuilder, class UEdGraphPin* InGraphPin)
{
	// Make sure we have a unique name for every entry in the list
	TMap< FString, uint32 > LinkTitleCount;

	// Add all the links we could break from
	for (TArray<class UEdGraphPin*>::TConstIterator Links(InGraphPin->LinkedTo); Links; ++Links)
	{
		UEdGraphPin* Pin = *Links;
		FString TitleString = Pin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString();
		FText Title = FText::FromString(TitleString);
		if (Pin->PinName != TEXT(""))
		{
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
			TitleString = FString::Printf(TEXT("%s (%s)"), *TitleString, *Pin->PinName);
#endif
			// Add name of connection if possible
			FFormatNamedArguments Args;
			Args.Add(TEXT("NodeTitle"), Title);
			Args.Add(TEXT("PinName"), Pin->GetDisplayName());
			Title = FText::Format(LOCTEXT("BreakDescPin", "{NodeTitle} ({PinName})"), Args);
		}

		uint32 &Count = LinkTitleCount.FindOrAdd(TitleString);

		FText Description;
		FFormatNamedArguments Args;
		Args.Add(TEXT("NodeTitle"), Title);
		Args.Add(TEXT("NumberOfNodes"), Count);

		if (Count == 0)
		{
			Description = FText::Format(LOCTEXT("BreakDesc", "Break link to {NodeTitle}"), Args);
		}
		else
		{
			Description = FText::Format(LOCTEXT("BreakDescMulti", "Break link to {NodeTitle} ({NumberOfNodes})"), Args);
		}
		++Count;

		MenuBuilder.AddMenuEntry(Description, Description, FSlateIcon(), FUIAction(
			FExecuteAction::CreateUObject(this, &UMultiPackerAssetGraphSchema::BreakSinglePinLink, const_cast<UEdGraphPin*>(InGraphPin), *Links)));
	}
}

void UMultiPackerAssetGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);

	const FText AddToolTip = LOCTEXT("NewGenericGraphNodeTooltip", "Add node here");
	const FText Desc = LOCTEXT("NewGenericGraphNodeTooltip", "Add Texture Node");
	TSharedPtr<FMultiPackerAssetSchemaAction_NewNode> NewNodeAction(new FMultiPackerAssetSchemaAction_NewNode(LOCTEXT("GenericGraphNodeAction", "MultiPacker Node"), Desc, AddToolTip, 0));

	const FText AddToolTipMat = LOCTEXT("NewMaterialNodeTooltip", "Add node here");
	const FText DescMat = LOCTEXT("NewMaterialNodeTooltip", "Add Material Node");
	TSharedPtr<FMultiPackerAssetSchemaAction_NewNodeMat> NewNodeMatAction(new FMultiPackerAssetSchemaAction_NewNodeMat(LOCTEXT("MaterialNodeAction", "MultiPacker Node"), DescMat, AddToolTipMat, 0));

	const FText AddToolTipComment = LOCTEXT("NewCommentTooltip", "Add Comment here");
	const FText DescComment = LOCTEXT("NewCommentTooltip", "Add Comment Node");
	TSharedPtr<FMultiPackerAssetSchemaAction_NewNodeComment> NewNodeCommentAction(new FMultiPackerAssetSchemaAction_NewNodeComment(LOCTEXT("CommentAction", "MultiPacker Comment"), DescComment, AddToolTipComment, 0));
	
	ContextMenuBuilder.AddAction(NewNodeAction);
	ContextMenuBuilder.AddAction(NewNodeMatAction);
	ContextMenuBuilder.AddAction(NewNodeCommentAction);
}

void UMultiPackerAssetGraphSchema::GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, class FMenuBuilder* MenuBuilder, bool bIsDebugging) const
{
	if (InGraphPin != nullptr)
	{
		MenuBuilder->BeginSection("GenericGraphAssetGraphSchemaNodeActions", LOCTEXT("PinActionsMenuHeader", "Pin Actions"));
		{
			// Only display the 'Break Link' option if there is a link to break!
			if (InGraphPin->LinkedTo.Num() > 0)
			{
				MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().BreakPinLinks);

				// add sub menu for break link to
				if (InGraphPin->LinkedTo.Num() > 1)
				{
					MenuBuilder->AddSubMenu(
						LOCTEXT("BreakLinkTo", "Break Link To..."),
						LOCTEXT("BreakSpecificLinks", "Break a specific link..."),
						FNewMenuDelegate::CreateUObject((UMultiPackerAssetGraphSchema*const)this, &UMultiPackerAssetGraphSchema::GetBreakLinkToSubMenuActions, const_cast<UEdGraphPin*>(InGraphPin)));
				}
				else
				{
					((UMultiPackerAssetGraphSchema*const)this)->GetBreakLinkToSubMenuActions(*MenuBuilder, const_cast<UEdGraphPin*>(InGraphPin));
				}
			}
		}
		MenuBuilder->EndSection();
	}
	else if(InGraphNode != nullptr)
	{
		MenuBuilder->BeginSection("GenericGraphAssetGraphSchemaNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
		{
			MenuBuilder->AddMenuEntry(FGenericCommands::Get().Delete);
			MenuBuilder->AddMenuEntry(FGenericCommands::Get().Cut);
			MenuBuilder->AddMenuEntry(FGenericCommands::Get().Copy);
			MenuBuilder->AddMenuEntry(FGenericCommands::Get().Duplicate);

			MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
		}
		MenuBuilder->EndSection();
	}

	Super::GetContextMenuActions(CurrentGraph, InGraphNode, InGraphPin, MenuBuilder, bIsDebugging);
}

const FPinConnectionResponse UMultiPackerAssetGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	// Make sure the pins are not on the same node
	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Both are on the same node"));
	}

	// Compare the directions
	if ((A->Direction == EGPD_Input) && (B->Direction == EGPD_Input))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorInput", "Can't connect input node to input node"));
	}
	else if ((A->Direction == EGPD_Output) && (B->Direction == EGPD_Output))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOutput", "Can't connect output node to output node"));
	}

	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if (!CycleChecker.CheckForLoop(A->GetOwningNode(), B->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
}

FLinearColor UMultiPackerAssetGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	if (PinType.PinCategory == UMultiPackerEditorTypes::PinCategory_RGB)
	{
		return FLinearColor::White;
	}
	else if (PinType.PinCategory == UMultiPackerEditorTypes::PinCategory_Red)
	{
		return FLinearColor::Red;
	}
	else if (PinType.PinCategory == UMultiPackerEditorTypes::PinCategory_Green)
	{
		return FLinearColor::Green;
	}
	else if (PinType.PinCategory == UMultiPackerEditorTypes::PinCategory_Blue)
	{
		return FLinearColor::Blue;
	}
	else if (PinType.PinCategory == UMultiPackerEditorTypes::PinCategory_Alpha)
	{
		return FLinearColor::Gray;
	}
	else if (PinType.PinCategory == UMultiPackerEditorTypes::PinCategory_RGBA)
	{
		return FLinearColor::White;
	}
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	if (PinType.PinCategory == FName(*UMultiPackerEditorTypes::PinCategory_RGB))
	{
		return FLinearColor::White;
	}
	else if (PinType.PinCategory == FName(*UMultiPackerEditorTypes::PinCategory_Red))
	{
		return FLinearColor::Red;
	}
	else if (PinType.PinCategory == FName(*UMultiPackerEditorTypes::PinCategory_Green))
	{
		return FLinearColor::Green;
	}
	else if (PinType.PinCategory == FName(*UMultiPackerEditorTypes::PinCategory_Blue))
	{
		return FLinearColor::Blue;
	}
	else if (PinType.PinCategory == FName(*UMultiPackerEditorTypes::PinCategory_Alpha))
	{
		return FLinearColor::Gray;
	}
	else if (PinType.PinCategory == FName(*UMultiPackerEditorTypes::PinCategory_RGBA))
	{
		return FLinearColor::White;
	}
#endif
	return FLinearColor::White;
}

void UMultiPackerAssetGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));

	Super::BreakNodeLinks(TargetNode);
}

void UMultiPackerAssetGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
}

#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
void UMultiPackerAssetGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin)
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 19)
void UMultiPackerAssetGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}
#endif

bool UMultiPackerAssetGraphSchema::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UMultiPackerAssetGraphSchema::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UMultiPackerAssetGraphSchema::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

TSharedPtr<FEdGraphSchemaAction> UMultiPackerAssetGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FMultiPackerAssetSchemaAction_NewNodeComment));
}

#undef LOCTEXT_NAMESPACE
