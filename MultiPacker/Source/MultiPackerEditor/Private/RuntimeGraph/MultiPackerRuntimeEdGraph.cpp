/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/MultiPackerRuntimeEdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeOutputNode.h"
#include "RuntimeGraph/Nodes/MultiPackerRuntimeMaterialNode.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "MultiPackerRuntimeGraph.h"

UMultiPackerRuntimeEdGraph::UMultiPackerRuntimeEdGraph()
{
}

UMultiPackerRuntimeEdGraph::~UMultiPackerRuntimeEdGraph()
{
	//destructor
}

void UMultiPackerRuntimeEdGraph::RebuildGenericGraph()
{
	if (!IsRebuilding)
	{
		IsRebuilding = true;
		UMultiPackerRuntimeGraph* G = CastChecked<UMultiPackerRuntimeGraph>(GetOuter());
		G->ClearGraph();
		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UMultiPackerRuntimeOutputNode* OutNode = Cast<UMultiPackerRuntimeOutputNode>(Nodes[i]);

			if (OutNode == nullptr)
			{
				UMultiPackerRuntimeMaterialNode* MaterialNode = Cast<UMultiPackerRuntimeMaterialNode>(Nodes[i]);
				if (MaterialNode == nullptr || MaterialNode->MultiPackerRuntimeMatNode == nullptr)
					continue;
				UMultiPackerRuntimeMatNode* MatNode = MaterialNode->MultiPackerRuntimeMatNode;
				G->MatNodes.Add(MatNode);

				for (int PinIdx = 0; PinIdx < MaterialNode->Pins.Num(); ++PinIdx)
				{
					UEdGraphPin* Pin = MaterialNode->Pins[PinIdx];

					if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
						continue;

					for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
					{
						UMultiPackerRuntimeOutputNode* ChildEdNode = Cast<UMultiPackerRuntimeOutputNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode());

						if (ChildEdNode == nullptr)
							continue;
					}
				}
			}
			else
			{
				continue;
			}
		}
		IsRebuilding = false;
	}
}

#if WITH_EDITOR
void UMultiPackerRuntimeEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	RebuildGenericGraph();

	Modify();
}
#endif

