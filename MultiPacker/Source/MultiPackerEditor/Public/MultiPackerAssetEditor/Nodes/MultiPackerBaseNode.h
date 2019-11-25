/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerBaseNode.generated.h"

class FAssetThumbnail;
class UMultiPackerEdGraph;
class UTilePointer;

UCLASS(MinimalAPI)
class UMultiPackerBaseNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	TSharedPtr<class FAssetThumbnail> AssetThumbnail;

	UPROPERTY(Transient)
		TArray<UTilePointer*> Tiles;
	UPROPERTY(Transient)
		bool TilesProcessed = false;
	UPROPERTY(Transient)
		bool TilesChanged = false;


	//Enable if you want to Override the global Padding settings and apply this
	UPROPERTY(EditDefaultsOnly, Category = "Tiled Options")
		bool OverridePadding = false;

	//Padding for individual Tile, This allows to get out the blur border of the nearest tile
	//The Padding X work for right and left, and Y for up and down, Advice: control the values will be doubled.
	//EveryTime will be applied if you like to not have Padding set to 0
	UPROPERTY(EditDefaultsOnly, Category = "Tiled Options", meta = (EditCondition = OverridePadding))
		FVector2D TilePadding;

	UMultiPackerEdGraph* GetGenericGraphEdGraph();
	FText GetDescription() const;
	virtual bool GetMsdf();
	virtual bool IsNodeSelectedSdf();
	virtual FTileThumbDatabase GetTileThumbDatabase(uint16 num);
#if WITH_EDITOR  
	//TilePointer Side class
	TArray<UTilePointer*> GetTiles(const uint16 size, FVector2D InTilePadding);
	void SetDatabase();
	bool TilesNeedProcess();
	virtual void ProcessTiles();
	bool IsTilesSDF();

	virtual bool PinConnected();
	virtual bool IsObjectValid();
#endif
	bool GraphSDF = false;
	uint16 SizeTile = 0;

	virtual uint16 GetNumberTiles();
};