/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "MultiPackerBaseEnums.h"
#include "Engine/TextureDefines.h"
#include "TileUtils/TilePointer.h"
#include "MultiPacker.generated.h"

// Forward Declarations
class UMultiPackerTextureNode;
class UMultiPackerMatNode;
class UMultiPackerDataBase;
class UTextureRenderTarget2D;
class UTexture;

UCLASS(BlueprintType)
class MULTIPACKERRUNTIME_API UMultiPacker : public UObject
{
	GENERATED_BODY()

public:
	UMultiPacker();
	~UMultiPacker();

	uint16 TotalTiles = 0;

	//Select the Size for Every Tile on the Texture Output;
	//the Size of the Texture Output will depend on this
	//and on the number of tiles
	UPROPERTY(EditAnywhere, Category = "TextureSizeOutput", meta = (EditCondition = "!RectangleTiles"))
		ETextureSizeOutput TextureTileSizeOutput = ETextureSizeOutput::EMCE_Option1;

	//Select this to do an Atlas with tiles of different sizes
	UPROPERTY(EditAnywhere, Category = "TextureSizeOutput|Tiled Options")
		bool RectangleTiles;

	// Specifies the different heuristic rules that can be used when deciding where to place a new rectangle.
	UPROPERTY(EditDefaultsOnly, Category = "TextureSizeOutput|Tiled Options", meta = (EditCondition = RectangleTiles))
		EFreeRectChoiceHeuristic RectangleMethod = EFreeRectChoiceHeuristic::EMCE_Option3;

	//Width for the Output Texture
	UPROPERTY(EditDefaultsOnly, Category = "TextureSizeOutput|Tiled Options", meta = (EditCondition = RectangleTiles))
		ETextureSizeOutputPersonal OutputSizeX;

	//Height for the Output Texture
	UPROPERTY(EditDefaultsOnly, Category = "TextureSizeOutput|Tiled Options", meta = (EditCondition = RectangleTiles))
		ETextureSizeOutputPersonal OutputSizeY;

	//Padding for every Tile, This allows to get out the blur border of the nearest tile
	//The Padding X work for right and left, and Y for up and down, Advice: control the values will be doubled.
	//EveryTime will be applied if you like to not have Padding set to 0
	//You can override this setting individual on every node
	UPROPERTY(EditDefaultsOnly, Category = "TextureSizeOutput|Tiled Options", meta = (EditCondition = RectangleTiles))
		FVector2D TilePadding;

	//Select between multiples modes for save the texture on channel; 
	//Multiple saves 3 Textures by Channel
	//One save 1 Texture on Channel
	//Atlas save 1 Texture on RGB/A Channels
	//One_SDF save 1 Texture SDF on Channel , This is the Global option who process all the nodes to SDF, for a granular selection you have the Boolean selection on the nodes
	UPROPERTY(EditAnywhere, Category = "OutputConfig|ChannelsOptions")
		EChannelTextureSave ChannelMethod = EChannelTextureSave::CS_Atlas;

	//Select this if you want to have the Alpha channel active on Texture Output
	UPROPERTY(EditAnywhere, Category = "OutputConfig|ChannelsOptions")
		bool Alpha;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
		int32 TileSize;

	//Make and save the Material Collection
	UPROPERTY(EditAnywhere, Category = "OutputConfig|SaveData")
		bool SaveMaterialCollection = true;

	//Make and save the Material Collection
	UPROPERTY(EditAnywhere, Category = "OutputConfig|SaveData")
		bool SaveDatabase = true;

	//Directory to save the Output Texture
	UPROPERTY(EditAnywhere, Category = "OutputConfig|Location", meta = (RelativeToGameContentDir, ContentDir))
		FDirectoryPath TargetDirectory;

	//Name of the Output Texture
	UPROPERTY(EditAnywhere, Category = "OutputConfig|Location")
		FString TargetName = "None";

	//Bool to activate the change of the Prefix Assets
	UPROPERTY(EditAnywhere, Category = "OutputConfig|Prefix")
		bool EditPrefix = false;
	//Prefix for the MultiPacker Database to use with blueprints
	UPROPERTY(EditAnywhere, Category = "OutputConfig|Prefix", meta = (EditCondition = EditPrefix))
		FString DatabasePrefix = "MPDB_";
	//Prefix for the Output Textures
	UPROPERTY(EditAnywhere, Category = "OutputConfig|Prefix", meta = (EditCondition = EditPrefix))
		FString TexturePrefix = "T_";
	//Prefix for the Material Collection
	UPROPERTY(EditAnywhere, Category = "OutputConfig|Prefix", meta = (EditCondition = EditPrefix))
		FString MaterialcollectionPrefix = "MC_";

	//Bool to activate the change of the Texture Output
	UPROPERTY(EditAnywhere, Category = "OutputConfig|TextureOutput")
		bool EditTexture = false;
	//Defines the Filter for the Texture Output
	UPROPERTY(EditAnywhere, Category = "OutputConfig|TextureOutput", meta = (EditCondition = EditTexture))
		TEnumAsByte<enum TextureFilter> T_TextureFilter = TF_Bilinear;
	//Defines the if the Texture will be saved as a SRGB
	UPROPERTY(EditAnywhere, Category = "OutputConfig|TextureOutput", meta = (EditCondition = EditTexture))
		bool Tex_SRGB = false;
	UPROPERTY(EditAnywhere, Category = "OutputConfig|TextureOutput", meta = (EditCondition = EditTexture))
		TEnumAsByte<enum TextureAddress> Tex_AddressX = TA_Clamp;
	UPROPERTY(EditAnywhere, Category = "OutputConfig|TextureOutput", meta = (EditCondition = EditTexture))
		TEnumAsByte<enum TextureAddress> Tex_AddressY = TA_Clamp;
	UPROPERTY(EditAnywhere, Category = "OutputConfig|TextureOutput", meta = (EditCondition = EditTexture))
		TEnumAsByte<enum ETexturePowerOfTwoSetting::Type> Tex_Power;

	//Texture Output
	UPROPERTY(VisibleAnywhere, Category = "OutputData")
		TArray<UTexture2D*> TextureOutput;

	//Database containing all the Data of the Graph for Blueprints Functions
	UPROPERTY(VisibleAnywhere, Category = "OutputData", meta = (EditCondition = SaveDatabase))
		UMultiPackerDataBase* DataBase;

	//Material Collection with the Individual Information about 
	UPROPERTY(VisibleAnywhere, Category = "OutputData", meta = (EditCondition = SaveMaterialCollection))
		UMaterialParameterCollection* MaterialCollection;

	UPROPERTY(VisibleAnywhere, Transient, Category = "TileDataBaseDebugOnly")
		TMap<FName, FTileDatabase> TileMap;

	UPROPERTY(VisibleAnywhere, Transient, Category = "TileDataBaseDebugOnly")
		TArray<UTilePointer*> TilePointers;

	TArray < TSharedRef<struct FTileDatabase>> NewTileData;

	TArray<UMultiPackerTextureNode*> TexNodes;
	TArray<UMultiPackerMatNode*> MatNodes;
	TSubclassOf<UMultiPackerTextureNode> NodeType;
	TSubclassOf<UMultiPackerMatNode> MatNodeType;
	EObjectFlags Flags;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		class UEdGraph* EdGraph;
#endif
	void ClearGraph();

private:
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
