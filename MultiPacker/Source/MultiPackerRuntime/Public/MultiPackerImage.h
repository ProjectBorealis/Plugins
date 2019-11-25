/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "CoreMinimal.h"
#include "Components/Image.h"
#include "MultiPackerImageCore.h"
#include "MultiPackerImage.generated.h"

class UMultiPackerDataBase;
class UMultiPackerLayerDatabase;

UCLASS()
class UMultiPackerImage : public UImage
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Appearance)
		EMultiPackerImageLayer TypeLayer = EMultiPackerImageLayer::EMCE_Option1;

	UPROPERTY()//Old code version VisibleAnywhere, Category = "Appearance|Layers", AdvancedDisplay)
		bool layer_1 = true;
	UPROPERTY()
		bool layer_2 = false;
	UPROPERTY()
		bool layer_3 = false;

	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base", meta = (EditCondition = layer_1))
		FLayerDatabase LayerBase = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition", meta = (EditCondition = layer_2))
		FLayerDatabase LayerAddition = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail", meta = (EditCondition = layer_3))
		FLayerDatabase LayerDetail = FLayerDatabase();
	/////////////////////////BASE
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base")
		FLinearColor BaseColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base")
		FLinearColor BaseColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base")
		float BaseOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float BaseSDFThresold = 0.5;

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base")
		bool BaseStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Base")
		UMultiPackerLayerDatabase* LayerBaseStyle;

	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetBaseColorOutline(FLinearColor InBaseColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetBaseColorInterior(FLinearColor InBaseColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetBaseOutlineThresold(float InBaseOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetBaseSDFThresold(float InBaseSDFThresold);
	/////////////////////////ADDITION
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition")
		FLinearColor AdditionColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition")
		FLinearColor AdditionColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition")
		float AdditionOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float AdditionSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetAdditionColorOutline(FLinearColor InAdditionColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetAdditionColorInterior(FLinearColor InAdditionColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetAdditionOutlineThresold(float InAdditionOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetAdditionSDFThresold(float InAdditionSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition")
		bool AdditionStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Addition")
		UMultiPackerLayerDatabase* LayerAdditionStyle;
	/////////////////////////DETAIL
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail")
		FLinearColor DetailColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail")
		FLinearColor DetailColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail")
		float DetailOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float DetailSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetDetailColorOutline(FLinearColor InDetailColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetDetailColorInterior(FLinearColor InDetailColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetDetailOutlineThresold(float InDetailOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Image|Appearance")
		void SetDetailSDFThresold(float InDetailSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail")
		bool DetailStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Layers|Detail")
		UMultiPackerLayerDatabase* LayerDetailStyle;
	// UWidget interface
	void SynchronizeProperties() override;
	// End of UWidget interface

	void SetVarsFromStyle(FLayerDatabase InLayerBase, FLinearColor& InOutline, FLinearColor& InColor, float& InOutlineThresold, float& InSDFThresold);
private:
#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
	/*FLayerDatabase ProcessStyle(FLayerDatabase InLayer, UMultiPackerLayerDatabase* InStyle);*/
	/*FLayerDatabase ProcessChanges(FLayerDatabase Layer);*/

	UMaterialInstanceDynamic* MID_Image;

	void ProcessChanges();

};

