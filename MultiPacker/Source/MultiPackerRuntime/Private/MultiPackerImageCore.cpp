/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerImageCore.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerDataBase.h"
#include <Engine/Texture.h>
#include <Materials/MaterialInstanceDynamic.h>
#include "MultiPackerLayerDatabase.h"

UMaterialInstanceDynamic* UMultiPackerImageCore::LayerMaterialInstance(UMaterialInstanceDynamic* MB_Function, FLayerDatabase LayerInfo, FTileDatabase layer_interior, FTileDatabase layer_alpha, FString SuFix)
{
	//Textures
	MB_Function->SetTextureParameterValue(FName(*FString("Color_Tex").Append(SuFix)), Cast<UTexture>(LayerInfo.Color.Database->TextureOutput[layer_interior.Frame]));
	MB_Function->SetTextureParameterValue(FName(*FString("Opacity_Tex").Append(SuFix)), Cast<UTexture>(LayerInfo.Alpha.Database->TextureOutput[layer_alpha.Frame]));
	//Booleans
	MB_Function->SetScalarParameterValue(FName(*FString("bColorSDF").Append(SuFix)), (LayerInfo.SDF_Interior | LayerInfo.MSDF) ? (LayerInfo.Outline ? 1 : 2) : (LayerInfo.UseColor ? (LayerInfo.Outline ? 1 : 2) : 0));
	MB_Function->SetScalarParameterValue(FName(*FString("bOpacitySDF").Append(SuFix)), !LayerInfo.MSDF ? 1 : (LayerInfo.SDF_Alpha ? 1 : 0));
	MB_Function->SetScalarParameterValue(FName(*FString("bSdfOutline").Append(SuFix)), !LayerInfo.MSDF ? (!LayerInfo.Outline ? 2 : 3) : (LayerInfo.Outline ? 1 : 0));//WTF is this on reverse mode
	MB_Function->SetVectorParameterValue(FName(*FString("Color_BooleansTiled").Append(SuFix)), LayerInfo.Color.Database->layer_bools);//r: tiled g:one b:multiple 
	MB_Function->SetVectorParameterValue(FName(*FString("Opacity_BooleansTiled").Append(SuFix)), LayerInfo.Alpha.Database->layer_bools);//r: tiled g:one b:multiple 
	//Scalars
	MB_Function->SetScalarParameterValue(FName(*FString("EdgeSoftness").Append(SuFix)), LayerInfo.EdgeSoftness);
	MB_Function->SetScalarParameterValue(FName(*FString("OutlineThresold").Append(SuFix)), LayerInfo.OutlineThresold);
	MB_Function->SetScalarParameterValue(FName(*FString("SDFThresold").Append(SuFix)), LayerInfo.SDFThresold);
	//Vectors
	MB_Function->SetVectorParameterValue(FName(*FString("Color_Channel").Append(SuFix)), layer_interior.Channel_vec);
	MB_Function->SetVectorParameterValue(FName(*FString("Color_layer").Append(SuFix)), layer_interior.Layer_vec);
	MB_Function->SetVectorParameterValue(FName(*FString("Color_SizePadding").Append(SuFix)), LayerInfo.Color.Database->TileMap.Find(LayerInfo.Color.Name)->SizePadding_vec);
	MB_Function->SetVectorParameterValue(FName(*FString("ColorInterior").Append(SuFix)), LayerInfo.ColorInterior);
	MB_Function->SetVectorParameterValue(FName(*FString("ColorOutline").Append(SuFix)), LayerInfo.ColorOutline);
	MB_Function->SetVectorParameterValue(FName(*FString("Opacity_Channels").Append(SuFix)), layer_alpha.Channel_vec);
	MB_Function->SetVectorParameterValue(FName(*FString("Opacity_Layers").Append(SuFix)), layer_alpha.Layer_vec);
	MB_Function->SetVectorParameterValue(FName(*FString("Opacity_SizePadding").Append(SuFix)), LayerInfo.Alpha.Database->TileMap.Find(LayerInfo.Alpha.Name)->SizePadding_vec);
	return MB_Function;
}

