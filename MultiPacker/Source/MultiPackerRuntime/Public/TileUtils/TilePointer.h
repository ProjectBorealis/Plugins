/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include <Templates/UniquePtr.h>
#include "Engine/Texture2D.h"
#include "MultiPackerBaseEnums.h"
#include "TilePointer.generated.h"

UCLASS(hidecategories = object)
class MULTIPACKERRUNTIME_API UTilePointer : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Default")
		UTextureRenderTarget2D* TileRT;
	UPROPERTY(VisibleAnywhere, Category = "Default")
		UTexture2D* TileTexture; 
	UPROPERTY(VisibleAnywhere, Category = "DebugName")
		FString NameUTile = "Default";
	UPROPERTY()
		uint16 TileWidth;
	UPROPERTY()
		uint16 TileHeight;

	UPROPERTY()
		FVector2D TilePadding;

	UPROPERTY()
		FTileDatabase TileDatabase;
	//From an Input RenderTarget fill the TileData and convert it to a Texture
	void GenerateFromRT(UTextureRenderTarget2D* InTileRT, uint16 InTileWidth, uint16 InTileHeight);
	void GenerateFromMaterial(UObject* InWorldContextObject, UTextureRenderTarget2D* RTMaterial, UMaterialInterface* Material, uint16 new_width, uint16 new_height);
	void GenerateFromTexture(UTexture2D* Texture, const int InTileWidth, const int InTileHeight);
	TArray<UTilePointer*> SplitTile(uint16 VerticalSlices, uint16 HorizontalSlices);
	UTilePointer* GetSplitTile(FRectSizePadding SizePadding);
	void FromChannelToTexture(EChannelSelectionInput Channel);
	void InvertAllChannels();
	void ChangeResolution(uint16 new_width, uint16 new_height, UTilePointer* InTile);
	void GenerateAndSetArrayTilesOnRenderTarget(UObject* InWorldContextObject, UTextureRenderTarget2D* OutputTexture, TArray<UTilePointer*> TileArray, uint16 Width, uint16 Height);
	void SDFGenParallel(uint16 Radius, uint16 V_size, uint16 H_size);
	void GenerateThumbnailFromTile(UTilePointer* Tile);
	TArray<FColor> GetColorArray();
	UTexture2D* FillTextureOutput(UObject* Outer, const FString Name, EObjectFlags Flags);
	void GenerateTextureCanvas(const uint16 Width, const uint16 Height);

	static TArray<UTilePointer*> DoFinalTextures(TArray<UTilePointer*> InTileBinPack, uint16 InMasks, bool Alpha);
	static TArray<UTilePointer*> SortArrayTiles(TArray<UTilePointer*> InTiles);

	UPROPERTY(VisibleAnywhere, Transient, Category = "Default")
		TArray<UTilePointer*> TileBinPack;
	//Private side
private:
	UPROPERTY()
		TArray<FColor> TileData;

	UPROPERTY()
		uint32 TileDimension;

	TArray<float> TileDataFloat;
	TUniquePtr<FUpdateTextureRegion2D> echoUpdateTextureRegion;

	void GenerateAndCombineTexturesOnChannels(uint16 new_width, uint16 new_height, UTilePointer* TileRed, UTilePointer* TileGreen, UTilePointer* TileBlue, UTilePointer* TileAlpha);
	void GenerateAndCombineTexturesOnLayers(uint16 new_width, uint16 new_height, UTilePointer* TileRed_A, UTilePointer* TileRed_B, UTilePointer* TileRed_C, UTilePointer* TileGreen_A, UTilePointer* TileGreen_B, UTilePointer* TileGreen_C, UTilePointer* TileBlue_A, UTilePointer* TileBlue_B, UTilePointer* TileBlue_C, UTilePointer* TileAlpha_A, UTilePointer* TileAlpha_B, UTilePointer* TileAlpha_C);
	uint8 GenerateChannelFromLayer(uint8 Tile_A, uint8 Tile_B, uint8 Tile_C);
	//BinPack To Texture and Database Filler
	static TArray<UTilePointer*> TileAtlas(TArray<UTilePointer*> InTileBinPack, bool Alpha); //private
	static TArray<UTilePointer*> TileChannels(TArray<UTilePointer*> InTileBinPack, uint16 InMasks, bool Alpha); //private
	static UTilePointer* TileRGBA(TArray<UTilePointer*> InTileBinPack, bool Alpha);
	static UTilePointer* TileMultipleRGBA(TArray<UTilePointer*> InTileBinPack, bool Alpha);
	//Channel
	void SetChannelsBinPack(TArray<UTilePointer*> InTileBinPack, EChannelOutput ChannelTexture);
	void SetChannels(UTilePointer* InTileChannel, EChannelOutput ChannelTexture);
	void RegenerateTileData(UTexture2D* Texture = nullptr);
	FTexture* GetResource();
	void GenerateDefaultVars(const uint16 Width, const uint16 Height);
	uint8 GetPixelCombinedRGB(uint32 pixel);
	void GetPointerFromRT();
	void InitializeSDFFloatArray();
	//void setPixelColor(uint8*& pointer, uint8 red, uint8 green, uint8 blue, uint8 alpha);
	void setPixelColor(FColor pointer, uint8 red, uint8 green, uint8 blue, uint8 alpha);
	void ClearTextureCanvas();
	void UpdateTextureCanvas();
	void SetColorArray(uint16 width, uint16 height, const TArray<FColor>& colorArray);
	void FromRTtoTexture(uint16 new_width, uint16 new_height);
};
