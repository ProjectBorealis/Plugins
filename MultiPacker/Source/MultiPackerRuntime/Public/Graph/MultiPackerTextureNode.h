/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "MultiPackerImageCore.h"
#include "MultiPackerTextureNode.generated.h"

class UMultiPacker;
class UTextureRenderTarget2D;
class UTilePointer;
class UTexture;
class UTilePointer;

UCLASS()
class MULTIPACKERRUNTIME_API UMultiPackerTextureNode : public UObject
{
	GENERATED_BODY()

public:
	UMultiPackerTextureNode();
	virtual ~UMultiPackerTextureNode();
	TArray<UTilePointer*> BaseTile;

	//Here you set the Texture to be processed by the Node Parameters
	UPROPERTY(EditAnywhere, Category = "TextureNode")
	UTexture* TextureInput;

	//This option is Individual for the node, the size of the Tile has a several impact on performance
	UPROPERTY(EditAnywhere, Category = "TextureNode")
		bool SDF = false;

	UPROPERTY(EditAnywhere, Category = "TextureNode", meta = (ClampMin = "1", ClampMax = "64", UIMin = "1", UIMax = "64", EditCondition = SDF))
		int32 SDF_Radius = 64;

	//This option is Individual for the node and do some changes on the Texture to get a better result
	UPROPERTY(EditAnywhere, Category = "TextureNode")
		bool MSDF = false;

	//Number of Vertical Tiles on the Texture to divide them
	UPROPERTY(EditAnywhere, Category = "TextureNode|Tiled Options", meta = (ClampMin = "1", ClampMax = "50", UIMin = "1", UIMax = "50"))
		int32 TilesVertical = 1;

	//Number of Horizontal Tiles on the Texture to divide them
	UPROPERTY(EditAnywhere, Category = "TextureNode|Tiled Options", meta = (ClampMin = "1", ClampMax = "50", UIMin = "1", UIMax = "50"))
		int32 TilesHorizontal = 1;

	//Textures can have a Size non squared
	UPROPERTY(VisibleAnywhere, Category = "TextureNode|Non Square Size")
		bool RectangleSize = false;

	//Select to autoFill the Size of the Texture Input
	UPROPERTY(EditDefaultsOnly, Category = "TextureNode|Non Square Size", meta = (EditCondition = RectangleSize))
		ESizeTexture AutoSizeFill;

	//Select to autoFill the Size of the Texture Input
	UPROPERTY(EditDefaultsOnly, Category = "TextureNode|Non Square Size", meta = (EditCondition = RectangleSize))
		ETextureSizeOutputPersonal SetSizeByParameter = ETextureSizeOutputPersonal::EMCE_Option1;

	//Size Vertical of the Texture
	UPROPERTY(EditDefaultsOnly, Category = "TextureNode|Non Square Size", meta = (ClampMin = "1", ClampMax = "8096", UIMin = "1", UIMax = "8096", EditCondition = RectangleSize))
		int32 SizeVertical = 32;

	//Size Horizontal of the Texture
	UPROPERTY(EditDefaultsOnly, Category = "TextureNode|Non Square Size", meta = (ClampMin = "1", ClampMax = "8096", UIMin = "1", UIMax = "8096", EditCondition = RectangleSize))
		int32 SizeHorizontal = 32;

	//Select to autoFill the names of the DataBase
	UPROPERTY(EditAnywhere, Category = "TextureNode_TileDatabase")
		ENameTexture AutoNameFill;

	//Database with the information of every Tile
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "TextureNode_TileDatabase")
		TArray<FTileThumbDatabase> ATexTileData;

	UPROPERTY(Transient)
		TArray<UTilePointer*> Thumbnails;

	int32 numTiles = 1;
	void SetDataBaseTiles(TArray<UTextureRenderTarget2D*> RT_Array);
	void SetDataBaseTiles(TArray<UTilePointer*> TileThumbs);
	bool CompareSizes(int32 Vertical, int32 Horizontal);
	bool NodeCleanErrors() const;
	void SetErrorTiles(bool error);
	bool CanSetThumbnail() const;
	FString GetNodeTitle() const;
	FLinearColor GetBackground() const;
	void ChangeBackground(bool error);
private:
	UMultiPacker* GetGraph() const;
	bool CanProcess() const;
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
	bool ErrorSize = false;
	bool ErrorTiles = false;
	FLinearColor BackgroundColor;
};
