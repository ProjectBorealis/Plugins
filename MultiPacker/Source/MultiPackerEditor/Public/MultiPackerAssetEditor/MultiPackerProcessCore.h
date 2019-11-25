/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerProcessCore.generated.h"

class UMultiPacker;
class UTilePointer;
class UTextureRenderTarget2D;
class FAssetToolsModule;
class FContentBrowserModule;
class UMaterialInstanceDynamic;

UCLASS(hidecategories = object, MinimalAPI)
class UMultiPackerProcessCore : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	//Variables
	UMultiPacker* BaseInput = NULL;
	uint16 VTile = 1;
	uint16 HTile = 1;
	uint16 TileSize = 32;
	bool Do_SDF = false;

	bool Do_MSDF = false;
	EObjectFlags Flags;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
		TMap<FName, FTileDatabase> TileMap;
	UPROPERTY()
		TArray<UTilePointer*> Output;
	//old code I must try to change it
	void SetGraph(UMultiPacker* EditingGraph);
	uint16 GetNumberTiles();
	//Split Canvas
	void ProcessNodes(UMultiPacker* Graph);
	TArray<UTilePointer*> GetTilesFromNodes();
	TArray<UTilePointer*> TileBinPack(TArray<UTilePointer*> InputTiles, uint16 SizeVertical, uint16 SizeHorizontal, uint16 Masks);
	FRectSizePadding GetSizePaddingWithoutPadding(FRectSizePadding InPadding, FVector2D InTilePadding);
	//code usefull
	uint16 GetNumberOfMasks();
	void GetOutputTileNumbers(uint16 Tiles);
	void SaveDataBase(FString TargetName, FAssetToolsModule& AssetToolsModule, FContentBrowserModule& ContentBrowserModule, uint16 width, uint16 height);
	void CreateMaterialCollection(uint16 width, uint16 height, FAssetToolsModule& AssetToolsModule, FContentBrowserModule& ContentBrowserModule);
	void PopulateMap(uint16 SizeVertical, uint16 SizeHorizontal, uint16 Masks);
	void SaveTextureFromTile(UTilePointer* InTile, FString TextureNum, FAssetToolsModule& AssetToolsModule);
	TArray<FString> PackageName(FString Prefix, FString TextureNum, FAssetToolsModule& AssetToolsModule);
	TArray<FString> TexturePackageName(FString Prefix, FString TextureNum, FAssetToolsModule& AssetToolsModule);
	void SetDataLinearColors(EChannelOutput ChannelOutput, FLinearColor& Channel, FLinearColor& Layer);
};
