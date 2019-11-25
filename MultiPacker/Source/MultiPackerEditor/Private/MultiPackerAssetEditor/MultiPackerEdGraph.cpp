/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/MultiPackerEdGraph.h"
#include "Graph/MultiPacker.h"
#include "EdGraph/EdGraphPin.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerTextureEdNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerMaterialNode.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "Graph/MultiPackerTextureNode.h"

UMultiPackerEdGraph::UMultiPackerEdGraph()
{
}

UMultiPackerEdGraph::~UMultiPackerEdGraph()
{
}

void UMultiPackerEdGraph::RebuildGenericGraph()
{
	if (!IsRebuilding)
	{
		IsRebuilding = true;
		UMultiPacker* G = CastChecked<UMultiPacker>(GetOuter());
		G->ClearGraph();
		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UMultiPackerTextureEdNode* TexNode = Cast<UMultiPackerTextureEdNode>(Nodes[i]);

			if (TexNode == nullptr || TexNode->GenericGraphNode == nullptr)
			{
				UMultiPackerOutputNode* OutputNode = Cast<UMultiPackerOutputNode>(Nodes[i]);

				if (OutputNode == nullptr )
				{
					UMultiPackerMaterialNode* MatNode = Cast<UMultiPackerMaterialNode>(Nodes[i]);
					if (MatNode == nullptr || MatNode->MultiPackerMatNode == nullptr)
						continue;
					UMultiPackerMatNode* MNode = MatNode->MultiPackerMatNode;
					G->MatNodes.Add(MNode);
				}
				else
				{
					OutputNode->ProcessArrayThumbnail();
					continue;
				}
			}
			else
			{
				UMultiPackerTextureNode* TNode = TexNode->GenericGraphNode;
 
				if (TNode->CanSetThumbnail())
				{
					TexNode->AssetThumbnail = MakeShareable(new FAssetThumbnail(TNode->TextureInput, 128, 128, FMultiPackerEditorThumbnail::Get()));
				}
				G->TexNodes.Add(TNode);
			}
		}
		IsRebuilding = false;
	}
}

#if WITH_EDITOR
void UMultiPackerEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	RebuildGenericGraph();

	Modify();
}
#endif