UMaterialInstanceDynamic* UMultiPackerImageCore::CreateMaterial(EMultiPackerImageLayer Layer, UObject *InOuter, FLayerDatabase LayerBase, FLayerDatabase LayerAddition, FLayerDatabase LayerDetail)
{
	UMaterialInstanceDynamic* MB_Function = UMaterialInstanceDynamic::Create(Cast<UMaterial>(UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_1Image.MB_Umg_1Image")), NULL);
	switch (Layer)
	{
	case EMultiPackerImageLayer::EMCE_Option1:
		MB_Function = UMaterialInstanceDynamic::Create(Cast<UMaterial>(LayerBase.Multiple ? UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_1Image_M.MB_Umg_1Image_M") : UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_1Image.MB_Umg_1Image")), NULL);
		MB_Function = SetDataBaseLayer(MB_Function, LayerBase, "");
		break;
	case EMultiPackerImageLayer::EMCE_Option2:
		MB_Function = UMaterialInstanceDynamic::Create(Cast<UMaterial>(LayerBase.Multiple ? UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_2Image_M.MB_Umg_2Image_M") : UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_2Image.MB_Umg_2Image")), NULL);
		MB_Function = SetDataBaseLayer(MB_Function, LayerBase, "_1");
		MB_Function = SetDataBaseLayer(MB_Function, LayerAddition, "_2");
		break;
	case EMultiPackerImageLayer::EMCE_Option3:
		MB_Function = UMaterialInstanceDynamic::Create(Cast<UMaterial>(LayerBase.Multiple ? UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_3Image_M.MB_Umg_3Image_M") : UMultiPackerBaseEnums::LoadAssetFromContent("/MultiPacker/Material/UMG/MB_Umg_3Image.MB_Umg_3Image")), NULL);
		MB_Function = SetDataBaseLayer(MB_Function, LayerBase, "_1");
		MB_Function = SetDataBaseLayer(MB_Function, LayerAddition, "_2");
		MB_Function = SetDataBaseLayer(MB_Function, LayerDetail, "_3");
		break;
	}
	return MB_Function;
}

FLayerDatabase UMultiPackerImageCore::ProcessChanges(FLayerDatabase Layer)
{
	Layer.SDF = false;
	Layer.MSDF = false;
	Layer.SDF_Interior = false;
	Layer.SDF_Alpha = false;
	Layer.CanProcess_Interior = false;
	Layer.CanProcess_Alpha = false;
	if (Layer.Color.Database != NULL)
	{
		Layer.Multiple = Layer.Color.Database->ChannelMethod == EChannelTextureSave::CS_Multiple;
		if (Layer.Color.Database->TileMap.Contains(Layer.Color.Name))
		{
			if (Layer.Color.Database->TileMap.Find(Layer.Color.Name)->SDFNode)
			{
				Layer.SDF = true;
				Layer.SDF_Interior = true;
			}
			if (Layer.Color.Database->TileMap.Find(Layer.Color.Name)->MSDFNode)
			{
				Layer.SDF = true;
				Layer.SDF_Interior = true;
				Layer.MSDF = true;
			}
			Layer.CanProcess_Interior = true;
		}
	}
	if (Layer.Alpha.Database != NULL)
	{
		Layer.Multiple = Layer.Alpha.Database->ChannelMethod == EChannelTextureSave::CS_Multiple;
		if (Layer.Alpha.Database->TileMap.Contains(Layer.Alpha.Name))
		{
			if (Layer.Alpha.Database->TileMap.Find(Layer.Alpha.Name)->SDFNode)
			{
				Layer.SDF_Alpha = true;
			}
			if (Layer.Alpha.Database->TileMap.Find(Layer.Alpha.Name)->MSDFNode)
			{
				Layer.SDF_Alpha = true;
				Layer.MSDF = true;
			}
			Layer.CanProcess_Alpha = true;
		}
	}
	return Layer;
}

FLayerDatabase UMultiPackerImageCore::ProcessStyle(FLayerDatabase InLayer, UMultiPackerLayerDatabase* InStyle)
{
	FLayerDatabase NewLayerBase;
	NewLayerBase.SetData(InStyle->LayerBase);
	if (InLayer.Color.Database != nullptr)
	{
		NewLayerBase.Color.Database = InLayer.Color.Database;
		if (InLayer.Color.Name != FName(TEXT("")))
		{
			NewLayerBase.Color.Name = InLayer.Color.Name;
		}
	}
	if (InLayer.Alpha.Database != nullptr)
	{
		NewLayerBase.Alpha.Database = InLayer.Alpha.Database;
		if (InLayer.Alpha.Name != FName(TEXT("")))
		{
			NewLayerBase.Alpha.Name = InLayer.Alpha.Name;
		}
	}
	NewLayerBase.UseColor = InLayer.UseColor;
	NewLayerBase.Outline = InLayer.Outline;
	NewLayerBase.ColorInterior = InStyle->ColorInterior;
	NewLayerBase.ColorOutline = InStyle->ColorOutline;
	NewLayerBase.EdgeSoftness = InStyle->LayerBase.EdgeSoftness;
	NewLayerBase.OutlineThresold = InStyle->OutlineThresold;
	NewLayerBase.SDFThresold = InStyle->SDFThresold;
	return NewLayerBase;
}

UMaterialInstanceDynamic* UMultiPackerImageCore::SetDataBaseLayer(UMaterialInstanceDynamic* MB_Function, FLayerDatabase LayerBase, FString SuFix)
{
	bool NamesConvalidation = false;
	LayerBase.SDF = false;
	LayerBase.MSDF = false;
	LayerBase.SDF_Interior = false;
	LayerBase.SDF_Alpha = false;
	LayerBase.CanProcess_Interior = false;
	LayerBase.CanProcess_Alpha = false;
	FTileDatabase layer_interior = FTileDatabase();
	FTileDatabase layer_alpha = FTileDatabase();
	if (LayerBase.Color.Database != NULL)
	{
		if (LayerBase.Color.Database->TileMap.Contains(LayerBase.Color.Name))
		{
			NamesConvalidation = true;
			layer_interior.SetFromPointer(LayerBase.Color.Database->TileMap.Find(LayerBase.Color.Name));
			if (layer_interior.SDFNode)
			{
				LayerBase.SDF = true;
				LayerBase.SDF_Interior = true;
			}
			if (layer_interior.MSDFNode)
			{
				LayerBase.SDF = true;
				LayerBase.SDF_Interior = true;
				LayerBase.MSDF = true;
			}
			LayerBase.CanProcess_Interior = true;
		}
	}
	if (LayerBase.Alpha.Database != NULL)
	{
		if (LayerBase.Alpha.Database->TileMap.Contains(LayerBase.Alpha.Name))
		{
			NamesConvalidation = NamesConvalidation ? true : false;
			layer_alpha.SetFromPointer(LayerBase.Alpha.Database->TileMap.Find(LayerBase.Alpha.Name));
			if (layer_alpha.SDFNode)
			{
				LayerBase.SDF_Alpha = true;
			}
			if (layer_alpha.MSDFNode)
			{
				LayerBase.SDF_Alpha = true;
				LayerBase.MSDF = true;
			}
			LayerBase.CanProcess_Alpha = true;
		}
		else
		{
			NamesConvalidation = false;
		}
	}
	
	if ((LayerBase.Color.Database != nullptr && LayerBase.Alpha.Database != nullptr) && (LayerBase.Color.Database->TextureOutput.Num() > 0 && LayerBase.Alpha.Database->TextureOutput.Num() > 0) && NamesConvalidation)
	{
		return LayerMaterialInstance(MB_Function, LayerBase, layer_interior, layer_alpha, SuFix);
	}
	else
	{
		return MB_Function;
	}
}
