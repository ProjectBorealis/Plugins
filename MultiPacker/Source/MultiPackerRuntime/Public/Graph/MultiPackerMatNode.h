/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "MultiPackerImageCore.h"
#include "MultiPackerMatNode.generated.h"

//class UMultiPackerOutputNodeBase;
class UMultiPacker;
class UMaterialInterface;
class UTilePointer;

UCLASS()
class MULTIPACKERRUNTIME_API UMultiPackerMatNode : public UObject
{
	GENERATED_BODY()

public:
	UMultiPackerMatNode();

	//Here you can select and change the Material or Material Instance
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode")
		UMaterialInterface* MaterialBaseInput;

	//This option is Individual for the node, the size of the Tile has a several impact on performance
	UPROPERTY(EditAnywhere, Category = "MaterialNode")
		bool SDF = false;

	UPROPERTY(EditAnywhere, Category = "MaterialNode", meta = (ClampMin = "1", ClampMax = "64", UIMin = "1", UIMax = "64", EditCondition = SDF))
		int32 SDF_Radius = 64;
	
	//Materials can be splitted to get Individual Textures from there, this work for every numprint
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode|Tiled Options")
		bool Atlas = false;

	//Number of Vertical Tiles on the Material to divide them
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode|Tiled Options", meta = (ClampMin = "1", ClampMax = "50", UIMin = "1", UIMax = "50", EditCondition = Atlas))
		int32 TilesVertical = 1;

	//Number of Horizontal Tiles on the Material to divide them
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode|Tiled Options", meta = (ClampMin = "1", ClampMax = "50", UIMin = "1", UIMax = "50", EditCondition = Atlas))
		int32 TilesHorizontal = 1;

	//Materials can have a Size non squared
	UPROPERTY(VisibleAnywhere, Category = "MaterialNode|Non Square Size")
		bool RectangleSize = false;

	//Select to autoFill the Size of the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode|Non Square Size", meta = (EditCondition = RectangleSize))
		ETextureSizeOutputPersonal SetSizeByParameter = ETextureSizeOutputPersonal::EMCE_Option1;

	//Size Vertical of the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode|Non Square Size", meta = (ClampMin = "1", ClampMax = "8096", UIMin = "1", UIMax = "8096", EditCondition = RectangleSize))
		int32 SizeVertical = 32;

	//Size Horizontal of the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode|Non Square Size", meta = (ClampMin = "1", ClampMax = "8096", UIMin = "1", UIMax = "8096", EditCondition = RectangleSize))
		int32 SizeHorizontal = 32;
	
	//UMultiPackerOutputNodeBase* ParentNode;
	int32 NumTiles = 1;
	//from multipackerOutput error collector
	bool CompareSizes(int32 Vertical, int32 Horizontal);
	void SetErrorTiles(bool error);

	void ChangeBackground(bool error);
	bool CanProcess();
	FLinearColor GetBackground() const;
public:
	bool ErrorTiles = false;
	bool ErrorSize = false;
	bool bCanProcess = false;
	bool bDebug_MatNode = false;
	bool bMatAsset = false;
	bool DirtyMaterial = false;

private:
	FLinearColor BackgroundColor;
	UMultiPacker* GetGraph() const;
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};

