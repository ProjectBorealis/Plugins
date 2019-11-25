/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#pragma once
#include "CoreMinimal.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerRuntimeBinPack.generated.h"

class UMaterialInterface;
class UMaxRectsBinPack;
class UTextureRenderTarget2D;

USTRUCT(BlueprintType)
struct FMaterialDataBinPack
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		int32 Width;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		int32 Height;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Default")
		UTextureRenderTarget2D* RT_Material;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		UMaterialInterface* Material;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		int32 NumPage;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
		FRectSizePadding MaterialRect;
};

UCLASS(hidecategories = object)
class MULTIPACKERRUNTIME_API UMultiPackerRuntimeBinPack : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
		TArray<UMaxRectsBinPack*>  BinPackPages;

	UPROPERTY(VisibleAnywhere, Transient, Category = BinPack)
		TArray<UTextureRenderTarget2D*>  RenderTargetPages;

	UPROPERTY()
		UObject* WorldContext;

	UPROPERTY(Transient)
		int Size;

	UPROPERTY(Transient)
		bool Alpha;

	UPROPERTY(Transient)
		TMap<FName, FMaterialDataBinPack> MaterialMap;

	TArray<int> NumPages;
	void Init(UObject* WorldContextObject, int InSize, bool InAlpha);
	void SetMaterial(UMaterialInterface* Material, int32 Width, int32 Height);
	FMaterialDataBinPack* GetMaterialData(FName Material);
	FMaterialDataBinPack AddMaterialToMap(UMaterialInterface* Material, int32 Width, int32 Height);
	int MakeNewPage();
	TArray<UTextureRenderTarget2D*> RuntimePreProcess(UObject* WorldContextObject, TMap<FName, FMaterialDataBinPack> InMaterialMap, int InPages, int InSize, bool InAlpha);
	UTextureRenderTarget2D* SetRTOnCanvas(UTextureRenderTarget2D* OutputTexture, UTextureRenderTarget2D* RT_Array_New, FRectSizePadding Rect, uint16 Width, uint16 Height);
};

