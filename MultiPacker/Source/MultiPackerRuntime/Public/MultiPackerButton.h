/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "CoreMinimal.h"
#include "Components/Button.h"
#include "MultiPackerImageCore.h"
#include "MultiPackerButton.generated.h"

UCLASS()
class UMultiPackerButton : public UButton
{
public:
	GENERATED_UCLASS_BODY()

public:
	///////Normal
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal")
		EMultiPackerImageLayer TypeLayerNormal = EMultiPackerImageLayer::EMCE_Option1;

	UPROPERTY(EditAnywhere, Category = "Appearance", AdvancedDisplay)
		bool SetAsImage = true;

	UPROPERTY()//Old code Version VisibleInstanceOnly, Category = "Appearance", AdvancedDisplay)
		bool N_layer_1 = true;
	UPROPERTY()
		bool N_layer_2 = false;
	UPROPERTY()
		bool N_layer_3 = false;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base", meta = (EditCondition = N_layer_1))
		FLayerDatabase N_LayerBase = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition", meta = (EditCondition = N_layer_2))
		FLayerDatabase N_LayerAddition = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail", meta = (EditCondition = N_layer_3))
		FLayerDatabase N_LayerDetail = FLayerDatabase();
	///////Hovered
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered")
		EMultiPackerImageLayer TypeLayerHovered = EMultiPackerImageLayer::EMCE_Option1;

	UPROPERTY()
		bool H_layer_1 = true;
	UPROPERTY()
		bool H_layer_2 = false;
	UPROPERTY()
		bool H_layer_3 = false;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base", meta = (EditCondition = H_layer_1))
		FLayerDatabase H_LayerBase = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition", meta = (EditCondition = H_layer_2))
		FLayerDatabase H_LayerAddition = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail", meta = (EditCondition = H_layer_3))
		FLayerDatabase H_LayerDetail = FLayerDatabase();
	///////Pressed
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed")
		EMultiPackerImageLayer TypeLayerPressed = EMultiPackerImageLayer::EMCE_Option1;

	UPROPERTY()
		bool P_layer_1 = true;
	UPROPERTY()
		bool P_layer_2 = false;
	UPROPERTY()
		bool P_layer_3 = false;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base", meta = (EditCondition = P_layer_1))
		FLayerDatabase P_LayerBase = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition", meta = (EditCondition = P_layer_2))
		FLayerDatabase P_LayerAddition = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail", meta = (EditCondition = P_layer_3))
		FLayerDatabase P_LayerDetail = FLayerDatabase();
	///////Disabled
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled")
		bool SetDisabledStyle = false;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled", meta = (EditCondition = SetDisabledStyle))
		EMultiPackerImageLayer TypeLayerDisabled = EMultiPackerImageLayer::EMCE_Option1;

	UPROPERTY()
		bool D_layer_1 = true;
	UPROPERTY()
		bool D_layer_2 = false;
	UPROPERTY()
		bool D_layer_3 = false;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base", meta = (EditCondition = D_layer_1))
		FLayerDatabase D_LayerBase = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition", meta = (EditCondition = D_layer_2))
		FLayerDatabase D_LayerAddition = FLayerDatabase();
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail", meta = (EditCondition = D_layer_3))
		FLayerDatabase D_LayerDetail = FLayerDatabase();

	// UWidget interface
	void SynchronizeProperties() override;
	// End of UWidget interface

private:
	UMaterialInstanceDynamic* MID_Normal;
	UMaterialInstanceDynamic* MID_Hovered;
	UMaterialInstanceDynamic* MID_Pressed;
	UMaterialInstanceDynamic* MID_Disabled;

#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif

