/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerDataBase.h"
#include "Engine/StreamableManager.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Styling/SlateBrush.h"
#include <Materials/MaterialInstanceDynamic.h>
#include <Styling/SlateTypes.h>


UMultiPackerDataBase::UMultiPackerDataBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UMaterialInstanceDynamic* UMultiPackerDataBase::GetMaterialTile(const FName Key)
{
	if (TileMap.Contains(Key))
	{
		FTileDatabase* NewTile = TileMap.Find(Key);
		UMaterialInterface* MI_Dyn = Cast<UMaterial>(UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_Tile.MB_Umg_Tile"));
		UMaterialInstanceDynamic* MI_DynTile = UMaterialInstanceDynamic::Create(MI_Dyn, this);
		MI_DynTile->SetTextureParameterValue("Tex", Cast<UTexture>(TextureOutput[NewTile->Frame]) );
		MI_DynTile->SetVectorParameterValue("Channel", NewTile->Channel_vec);
		MI_DynTile->SetVectorParameterValue("Booleans", layer_bools);
		MI_DynTile->SetVectorParameterValue("SizePadding", NewTile->SizePadding_vec);
		return MI_DynTile;
	}
	else
	{
		return NULL;
	}
}

UMaterialInstanceDynamic* UMultiPackerDataBase::GetButtonMaterialState(UObject* WorldContextObject, const FName KeyIcon, const FName KeyBase, const FName KeySelected, int32 size, FLinearColor Color_Base, FLinearColor Color_Icon_Normal, FLinearColor Color_Icon_Press, FLinearColor Color_Ic_Selected, EStateButton EState, bool selected)
{
	UMaterialInstanceDynamic* MI_DynTile1 = GetMaterialTile(KeyIcon);//Icon
	UTextureRenderTarget2D* RT_Tex1 = UMultiPackerBaseEnums::GenerateRenderTarget(size, size, Alpha);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContextObject, RT_Tex1, MI_DynTile1);
	UMaterialInstanceDynamic* MI_DynTile2 = GetMaterialTile(KeyBase);//Base
	UTextureRenderTarget2D* RT_Tex2 = UMultiPackerBaseEnums::GenerateRenderTarget(size, size, Alpha);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContextObject, RT_Tex2, MI_DynTile2);
	UMaterialInterface* MI_Dyn;
	if (selected)
	{
		MI_Dyn = Cast<UMaterialInstance>(UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MI_IconSelected.MI_IconSelected"));
	}
	else
	{
		MI_Dyn = Cast<UMaterial>(UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Icon.MB_Icon"));
	}
	UMaterialInstanceDynamic* MI_DynTile = UMaterialInstanceDynamic::Create(MI_Dyn, this);
	MI_DynTile->SetTextureParameterValue("Tex1", RT_Tex1);
	MI_DynTile->SetTextureParameterValue("Tex2", RT_Tex2);
	MI_DynTile->SetVectorParameterValue("Color2", Color_Base);
	FLinearColor Color1;
	switch (EState)
	{
	case EStateButton::EMCE_Option1://Normal
		Color1 = Color_Icon_Normal;
		break;
	case EStateButton::EMCE_Option2://Press
		Color1 = Color_Icon_Press;
		break;
	case EStateButton::EMCE_Option3://Indicator
		Color1 = Color_Ic_Selected;
		break;
	case EStateButton::EMCE_Option4://Single
		Color1 = Color_Icon_Normal;
		break;
	}
	MI_DynTile->SetVectorParameterValue("Color1", Color1);
	if (selected)
	{
		UMaterialInstanceDynamic* MI_DynTile3 = GetMaterialTile(KeySelected);//Selected
		UTextureRenderTarget2D* RT_Tex3 = UMultiPackerBaseEnums::GenerateRenderTarget(size, size, Alpha);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContextObject, RT_Tex3, MI_DynTile3);
		MI_DynTile->SetVectorParameterValue("Color3", Color_Ic_Selected);
		MI_DynTile->SetTextureParameterValue("Tex3", RT_Tex3);
	}
	return MI_DynTile;
}

UTextureRenderTarget2D* UMultiPackerDataBase::GetTextureTile(UObject* WorldContextObject, const FName Key, int size)
{
	UMaterialInstanceDynamic* NewMat = GetMaterialTile(Key);
	UTextureRenderTarget2D* RT_new = UMultiPackerBaseEnums::GenerateRenderTarget(size, size, Alpha);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContextObject, RT_new, NewMat);
	return RT_new;
}

FButtonStyle UMultiPackerDataBase::GetButtonStyle(UObject* WorldContextObject, const FName KeyIcon, const FName KeyBase, const FName KeySelected, int32 size, FLinearColor Color_Base, FLinearColor Color_Icon_Normal, FLinearColor Color_Icon_Press, FLinearColor Color_Ic_Selected, bool selected)
{
	if (size == 0)
	{
		return FButtonStyle();//Avoiding error on construction
	}
	UMaterialInstanceDynamic* MI_DynTile1 = GetButtonMaterialState(WorldContextObject, KeyIcon, KeyBase, KeySelected, size, Color_Base, Color_Icon_Normal, Color_Icon_Press, Color_Ic_Selected, EStateButton::EMCE_Option1, selected);
	UMaterialInstanceDynamic* MI_DynTile2 = GetButtonMaterialState(WorldContextObject, KeyIcon, KeyBase, KeySelected, size, Color_Base, Color_Icon_Normal, Color_Icon_Press, Color_Ic_Selected, EStateButton::EMCE_Option2, selected);
	FSlateBrush Normal;
	Normal.SetResourceObject(MI_DynTile1);
	Normal.ImageSize = FVector2D(size, size);
	FSlateBrush Pressed;
	Pressed.SetResourceObject(MI_DynTile2);
	Pressed.ImageSize = FVector2D(size, size);
	FButtonStyle AButtonStyle = FButtonStyle();
	AButtonStyle.SetNormal(Normal);
	AButtonStyle.SetHovered(Normal);
	AButtonStyle.SetPressed(Pressed);
	return AButtonStyle;
}

