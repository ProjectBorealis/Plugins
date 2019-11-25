/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerRuntimeMatNode.generated.h"


class UMultiPackerRuntimeGraph;
class UMultiPackerRuntimeOutputNodeBase;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class MULTIPACKERRUNTIME_API UMultiPackerRuntimeMatNode : public UObject
{
	GENERATED_BODY()

public:
	UMultiPackerRuntimeMatNode();
	virtual ~UMultiPackerRuntimeMatNode();

	//Here you can select and change the Material or Material Instance
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode")
		UMaterialInterface* MaterialBaseInput;

	//Size Vertical of the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode", meta = (ClampMin = "1", ClampMax = "8096", UIMin = "1", UIMax = "8096"))
		int32 Height = 1;

	//Size Horizontal of the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode", meta = (ClampMin = "1", ClampMax = "8096", UIMin = "1", UIMax = "8096"))
		int32 Width = 1;

	//Size Vertical and Horizontal of the Material
	UPROPERTY(EditDefaultsOnly, Category = "MaterialNode")
		ETextureSizeOutputPersonal MaterialSize;

	UMultiPackerRuntimeOutputNodeBase* ParentNode;

	void ChangeBackground(bool error);
	FLinearColor GetBackground() const;
private:
	FLinearColor BackgroundColor;
	UMultiPackerRuntimeGraph* GetGraph() const;
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};

