/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "MultiPackerBaseEnums.h"
#include "BinPack/MultiPackerRuntimeBinPack.h"
#include "MultiPackerRuntimeGraph.generated.h"

// Forward Declarations
class UMultiPackerRuntimeMatNode;
class UMultiPackerDataBase;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;

UCLASS(BlueprintType)
class MULTIPACKERRUNTIME_API UMultiPackerRuntimeGraph : public UObject
{
	GENERATED_BODY()

public:
	UMultiPackerRuntimeGraph();
	~UMultiPackerRuntimeGraph();

	//Output Size of the RenderTarget
	UPROPERTY(EditAnywhere, Category = "OutputConfig")
		ETextureSizeOutputPersonal RenderTargetSizeOutput;

	//Select this if you want to have the Alpha channel active on Texture Output
	UPROPERTY(EditAnywhere, Category = "OutputConfig")
		bool Alpha = false;

	UPROPERTY(VisibleAnywhere, instanced, Category = BinPack)
		UMultiPackerRuntimeBinPack* RuntimeData;

	UPROPERTY(VisibleAnywhere, Category = BinPack)
		int Size;

	UPROPERTY(VisibleAnywhere, Category = BinPack)
		int Pages;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BinPack)
		TMap<FName, FMaterialDataBinPack> MaterialMap;

	UPROPERTY(BlueprintReadOnly, Category = BinPack)
		TArray<UTextureRenderTarget2D*>  RenderTargetPages;

	TArray<UMultiPackerRuntimeMatNode*> MatNodes;
	TSubclassOf<UMultiPackerRuntimeMatNode> MatNodeType;

	UFUNCTION(BlueprintCallable, Category = BinPack, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		void RuntimePreProcess(UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable, Category = BinPack, meta = ( WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		void RetrieveMaterialInfo(UObject* WorldContextObject, UMaterialInterface* Material, /*out*/ UTextureRenderTarget2D*& RT_Output, /*out*/ FMaterialDataBinPack& Database);
	UFUNCTION(BlueprintCallable, Category = BinPack, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		void SetTextureRuntimeOnMaterial(UObject* WorldContextObject, UMaterialInterface* Material, UMaterialInstanceDynamic* MaterialToSet, FName Texture, FName Booleans, FName SizePadding);
	UFUNCTION(BlueprintCallable, Category = BinPack, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		UTexture2D* GetTileTexture(UObject* WorldContextObject, UMaterialInterface* MaterialToGet);
	UFUNCTION(BlueprintCallable, Category = BinPack, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		void ValidationOnRuntime(UObject* WorldContextObject);

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		class UEdGraph* EdGraph;
#endif
	void ClearGraph();

	void ProcessNodes(UObject* WorldContextObject);
private:
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};