	void ProcessChanges();
	void SetVarsFromStyle(FLayerDatabase InLayerBase, FLinearColor& InOutline, FLinearColor& InColor, float& InOutlineThresold, float& InSDFThresold);
	void ProcessBooleansLayers(EMultiPackerImageLayer TypeLayer, bool& Layer_1, bool& Layer_2, bool& Layer_3);
	void ProcessLayerDatabaseAndStyle(EMultiPackerImageLayer TypeLayer, FLayerDatabase& Base, FLayerDatabase& Addition, FLayerDatabase& Detail, 
		UMultiPackerLayerDatabase* StyleBase, UMultiPackerLayerDatabase* StyleAddition, UMultiPackerLayerDatabase* StyleDetail, 
		FLinearColor& InBOutline, FLinearColor& InBColor, float& InBOutlineThresold, float& InBSDFThresold, 
		FLinearColor& InAOutline, FLinearColor& InAColor, float& InAOutlineThresold, float& InASDFThresold, 
		FLinearColor& InDOutline, FLinearColor& InDColor, float& InDOutlineThresold, float& InDSDFThresold,
		bool& bnormal, bool& baddition, bool& bdetail );
public:
	///////Normal
	/////////////////////////BASE
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base")
		FLinearColor NormalBaseColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base")
		FLinearColor NormalBaseColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base")
		float NormalBaseOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float NormalBaseSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalBaseColorOutline(FLinearColor InBaseColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalBaseColorInterior(FLinearColor InBaseColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalBaseOutlineThresold(float InBaseOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalBaseSDFThresold(float InBaseSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base")
		bool N_BaseStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Base")
		UMultiPackerLayerDatabase* LayerNormalBaseStyle;
	/////////////////////////ADDITION
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition")
		FLinearColor NormalAdditionColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition")
		FLinearColor NormalAdditionColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition")
		float NormalAdditionOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float NormalAdditionSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalAdditionColorOutline(FLinearColor InAdditionColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalAdditionColorInterior(FLinearColor InAdditionColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalAdditionOutlineThresold(float InAdditionOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalAdditionSDFThresold(float InAdditionSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition")
		bool N_AdditionStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Addition")
		UMultiPackerLayerDatabase* LayerNormalAdditionStyle;
	/////////////////////////DETAIL
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail")
		FLinearColor NormalDetailColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail")
		FLinearColor NormalDetailColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail")
		float NormalDetailOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float NormalDetailSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalDetailColorOutline(FLinearColor InDetailColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalDetailColorInterior(FLinearColor InDetailColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalDetailOutlineThresold(float InDetailOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetNormalDetailSDFThresold(float InDetailSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail")
		bool N_DetailStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Normal|Detail")
		UMultiPackerLayerDatabase* LayerNormalDetailStyle;
	///////Hovered
	/////////////////////////BASE
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base")
		FLinearColor HoveredBaseColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base")
		FLinearColor HoveredBaseColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base")
		float HoveredBaseOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float HoveredBaseSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredBaseColorOutline(FLinearColor InBaseColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredBaseColorInterior(FLinearColor InBaseColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredBaseOutlineThresold(float InBaseOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredBaseSDFThresold(float InBaseSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base")
		bool H_BaseStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Base")
		UMultiPackerLayerDatabase* LayerHoveredBaseStyle;
	/////////////////////////ADDITION
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition")
		FLinearColor HoveredAdditionColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition")
		FLinearColor HoveredAdditionColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition")
		float HoveredAdditionOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float HoveredAdditionSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredAdditionColorOutline(FLinearColor InAdditionColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredAdditionColorInterior(FLinearColor InAdditionColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredAdditionOutlineThresold(float InAdditionOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredAdditionSDFThresold(float InAdditionSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition")
		bool H_AdditionStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Addition")
		UMultiPackerLayerDatabase* LayerHoveredAdditionStyle;
	/////////////////////////DETAIL
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail")
		FLinearColor HoveredDetailColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail")
		FLinearColor HoveredDetailColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail")
		float HoveredDetailOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float HoveredDetailSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredDetailColorOutline(FLinearColor InDetailColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredDetailColorInterior(FLinearColor InDetailColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredDetailOutlineThresold(float InDetailOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetHoveredDetailSDFThresold(float InDetailSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail")
		bool H_DetailStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Hovered|Detail")
		UMultiPackerLayerDatabase* LayerHoveredDetailStyle;
	///////Pressed
	/////////////////////////BASE
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base")
		FLinearColor PressedBaseColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base")
		FLinearColor PressedBaseColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base")
		float PressedBaseOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float PressedBaseSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedBaseColorOutline(FLinearColor InBaseColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedBaseColorInterior(FLinearColor InBaseColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedBaseOutlineThresold(float InBaseOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedBaseSDFThresold(float InBaseSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base")
		bool P_BaseStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Base")
		UMultiPackerLayerDatabase* LayerPressedBaseStyle;
	/////////////////////////ADDITION
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition")
		FLinearColor PressedAdditionColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition")
		FLinearColor PressedAdditionColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition")
		float PressedAdditionOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float PressedAdditionSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedAdditionColorOutline(FLinearColor InAdditionColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedAdditionColorInterior(FLinearColor InAdditionColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedAdditionOutlineThresold(float InAdditionOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedAdditionSDFThresold(float InAdditionSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition")
		bool P_AdditionStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Addition")
		UMultiPackerLayerDatabase* LayerPressedAdditionStyle;
	/////////////////////////DETAIL
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail")
		FLinearColor PressedDetailColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail")
		FLinearColor PressedDetailColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail")
		float PressedDetailOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float PressedDetailSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedDetailColorOutline(FLinearColor InDetailColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedDetailColorInterior(FLinearColor InDetailColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedDetailOutlineThresold(float InDetailOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetPressedDetailSDFThresold(float InDetailSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail")
		bool P_DetailStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Pressed|Detail")
		UMultiPackerLayerDatabase* LayerPressedDetailStyle;
	///////Disabled
	/////////////////////////BASE
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base")
		FLinearColor DisabledBaseColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base")
		FLinearColor DisabledBaseColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base")
		float DisabledBaseOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float DisabledBaseSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledBaseColorOutline(FLinearColor InBaseColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledBaseColorInterior(FLinearColor InBaseColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledBaseOutlineThresold(float InBaseOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledBaseSDFThresold(float InBaseSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base")
		bool D_BaseStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Base")
		UMultiPackerLayerDatabase* LayerDisabledBaseStyle;
	/////////////////////////ADDITION
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition")
		FLinearColor DisabledAdditionColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition")
		FLinearColor DisabledAdditionColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition")
		float DisabledAdditionOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float DisabledAdditionSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledAdditionColorOutline(FLinearColor InAdditionColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledAdditionColorInterior(FLinearColor InAdditionColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledAdditionOutlineThresold(float InAdditionOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledAdditionSDFThresold(float InAdditionSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition")
		bool D_AdditionStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Addition")
		UMultiPackerLayerDatabase* LayerDisabledAdditionStyle;
	/////////////////////////DETAIL
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail")
		FLinearColor DisabledDetailColorOutline;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail")
		FLinearColor DisabledDetailColorInterior;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail")
		float DisabledDetailOutlineThresold = 0.5;
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail", meta = (ClampMin = "-0.7", ClampMax = "0.7", UIMin = "-0.7", UIMax = "0.7"))
		float DisabledDetailSDFThresold = 0.5;
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledDetailColorOutline(FLinearColor InDetailColorOutline);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledDetailColorInterior(FLinearColor InDetailColorInterior);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledDetailOutlineThresold(float InDetailOutlineThresold);
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
		void SetDisabledDetailSDFThresold(float InDetailSDFThresold);

	//Allow to change the style if the asset its modified
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail")
		bool D_DetailStyleChanged = false;
	//Every Variable Changed out of the default overwrites this Style
	UPROPERTY(EditAnywhere, Category = "Appearance|Disabled|Detail")
		UMultiPackerLayerDatabase* LayerDisabledDetailStyle;
};

