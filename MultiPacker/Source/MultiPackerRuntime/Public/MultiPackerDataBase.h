/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "CoreMinimal.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerDataBase.generated.h"

class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;
class UMaterialParameterCollection;
class UTexture;

UCLASS()
class MULTIPACKERRUNTIME_API UMultiPackerDataBase : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	/** Structure to use for each row of the table, must inherit from FTableRowBase */
	UPROPERTY()
		UScriptStruct*			RowStruct;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		int32 VTile;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		int32 HTile;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		bool Do_SDF = false;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		bool BinPack = false;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		bool Alpha = false;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		EChannelTextureSave ChannelMethod;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		TArray<UTexture2D*> TextureOutput;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		UMaterialParameterCollection* MaterialCollection;

	//Extremelly caution modifing this manually, can broke refences
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "OutputData")
		TMap<FName, FTileDatabase> TileMap;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		int32 TileSize;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		FLinearColor layer_bools;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "OutputData")
		FLinearColor Tiles_Size = FLinearColor(1,1,1,1);
	UFUNCTION(BlueprintCallable, Category = MultiPackerDataBase, meta = (UnsafeDuringActorConstruction = "true"))
		UMaterialInstanceDynamic* GetMaterialTile(const FName Key);
	UFUNCTION(BlueprintCallable, Category = MultiPackerDataBase, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		UMaterialInstanceDynamic* GetButtonMaterialState(UObject* WorldContextObject, const FName KeyIcon, const FName KeyBase, const FName KeySelected, int32 size, FLinearColor Color_Base, FLinearColor Color_Icon_Normal, FLinearColor Color_Icon_Press, FLinearColor Color_Ic_Selected, EStateButton EState, bool selected);
	UFUNCTION(BlueprintCallable, Category = MultiPackerDataBase, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		UTextureRenderTarget2D* GetTextureTile(UObject* WorldContextObject, const FName Key, int size);
	UFUNCTION(BlueprintCallable, Category = MultiPackerDataBase, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		FButtonStyle GetButtonStyle(UObject* WorldContextObject, const FName KeyIcon, const FName KeyBase, const FName KeySelected, int32 size, FLinearColor Color_Base, FLinearColor Color_Icon_Normal, FLinearColor Color_Icon_Press, FLinearColor Color_Ic_Selected, bool selected);

};