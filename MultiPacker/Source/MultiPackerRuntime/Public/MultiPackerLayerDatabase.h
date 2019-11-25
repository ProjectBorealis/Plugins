/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "CoreMinimal.h"
#include "MultiPackerImageCore.h"
#include "TileUtils/TileCopy.h"
#include "MultiPackerLayerDatabase.generated.h"

class UMaterialInstanceDynamic;

UCLASS()
class MULTIPACKERRUNTIME_API UMultiPackerLayerDatabase : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Appearance")
		FLayerDatabase LayerBase = FLayerDatabase();

	UPROPERTY(EditAnywhere, Category = "Appearance")
		FLinearColor ColorOutline;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		FLinearColor ColorInterior;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		float OutlineThresold = 0.5;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		float SDFThresold = 0.5;

	UPROPERTY(EditAnywhere, Category = "GetNodesToClipboard")
		UTileCopy* Button;

	UPROPERTY()
	UMaterialInstanceDynamic* Material;
};
