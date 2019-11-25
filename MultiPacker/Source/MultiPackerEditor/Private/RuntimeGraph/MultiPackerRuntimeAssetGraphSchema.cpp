/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/MultiPackerRuntimeAssetGraphSchema.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeMaterialNode.h"
#include "ScopedTransaction.h"
#include "MultiPackerRuntimeGraph.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "EdGraphNode_Comment.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "MultiPackerAssetSchema"

int32 UMultiPackerRuntimeAssetGraphSchema::CurrentCacheRefreshID = 0;

class FNodeRuntimeVisitorCycleChecker
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

UEdGraphNode* FMultiPackerRuntimeAssetSchemaAction_NewNodeMat::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UMultiPackerRuntimeGraph* Graph = CastChecked<UMultiPackerRuntimeGraph>(ParentGraph->GetOuter());

	const FScopedTransaction Transaction(LOCTEXT("MultiPackerRuntimeMaterialEditorNewNode", "MultiPacker Editor: New Material Node"));
	ParentGraph->Modify();
	Graph->Modify();

	UMultiPackerRuntimeMatNode* NewNode = NewObject<UMultiPackerRuntimeMatNode>(Graph, Graph->MatNodeType);
	Graph->MatNodes.Add(NewNode);

	FGraphNodeCreator<UMultiPackerRuntimeMaterialNode> NodeCreator(*Graph->EdGraph);
	UMultiPackerRuntimeMaterialNode* GraphNode = NodeCreator.CreateNode(true);
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
UEdGraphNode* FMultiPackerRuntimeAssetSchemaAction_NewNodeComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode )
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

void UMultiPackerRuntimeAssetGraphSchema::GetBreakLinkToSubMenuActions(class FMenuBuilder& MenuBuilder, class UEdGraphPin* InGraphPin)
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
			FExecuteAction::CreateUObject(this, &UMultiPackerRuntimeAssetGraphSchema::BreakSinglePinLink, const_cast<UEdGraphPin*>(InGraphPin), *Links)));
	}
}

void UMultiPackerRuntimeAssetGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);
	
	const FText AddToolTipMat = LOCTEXT("NewMaterialNodeTooltip", "Add node here");
	const FText DescMat = LOCTEXT("NewMaterialNodeTooltip", "Add Material Node");
	TSharedPtr<FMultiPackerRuntimeAssetSchemaAction_NewNodeMat> NewNodeMatAction(new FMultiPackerRuntimeAssetSchemaAction_NewNodeMat(LOCTEXT("MaterialNodeAction", "MultiPacker Node"), DescMat, AddToolTipMat, 0));

	const FText AddToolTipComment = LOCTEXT("NewCommentTooltip", "Add Comment here");
	const FText DescComment = LOCTEXT("NewCommentTooltip", "Add Comment Node");
	TSharedPtr<FMultiPackerRuntimeAssetSchemaAction_NewNodeComment> NewNodeCommentAction(new FMultiPackerRuntimeAssetSchemaAction_NewNodeComment(LOCTEXT("CommentAction", "MultiPacker Comment"), DescComment, AddToolTipComment, 0));
	
	ContextMenuBuilder.AddAction(NewNodeMatAction);
	ContextMenuBuilder.AddAction(NewNodeCommentAction);
}

void UMultiPackerRuntimeAssetGraphSchema::GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, class FMenuBuilder* MenuBuilder, bool bIsDebugging) const
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
						FNewMenuDelegate::CreateUObject((UMultiPackerRuntimeAssetGraphSchema*const)this, &UMultiPackerRuntimeAssetGraphSchema::GetBreakLinkToSubMenuActions, const_cast<UEdGraphPin*>(InGraphPin)));
				}
				else
				{
					((UMultiPackerRuntimeAssetGraphSchema*const)this)->GetBreakLinkToSubMenuActions(*MenuBuilder, const_cast<UEdGraphPin*>(InGraphPin));
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

const FPinConnectionResponse UMultiPackerRuntimeAssetGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
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
	FNodeRuntimeVisitorCycleChecker CycleChecker;
	if (!CycleChecker.CheckForLoop(A->GetOwningNode(), B->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
}

FLinearColor UMultiPackerRuntimeAssetGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::White;
}

void UMultiPackerRuntimeAssetGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));

	Super::BreakNodeLinks(TargetNode);
}

void UMultiPackerRuntimeAssetGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
}

#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
void UMultiPackerRuntimeAssetGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin)
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 19)
void UMultiPackerRuntimeAssetGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}
#endif

bool UMultiPackerRuntimeAssetGraphSchema::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UMultiPackerRuntimeAssetGraphSchema::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UMultiPackerRuntimeAssetGraphSchema::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

TSharedPtr<FEdGraphSchemaAction> UMultiPackerRuntimeAssetGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FMultiPackerRuntimeAssetSchemaAction_NewNodeComment));
}

#undef LOCTEXT_NAMESPACE
