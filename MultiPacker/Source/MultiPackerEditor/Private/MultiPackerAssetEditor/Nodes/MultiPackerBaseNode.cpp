/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/MultiPackerBaseNode.h"
#include "MultiPackerAssetEditor/MultiPackerEdGraph.h"
#include "MultiPackerBaseEnums.h"
#include "TileUtils/TilePointer.h"

#define LOCTEXT_NAMESPACE "MultiPackerBaseNode"

UMultiPackerEdGraph* UMultiPackerBaseNode::GetGenericGraphEdGraph()
{
	return Cast<UMultiPackerEdGraph>(GetGraph());
}

FText UMultiPackerBaseNode::GetDescription() const
{
	return FText::FromString("Texture");
}

bool UMultiPackerBaseNode::GetMsdf()
{
	return false;
}

bool UMultiPackerBaseNode::IsNodeSelectedSdf()
{
	return false;
}

FTileThumbDatabase UMultiPackerBaseNode::GetTileThumbDatabase(uint16 num)
{
	FTileThumbDatabase NewTileData;
	return NewTileData;
}

TArray<UTilePointer*> UMultiPackerBaseNode::GetTiles(const uint16 size, FVector2D InTilePadding)
{
	if (SizeTile != size || TilesNeedProcess())
	{
		SizeTile = size;
		ProcessTiles();
	}
	for (UTilePointer* Tile : Tiles)
	{
		Tile->TilePadding = InTilePadding;
	}
	return Tiles;
}

void UMultiPackerBaseNode::SetDatabase()
{
	for (uint16 n = 0; n < Tiles.Num(); ++n)
	{
		FTileDatabase NewTileData;
		NewTileData.SetFromThumbDatabase(GetTileThumbDatabase(n), IsTilesSDF(), GetMsdf());
		Tiles[n]->TileDatabase = NewTileData;
	}
}

bool UMultiPackerBaseNode::TilesNeedProcess()
{
	return !TilesProcessed || TilesChanged;
}

void UMultiPackerBaseNode::ProcessTiles()
{
}

bool UMultiPackerBaseNode::IsTilesSDF()
{
	return IsNodeSelectedSdf() || GraphSDF;
}

bool UMultiPackerBaseNode::PinConnected()
{
	return false;
}

bool UMultiPackerBaseNode::IsObjectValid()
{
	return false;
}

uint16 UMultiPackerBaseNode::GetNumberTiles()
{
	return 0;
}

#undef LOCTEXT_NAMESPACE