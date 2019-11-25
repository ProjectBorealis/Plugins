/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "DetailsProperty/FTileCopyCustomization.h"
#include "CoreMinimal.h"
#include <DetailLayoutBuilder.h>
#include "Widgets/Text/STextBlock.h"
#include "MultiPackerLayerDatabase.h"
#include "MultiPackerDataBase.h"
#include <Widgets/Input/SButton.h>
#include <IDocumentation.h>
#include <Engine/Texture2D.h>
#include <Materials/MaterialParameterCollection.h>
#include "Materials/MaterialInstanceDynamic.h"
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 18)
#include "HAL/PlatformApplicationMisc.h"
#endif
#define LOCTEXT_NAMESPACE "FTileCopyCustomizationLayout"

TSharedRef<IPropertyTypeCustomization> FTileCopyCustomizationLayout::MakeInstance()
{
	return MakeShareable(new FTileCopyCustomizationLayout());
}

void FTileCopyCustomizationLayout::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	static TArray<UObject*> OuterObjects;
	OuterObjects.Reset();

	InStructPropertyHandle->GetOuterObjects(OuterObjects);
	for (int loop=0; loop < OuterObjects.Num(); loop++)
	{
		FString name=OuterObjects[loop]->GetName();
		UE_LOG(LogTemp, Log, TEXT("UMultiPacker: %s"), *name);
		LayerDatabase=Cast<UMultiPackerLayerDatabase>(OuterObjects[loop]);
	}
}

void FTileCopyCustomizationLayout::CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
			#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
				StructBuilder.AddChildContent(LOCTEXT("NodesMCC", "MaterialCollection Nodes"))
			#endif
			#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
					StructBuilder.AddCustomRow(LOCTEXT("NodesMCC", "MaterialCollection Nodes"))
			#endif
					.NameContent()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NodesMC", "MaterialCollection Nodes"))
						.Font(StructCustomizationUtils.GetRegularFont())
					]
					.ValueContent()
						.MinDesiredWidth(500)
					[
						SNew(SBox)
						.WidthOverride(120.f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(this, &FTileCopyCustomizationLayout::OnButtonMCPressed)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("MaterialCollection", "Get the Material Nodes of this selection"))
						]
						]
					];
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
					StructBuilder.AddChildContent(LOCTEXT("NodesLCC", "LinearColor Nodes"))
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
						StructBuilder.AddCustomRow(LOCTEXT("NodesLCC", "LinearColor Nodes"))
#endif
						.NameContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("NodesLC", "LinearColor Nodes"))
						.Font(StructCustomizationUtils.GetRegularFont())
						]
					.ValueContent()
						.MinDesiredWidth(500)
						[
							SNew(SBox)
							.WidthOverride(120.f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(this, &FTileCopyCustomizationLayout::OnButtonLCPressed)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("LinearColor", "Get the Material Nodes of this selection"))
						]
						]
						];
}

FReply FTileCopyCustomizationLayout::OnButtonMCPressed()
{
	GetNodesOnText(true);
	return FReply::Handled();
}

FReply FTileCopyCustomizationLayout::OnButtonLCPressed()
{
	GetNodesOnText(false);
	return FReply::Handled();
}

void FTileCopyCustomizationLayout::GetNodesOnText(bool bMaterialCollection)
{
	//Get all the variables from the LayerDatabase, to string to replace on the Text
	//-Scalar Booleans Part
	//ColorSDFName
	FString ColorSDF = (LayerDatabase->LayerBase.SDF_Interior | LayerDatabase->LayerBase.MSDF) ? (LayerDatabase->LayerBase.Outline ? "1" : "2") : (LayerDatabase->LayerBase.UseColor ? (LayerDatabase->LayerBase.Outline ? "1" : "2") : "0") ;
	FString OpacitySDF = !LayerDatabase->LayerBase.MSDF ? "1" : (LayerDatabase->LayerBase.SDF_Alpha ? "1" : "0") ;//OpacitySDFName
	FString SDFOutline = !LayerDatabase->LayerBase.MSDF ? (!LayerDatabase->LayerBase.Outline ? "2" : "3") : (LayerDatabase->LayerBase.Outline ? "1" : "0");//SdfOutlineName
	FString ColorBooleans = "";//ColorBooleansName
	FString OpacityBooleans = "";// OpacityBooleansName
	if (LayerDatabase->LayerBase.Color.Database != NULL)
	{
		ColorBooleans = LayerDatabase->LayerBase.Color.Database->layer_bools.ToString();
	}
	if (LayerDatabase->LayerBase.Alpha.Database != NULL)
	{
		OpacityBooleans = LayerDatabase->LayerBase.Alpha.Database->layer_bools.ToString();
	}
	//-Texture Part
	FString ColorTex = "";//ColorTexName
	if (LayerDatabase->LayerBase.Color.Database != NULL)
	{
		FTileDatabase layer_interior = FTileDatabase();
		if (LayerDatabase->LayerBase.Color.Database->TileMap.Contains(LayerDatabase->LayerBase.Color.Name))
		{
			layer_interior.SetFromPointer(LayerDatabase->LayerBase.Color.Database->TileMap.Find(LayerDatabase->LayerBase.Color.Name));
			ColorTex = LayerDatabase->LayerBase.Color.Database->TextureOutput[layer_interior.Frame]->GetPathName(NULL);
		}
	}
	FString OpacityTex = "";//OpacityTexName
	if (LayerDatabase->LayerBase.Alpha.Database != NULL)
	{
		FTileDatabase layer_alpha = FTileDatabase();
		if (LayerDatabase->LayerBase.Alpha.Database->TileMap.Contains(LayerDatabase->LayerBase.Alpha.Name))
		{
			layer_alpha.SetFromPointer(LayerDatabase->LayerBase.Alpha.Database->TileMap.Find(LayerDatabase->LayerBase.Alpha.Name));
			OpacityTex = LayerDatabase->LayerBase.Alpha.Database->TextureOutput[layer_alpha.Frame]->GetPathName(NULL);
		}
	}
	//-Scalar Part
	FString EdgeSoftness = FString::SanitizeFloat(LayerDatabase->LayerBase.EdgeSoftness);//EdgeSoftnessName
	FString OutlineThresold = FString::SanitizeFloat(LayerDatabase->LayerBase.OutlineThresold);//OutlineThresoldName
	FString SDFThresold = FString::SanitizeFloat(LayerDatabase->LayerBase.SDFThresold);//SDFThresoldName
	//Vectors Part
	FString ColorInterior = LayerDatabase->LayerBase.ColorInterior.ToString();//ColorInteriorName
	FString ColorOutline = LayerDatabase->LayerBase.ColorOutline.ToString();//ColorOutlineName
	FString ColorMaterialCollection = "";//ColorMCName
	FString ColorChannel = "";//ColorchannelName
	FString ColorLayer = "";//ColorLayerName
	FString ColorSizePadding = "";//ColorsizePaddingName
	if (bMaterialCollection)
	{
		if (LayerDatabase->LayerBase.Color.Database != NULL)
		{
			ColorMaterialCollection = LayerDatabase->LayerBase.Color.Database->MaterialCollection->GetPathName();
			if (ColorMaterialCollection != "None")
			{
				FString ColorName = "";
				FString ColorAppend = "";
				LayerDatabase->LayerBase.Alpha.Name.ToString(ColorName);
				ColorAppend = ColorName;
				ColorChannel = ColorAppend.Append(FString(TEXT("_channel")));
				ColorAppend = ColorName;
				ColorLayer = ColorAppend.Append(FString(TEXT("_layer")));
				ColorAppend = ColorName;
				ColorSizePadding = ColorAppend.Append(FString(TEXT("_sizePadding")));
			}
		}
	}
	else {
		FLinearColor OutValue = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		LayerDatabase->Material->GetVectorParameterValue(FName(*FString("Color_Channel")), OutValue);
		ColorChannel = OutValue.ToString();
		LayerDatabase->Material->GetVectorParameterValue(FName(*FString("Color_layer")), OutValue);
		ColorLayer = OutValue.ToString();
		LayerDatabase->Material->GetVectorParameterValue(FName(*FString("Color_SizePadding")), OutValue);
		ColorSizePadding = OutValue.ToString();
	}
	FString OpacityMaterialCollection = "";//OpacityMCName
	FString OpacityChannel = "";//OpacitychannelName
	FString OpacityLayer = "";//OpacityLayerName
	FString OpacitySizePadding = "";//OpacitysizePaddingName
	if (bMaterialCollection)
	{
		if (LayerDatabase->LayerBase.Alpha.Database != NULL)
		{
			OpacityMaterialCollection = LayerDatabase->LayerBase.Alpha.Database->MaterialCollection->GetPathName();
			if (OpacityMaterialCollection != "None")
			{
				FString AlphaName = "";
				FString AlphaAppend = "";
				LayerDatabase->LayerBase.Alpha.Name.AppendString(AlphaName);
				AlphaAppend = AlphaName;
				OpacityChannel =  AlphaAppend.Append(FString(TEXT("_channel")));
				AlphaAppend = AlphaName;
				OpacityLayer = AlphaAppend.Append(FString(TEXT("_layer")));
				AlphaAppend = AlphaName;
				OpacitySizePadding = AlphaAppend.Append(FString(TEXT("_sizePadding")));
			}
		}
	}
	else {
		FLinearColor OutValue = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		LayerDatabase->Material->GetVectorParameterValue(FName(*FString("Opacity_Channels")), OutValue);
		OpacityChannel = OutValue.ToString();
		LayerDatabase->Material->GetVectorParameterValue(FName(*FString("Opacity_Layers")), OutValue);
		OpacityLayer = OutValue.ToString();
		LayerDatabase->Material->GetVectorParameterValue(FName(*FString("Opacity_SizePadding")), OutValue);
		OpacitySizePadding = OutValue.ToString();
	}
	//Get the String nodes by button pressed
	FString Nodes = bMaterialCollection ? GetNodesMaterialCollection() : GetNodesLinearColor();
	//Replaces the Key Strings to variables
	Nodes = Nodes.Replace( *FString("ColorSDFName"), *ColorSDF);
	Nodes = Nodes.Replace( *FString("OpacitySDFName"), *OpacitySDF);
	Nodes = Nodes.Replace( *FString("SdfOutlineName"), *SDFOutline);
	Nodes = Nodes.Replace( *FString("ColorBooleansName"), *ColorBooleans);
	Nodes = Nodes.Replace( *FString("OpacityBooleansName"), *OpacityBooleans);
	Nodes = Nodes.Replace( *FString("ColorTexName"), *ColorTex);
	Nodes = Nodes.Replace( *FString("OpacityTexName"), *OpacityTex);
	Nodes = Nodes.Replace( *FString("EdgeSoftnessName"), *EdgeSoftness);
	Nodes = Nodes.Replace( *FString("OutlineThresoldName"), *OutlineThresold);
	Nodes = Nodes.Replace( *FString("SDFThresoldName"), *SDFThresold);
	Nodes = Nodes.Replace( *FString("ColorInteriorName"), *ColorInterior);
	Nodes = Nodes.Replace( *FString("ColorOutlineName"), *ColorOutline);
	Nodes = Nodes.Replace( *FString("ColorMCName"), *ColorMaterialCollection);
	Nodes = Nodes.Replace( *FString("ColorchannelName"), *ColorChannel);
	Nodes = Nodes.Replace( *FString("ColorLayerName"), *ColorLayer);
	Nodes = Nodes.Replace( *FString("ColorsizePaddingName"), *ColorSizePadding);
	Nodes = Nodes.Replace( *FString("OpacityMCName"), *OpacityMaterialCollection);
	Nodes = Nodes.Replace( *FString("OpacitychannelName"), *OpacityChannel);
	Nodes = Nodes.Replace( *FString("OpacityLayerName"), *OpacityLayer);
	Nodes = Nodes.Replace( *FString("OpacitysizePaddingName"), *OpacitySizePadding);
	//copy to clipboard
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 17)
	FPlatformMisc::ClipboardCopy(*Nodes);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 18)
	FPlatformApplicationMisc::ClipboardCopy(*Nodes);
#endif
}

FString FTileCopyCustomizationLayout::GetNodesMaterialCollection()
{
	//Mega Text, all this Text causes error c2026, workarounds are made to get a result, literal fstring limit of 16380 single-byte characters.
	FString Part1 =
		"Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_31\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionMaterialFunctionCall Name=\"MaterialExpressionMaterialFunctionCall_0\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionMaterialFunctionCall_0\"\n \
				MaterialFunction=MaterialFunction'/MultiPacker/Material/UMG/MF_Icon_Color_opacity.MF_Icon_Color_opacity'\n \
				FunctionInputs(0)=(ExpressionInputId=00C7E0904EA915E8F461F482B9837515, Input=(Expression=MaterialExpressionCollectionParameter'MaterialGraphNode_47.MaterialExpressionCollectionParameter_3', InputName=\"Color_Channel\"))\n \
				FunctionInputs(1)=(ExpressionInputId=799562E845C9FBF983F12692412E2AF6, Input=(Expression=MaterialExpressionCollectionParameter'MaterialGraphNode_48.MaterialExpressionCollectionParameter_4', InputName=\"Color_Layers\"))\n \
				FunctionInputs(2)=(ExpressionInputId=857B89854DCC02960B9E14A60FE8CD6A, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_39.MaterialExpressionVectorParameter_11', InputName=\"Color_BooleansTiled\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
				FunctionInputs(3)=(ExpressionInputId=D8A9E1D14031BD43BBCE10921A3630E6, Input=(Expression=MaterialExpressionTextureObjectParameter'MaterialGraphNode_40.MaterialExpressionTextureObjectParameter_1', InputName=\"Color_Texture\"))\n \
				FunctionInputs(4)=(ExpressionInputId=23F28D134120EB5464BE7589F6B15C51, Input=(Expression=MaterialExpressionCollectionParameter'MaterialGraphNode_49.MaterialExpressionCollectionParameter_5', InputName=\"Color_SizePadding\"))\n \
				FunctionInputs(5)=(ExpressionInputId=8CA9AC9C4AC2E6870C56EE89C6AE9F68, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_35.MaterialExpressionVectorParameter_1', InputName=\"ColorInterior\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
				FunctionInputs(6)=(ExpressionInputId=84C423E542D65F6205E78FA63DE92A42, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_36.MaterialExpressionVectorParameter_2', InputName=\"ColorOutline\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
				FunctionInputs(7)=(ExpressionInputId=A65E351A403712943F29E8AC74B0B05E, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_42.MaterialExpressionScalarParameter_6', InputName=\"Color_IsSdf?\"))\n \
				FunctionInputs(8)=(ExpressionInputId=ABE6059A4FD7B7D5FC74B680318185A7, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_41.MaterialExpressionScalarParameter_5', InputName=\"Opacity_IsSdf?\"))\n \
				FunctionInputs(9)=(ExpressionInputId=C094A5B5465F00FBF172B5BC302A8253, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_34.MaterialExpressionScalarParameter_2', InputName=\"EdgeSofness\"))\n \
				FunctionInputs(10)=(ExpressionInputId=D2F3A9B547E5F2BE955A618948F61011, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_33.MaterialExpressionScalarParameter_1', InputName=\"OutlineThresold\"))\n \
				FunctionInputs(11)=(ExpressionInputId=832098AF416FF42075FA65AF95561554, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_32.MaterialExpressionScalarParameter_0', InputName=\"SdfOuline\"))\n \
				FunctionInputs(12)=(ExpressionInputId=EE8740374AC2F17A12DCDC9D9E599B6A, Input=(Expression=MaterialExpressionCollectionParameter'MaterialGraphNode_45.MaterialExpressionCollectionParameter_1', InputName=\"Opacity_Channels\"))\n \
				FunctionInputs(13)=(ExpressionInputId=8955DBE04E74F6C563D9AB9CE45CC5EB, Input=(Expression=MaterialExpressionCollectionParameter'MaterialGraphNode_46.MaterialExpressionCollectionParameter_2', InputName=\"Opacity_Layers\"))\n \
				FunctionInputs(14)=(ExpressionInputId=E4368CFB44CB086328BCC6B3024EBF17, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_37.MaterialExpressionVectorParameter_6', InputName=\"Opacity_BooleansTiled\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
				FunctionInputs(15)=(ExpressionInputId=89BA17F74D03963A75DACC9E85FFBE9C, Input=(Expression=MaterialExpressionTextureObjectParameter'MaterialGraphNode_38.MaterialExpressionTextureObjectParameter_0', InputName=\"Opacity_Texture\"))\n \
				FunctionInputs(16)=(ExpressionInputId=EA82CF0E423A94325512378060ECB20D, Input=(Expression=MaterialExpressionCollectionParameter'MaterialGraphNode_44.MaterialExpressionCollectionParameter_0', InputName=\"Opacity_SizePadding\"))\n \
				FunctionInputs(17)=(ExpressionInputId=899C73D643F415BACD8E5895471A6812, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_43.MaterialExpressionScalarParameter_3', InputName=\"SDFThresold\"))\n \
				FunctionOutputs(0)=(ExpressionOutputId=01A88CB84DD9F9920833A2A4A8667A48, Output=(OutputName=\"Emissive\"))\n \
				FunctionOutputs(1)=(ExpressionOutputId=FCD9FFE04A73D179C5E8859C3A11A25E, Output=(OutputName=\"Opacity\"))\n \
				MaterialExpressionEditorX=-688\n \
				MaterialExpressionEditorY=240\n \
				MaterialExpressionGuid=605A858C42D9D5DF7B4249A90F6C6679\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
				Outputs(0)=(OutputName=\"Emissive\")\n \
				Outputs(1)=(OutputName=\"Opacity\")\n \
			End Object\n \
			MaterialExpression=MaterialExpressionMaterialFunctionCall'MaterialExpressionMaterialFunctionCall_0'\n \
			NodePosX=-688\n \
			NodePosY=240\n \
	";
	FString Part2 =
		"NodeGuid=F5C62025462433473AB0B7BC972223AD\n \
			CustomProperties Pin(PinId=071C01F348DFB32ECCC939854D8DDCBF, PinName=\"Color_Channel (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_47 6F1897064328628C1F9C7182BFB21D90, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=51F4832542690155A1922E90CA1605A4, PinName=\"Color_Layers (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_48 BC48AF0C40DFB5A3543D049FA57BBB14, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=5D8540BB4D550B57A62C90B55B9FBC8D, PinName=\"Color_BooleansTiled (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_39 C179013741661EEC34E96F804F232BC8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=DDE12C30449233E5FF07B589FC96FF18, PinName=\"Color_Texture (T2d)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_40 1E30FDA0407BBACC9370A28048B704F1, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=F6D3512B477B01286D85C58AA444B7AC, PinName=\"Color_SizePadding (V4)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_49 EE5E26884AD9B5E2F665D69054B9F009, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=12C959D04AB9CD3C512DE19681400BC9, PinName=\"ColorInterior (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_35 B64877E145107A15DBDB7A99BC4F0192, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=BBAC12584E50DFFD6269EBB78C35EBE0, PinName=\"ColorOutline (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_36 7AF88E3E47567AA9B5EE8EAE0F106391, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=D540AE074650973C7F3E32A0D854E624, PinName=\"Color_IsSdf? (S)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_42 B5735EE7417162DFB0BCC4A39C0771E5, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=759872C341BCC62E20EF6DB202C22531, PinName=\"Opacity_IsSdf? (S)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_41 7370799C4C2B24844E8039A2C8F015E7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=A6501CB441884E834B19A7B7DC5D5902, PinName=\"EdgeSofness (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_34 5542509B4DBFEFA7D6B537B737CABD0B, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=9EEEABE040C34CFE65700EB57BD33EF8, PinName=\"OutlineThresold (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_33 7B4F31A044DC2CBF5E176FAA50D32705, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=C60B01A349889710D53A9C8DF8995200, PinName=\"SdfOuline (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_32 9B408347461997F03EFA109AC43936C8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=ADA90EBE4908EA91A0DB61B860C4D863, PinName=\"Opacity_Channels (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_45 4EAF267C4FE7F96A95E8CB8F788A5EDD, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=5B47A6C942C5B81DB2D9028FCF3F6483, PinName=\"Opacity_Layers (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_46 2F0738BA408A77E214635BABDD796CAF, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=330F460C4873888840160EB006B48241, PinName=\"Opacity_BooleansTiled (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_37 B1E4321A489BD6850B4DCE8482F2E6A7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=250999464B0E352425C80CB9F9172A52, PinName=\"Opacity_Texture (T2d)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_38 B8AB3F614CF8D881D8C642A13421F04D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=06C0B7A84AD0E25F2860CEB455986688, PinName=\"Opacity_SizePadding (V4)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_44 F9B927DC4BE9AECA9DF205AC16BD1E2A, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=5EA0B0204DE9589D80AB9FA96DC9C3D4, PinName=\"SDFThresold (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_43 8CF2901B47FC5E464BCBCB83E0877CF0, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=9195A74F41756A42E274DEAC32B52803, PinName=\"Emissive\", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_Root_2 EAF451EB465C5E69B37D00A3012D70EA, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=A2FFB01C41DD6441CB8F8EBECC4601F5, PinName=\"Opacity\", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_32\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_0\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionScalarParameter_0\"\n \
				DefaultValue= SdfOutlineName \n \
				ParameterName=\"bSdfOutline\"\n \
				ExpressionGUID=E0A9D51B458F0AA284911BA3AC68AAEA\n \
				MaterialExpressionEditorX=-1088\n \
				MaterialExpressionEditorY=672\n \
				MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_0'\n \
			NodePosX=-1088\n \
			NodePosY=672\n \
			bCanRenameNode=True\n \
			NodeGuid=01BF2AF14301ED5D6A58859D5C200A5D\n \
			CustomProperties Pin(PinId=9B408347461997F03EFA109AC43936C8, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 C60B01A349889710D53A9C8DF8995200, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_33\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_1\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionScalarParameter_1\"\n \
				DefaultValue= OutlineThresoldName \n \
				ParameterName=\"OutlineThresold\"\n \
				ExpressionGUID=3720ED974A672C92728EFE96CC4DD33E\n \
				MaterialExpressionEditorX=-1088\n \
				MaterialExpressionEditorY=576\n \
				MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_1'\n \
			NodePosX=-1088\n \
			NodePosY=576\n \
			bCanRenameNode=True\n \
			NodeGuid=720E16C44FC1A194D8A74DA884FD7011\n \
	";
	FString Part3 =
		"CustomProperties Pin(PinId=7B4F31A044DC2CBF5E176FAA50D32705, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 9EEEABE040C34CFE65700EB57BD33EF8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_34\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_2\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionScalarParameter_2\"\n \
				DefaultValue= EdgeSoftnessName \n \
				ParameterName=\"EdgeSoftness\"\n \
				ExpressionGUID=1EA0DFD349D2E7F5C5002A8660741CFE\n \
				MaterialExpressionEditorX=-1088\n \
				MaterialExpressionEditorY=496\n \
				MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_2'\n \
			NodePosX=-1088\n \
			NodePosY=496\n \
			bCanRenameNode=True\n \
			NodeGuid=E84D0A434373832322A24CA7B8ABA4DD\n \
			CustomProperties Pin(PinId=5542509B4DBFEFA7D6B537B737CABD0B, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 A6501CB441884E834B19A7B7DC5D5902, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_35\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_1\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionVectorParameter_1\"\n \
				DefaultValue= ColorInteriorName \n \
				ParameterName=\"ColorInterior\"\n \
				ExpressionGUID=A03BBEE74518D86ECB9D32A95EDC363A\n \
				MaterialExpressionEditorX=-1584\n \
				MaterialExpressionEditorY=-64\n \
				MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_1'\n \
			NodePosX=-1584\n \
			NodePosY=-64\n \
			bCanRenameNode=True\n \
			NodeGuid=E13FEEA5402FF737EF5B9487E345C116\n \
			CustomProperties Pin(PinId=B64877E145107A15DBDB7A99BC4F0192, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 12C959D04AB9CD3C512DE19681400BC9, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=7AE32E41492E51E03E7650B451939571, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=EC858BA9420F4EB68B36B0A871B809E4, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=EC86574E48A446E02A00F89797A156B3, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=B7E68BB64F94723E14831FAC446295D4, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_36\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_2\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionVectorParameter_2\"\n \
				DefaultValue= ColorOutlineName \n \
				ParameterName=\"ColorOutline\"\n \
				ExpressionGUID=3A71F4CE4933CBD2CA1F258F71C6AC89\n \
				MaterialExpressionEditorX=-1584\n \
				MaterialExpressionEditorY=128\n \
				MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_2'\n \
			NodePosX=-1584\n \
			NodePosY=128\n \
			bCanRenameNode=True\n \
			NodeGuid=573B63714A33145E1E0863B1D3F2D028\n \
			CustomProperties Pin(PinId=7AF88E3E47567AA9B5EE8EAE0F106391, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 BBAC12584E50DFFD6269EBB78C35EBE0, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=325C91034C3DB3FEB8F8CCA368788084, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=72D3127D4746735C81C8DEAC1F20F5EE, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=28AF10F8403DD1312FFD3EA60E4D18F9, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=E91A8D094BECCD595524369A0A361470, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_37\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_6\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionVectorParameter_6\"\n \
				DefaultValue=OpacityBooleansName \n \
				ParameterName=\"Opacity_BooleansTiled\"\n \
				ExpressionGUID=D50A248743ED79744B4F20999B4C3AFB\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=800\n \
				MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_6'\n \
			NodePosX=-1552\n \
			NodePosY=800\n \
			bCanRenameNode=True\n \
			NodeGuid=8C4344A14942F431C37A71A31E956BE9\n \
			CustomProperties Pin(PinId=B1E4321A489BD6850B4DCE8482F2E6A7, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 330F460C4873888840160EB006B48241, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=D649206D47C56B8789EFC58CD03B0404, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=0605567F40FA188F49C3B7B1A5B51F14, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=6FEE73814AA6CD2BE01DD0B876D1D4AB, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=04749F32454A13E31F6EB5B05A1B4AAD, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_38\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionTextureObjectParameter Name=\"MaterialExpressionTextureObjectParameter_0\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionTextureObjectParameter_0\"\n \
				ParameterName=\"Opacity_Tex\"\n \
				ExpressionGUID=A4C736644742F8562133A5BCFC7362A7\n \
				Texture=Texture2D'OpacityTexName'\n \
				SamplerType=SAMPLERTYPE_LinearColor\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=1008\n \
				MaterialExpressionGuid=B5ADCD2C47CF1548ECDAC788CB21F861\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
	";
	FString Part4 =
		"MaterialExpression=MaterialExpressionTextureObjectParameter'MaterialExpressionTextureObjectParameter_0'\n \
			NodePosX=-1552\n \
			NodePosY=1008\n \
			bCanRenameNode=True\n \
			NodeGuid=889520FF4B112DB58BF5C1A21200BC6E\n \
			CustomProperties Pin(PinId=B8AB3F614CF8D881D8C642A13421F04D, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 250999464B0E352425C80CB9F9172A52, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_39\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_11\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionVectorParameter_11\"\n \
				DefaultValue=ColorBooleansName \n \
				ParameterName=\"Color_BooleansTiled\"\n \
				ExpressionGUID=33A6D92043A92ABF645412B0373E33B3\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=-656\n \
				MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_11'\n \
			NodePosX=-1552\n \
			NodePosY=-656\n \
			bCanRenameNode=True\n \
			NodeGuid=CE9A67034292BD5DBC8A9EB20A7469B0\n \
			CustomProperties Pin(PinId=C179013741661EEC34E96F804F232BC8, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 5D8540BB4D550B57A62C90B55B9FBC8D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=3D5630234A9BD45FE03BB3AECA542B79, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=5D18B452435E61847A846180AACA8E4C, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=92359A39466B3088A7E68D9D1F5FD3A4, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
			CustomProperties Pin(PinId=95297F554F9B23355ED02A8558D31CCF, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_40\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionTextureObjectParameter Name=\"MaterialExpressionTextureObjectParameter_1\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionTextureObjectParameter_1\"\n \
				ParameterName=\"Color_Tex\"\n \
				ExpressionGUID=531F49D24844C58F6661158A5D0EF4AC\n \
				Texture=Texture2D'ColorTexName'\n \
				SamplerType=SAMPLERTYPE_LinearColor\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=-464\n \
				MaterialExpressionGuid=B5ADCD2C47CF1548ECDAC788CB21F861\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionTextureObjectParameter'MaterialExpressionTextureObjectParameter_1'\n \
			NodePosX=-1552\n \
			NodePosY=-464\n \
			bCanRenameNode=True\n \
			NodeGuid=8F08486147206FA9E131A499B9C10D40\n \
			CustomProperties Pin(PinId=1E30FDA0407BBACC9370A28048B704F1, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 DDE12C30449233E5FF07B589FC96FF18, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_41\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_5\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionScalarParameter_5\"\n \
				DefaultValue=OpacitySDFName \n \
				ParameterName=\"bOpacitySDF\"\n \
				ExpressionGUID=959BE9D6491ED2EBB3A5A991AF109D46\n \
				MaterialExpressionEditorX=-1088\n \
				MaterialExpressionEditorY=416\n \
				MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_5'\n \
			NodePosX=-1088\n \
			NodePosY=416\n \
			bCanRenameNode=True\n \
			NodeGuid=4A90A5D84C11CFBD149544A42655364F\n \
			CustomProperties Pin(PinId=7370799C4C2B24844E8039A2C8F015E7, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 759872C341BCC62E20EF6DB202C22531, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_42\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_6\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionScalarParameter_6\"\n \
				DefaultValue= ColorSDFName\n \
				ParameterName=\"bColorSDF\"\n \
				ExpressionGUID=D5F156934FE1F8A3B76A3886669350A2\n \
				MaterialExpressionEditorX=-1088\n \
				MaterialExpressionEditorY=336\n \
				MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_6'\n \
			NodePosX=-1088\n \
			NodePosY=336\n \
			bCanRenameNode=True\n \
			NodeGuid=EC6648A74B84FD1B11E9A79DB9D2A576\n \
			CustomProperties Pin(PinId=B5735EE7417162DFB0BCC4A39C0771E5, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 D540AE074650973C7F3E32A0D854E624, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_43\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_3\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionScalarParameter_3\"\n \
				DefaultValue= SDFThresoldName\n \
				ParameterName=\"SDFThresold\"\n \
				ExpressionGUID=B0656E824A67864FD7ECF69DDC1FE9C1\n \
				MaterialExpressionEditorX=-1088\n \
				MaterialExpressionEditorY=752\n \
				MaterialExpressionGuid=C3199C3C44772B6C093BB5BF2EAA4AE5\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_3'\n \
			NodePosX=-1088\n \
			NodePosY=752\n \
			bCanRenameNode=True\n \
			NodeGuid=AB424D654DDB9FB264BC0EBD663FFD6A\n \
			CustomProperties Pin(PinId=8CF2901B47FC5E464BCBCB83E0877CF0, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 5EA0B0204DE9589D80AB9FA96DC9C3D4, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_44\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionCollectionParameter Name=\"MaterialExpressionCollectionParameter_0\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionCollectionParameter_0\"\n \
				Collection=MaterialParameterCollection'OpacityMCName'\n \
				ParameterName=OpacitysizePaddingName\n \
				ParameterId=B6F8751E41DBBC84202CC1B00F6A0F66\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=1184\n \
				MaterialExpressionGuid=A536C9024C79736E1C0687B0915C00DD\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionCollectionParameter'MaterialExpressionCollectionParameter_0'\n \
			NodePosX=-1552\n \
			NodePosY=1184\n \
			NodeGuid=9DEB53DD48916C43314C7FAEB9D7B7CB\n \
			CustomProperties Pin(PinId=F9B927DC4BE9AECA9DF205AC16BD1E2A, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 06C0B7A84AD0E25F2860CEB455986688, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_45\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionCollectionParameter Name=\"MaterialExpressionCollectionParameter_1\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionCollectionParameter_1\"\n \
				Collection=MaterialParameterCollection'OpacityMCName'\n \
				ParameterName=OpacitychannelName\n \
				ParameterId=1D21065F4686ECAB83EA74A73FC35FB3\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=432\n \
				MaterialExpressionGuid=A536C9024C79736E1C0687B0915C00DD\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionCollectionParameter'MaterialExpressionCollectionParameter_1'\n \
			NodePosX=-1552\n \
			NodePosY=432\n \
			NodeGuid=1F5FB8824384BD288101A9BC79830BF2\n \
			CustomProperties Pin(PinId=4EAF267C4FE7F96A95E8CB8F788A5EDD, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 ADA90EBE4908EA91A0DB61B860C4D863, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_46\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionCollectionParameter Name=\"MaterialExpressionCollectionParameter_2\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionCollectionParameter_2\"\n \
				Collection=MaterialParameterCollection'OpacityMCName'\n \
				ParameterName=OpacityLayerName\n \
				ParameterId=CC0657EC4C49831000ED22BD3E45B094\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=624\n \
				MaterialExpressionGuid=A536C9024C79736E1C0687B0915C00DD\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
	";
	FString Part5 =
		"MaterialExpression=MaterialExpressionCollectionParameter'MaterialExpressionCollectionParameter_2'\n \
			NodePosX=-1552\n \
			NodePosY=624\n \
			NodeGuid=67EA74874835BD430E81659BED3928AB\n \
			CustomProperties Pin(PinId=2F0738BA408A77E214635BABDD796CAF, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 5B47A6C942C5B81DB2D9028FCF3F6483, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_47\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionCollectionParameter Name=\"MaterialExpressionCollectionParameter_3\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionCollectionParameter_3\"\n \
				Collection=MaterialParameterCollection'ColorMCName'\n \
				ParameterName=ColorchannelName\n \
				ParameterId=FE1FCCC14B6BAEAF1855809DB10E74D4\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=-1040\n \
				MaterialExpressionGuid=A536C9024C79736E1C0687B0915C00DD\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionCollectionParameter'MaterialExpressionCollectionParameter_3'\n \
			NodePosX=-1552\n \
			NodePosY=-1040\n \
			NodeGuid=93ECB00146325DE74B5EDC86FE15EE82\n \
			CustomProperties Pin(PinId=6F1897064328628C1F9C7182BFB21D90, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 071C01F348DFB32ECCC939854D8DDCBF, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_48\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionCollectionParameter Name=\"MaterialExpressionCollectionParameter_4\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionCollectionParameter_4\"\n \
				Collection=MaterialParameterCollection'ColorMCName'\n \
				ParameterName=ColorLayerName\n \
				ParameterId=2F225EAB470691DB062E54BD9B508821\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=-848\n \
				MaterialExpressionGuid=A536C9024C79736E1C0687B0915C00DD\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionCollectionParameter'MaterialExpressionCollectionParameter_4'\n \
			NodePosX=-1552\n \
			NodePosY=-848\n \
			NodeGuid=D20D050B4304816E53810BA490C6455D\n \
			CustomProperties Pin(PinId=BC48AF0C40DFB5A3543D049FA57BBB14, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 51F4832542690155A1922E90CA1605A4, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_49\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionCollectionParameter Name=\"MaterialExpressionCollectionParameter_5\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionCollectionParameter_5\"\n \
				Collection=MaterialParameterCollection'ColorMCName'\n \
				ParameterName=ColorsizePaddingName\n \
				ParameterId=DD11E96D49F04D8A328FB9B0E6E20C7E\n \
				MaterialExpressionEditorX=-1552\n \
				MaterialExpressionEditorY=-272\n \
				MaterialExpressionGuid=A536C9024C79736E1C0687B0915C00DD\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpression=MaterialExpressionCollectionParameter'MaterialExpressionCollectionParameter_5'\n \
			NodePosX=-1552\n \
			NodePosY=-272\n \
			NodeGuid=17C32E5745F23A46820A2E8C5B6E2440\n \
			CustomProperties Pin(PinId=EE5E26884AD9B5E2F665D69054B9F009, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 F6D3512B477B01286D85C58AA444B7AC, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode_Comment Name=\"MaterialGraphNode_Comment_2\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionComment Name=\"MaterialExpressionComment_11\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionComment_11\"\n \
				SizeX=400\n \
				SizeY=1024\n \
				Text=\"color\"\n \
				MaterialExpressionEditorX=-1632\n \
				MaterialExpressionEditorY=-1104\n \
				MaterialExpressionGuid=5E6561AC4262F454CE1FEFA7EB75E8ED\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpressionComment=MaterialExpressionComment'MaterialExpressionComment_11'\n \
			NodePosX=-1632\n \
			NodePosY=-1104\n \
			NodeHeight=1024\n \
			NodeComment=\"color\"\n \
			NodeGuid=329829C6495ED2566538E6A57B8A116F\n \
		End Object\n \
		Begin Object Class=/Script/UnrealEd.MaterialGraphNode_Comment Name=\"MaterialGraphNode_Comment_3\"\n \
			Begin Object Class=/Script/Engine.MaterialExpressionComment Name=\"MaterialExpressionComment_12\"\n \
			End Object\n \
			Begin Object Name=\"MaterialExpressionComment_12\"\n \
				SizeX=400\n \
				SizeY=1024\n \
				Text=\"opacity\"\n \
				MaterialExpressionEditorX=-1616\n \
				MaterialExpressionEditorY=368\n \
				MaterialExpressionGuid=CE2D966E49F28E8C0E430EAF0DB05F2F\n \
				Material=PreviewMaterial'/Engine/Transient.NewMaterial'\n \
			End Object\n \
			MaterialExpressionComment=MaterialExpressionComment'MaterialExpressionComment_12'\n \
			NodePosX=-1616\n \
			NodePosY=368\n \
			NodeHeight=1024\n \
			NodeComment=\"opacity\"\n \
			NodeGuid=961C851849400BA248CC1791CE719CE7\n \
		End Object\n \
	";
	OutputString = Part1.Append(Part2.Append(Part3.Append(Part4.Append(Part5))));
	return OutputString;
}

FString FTileCopyCustomizationLayout::GetNodesLinearColor()
{
	FString Part1 =
	"Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_19\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionMaterialFunctionCall Name=\"MaterialExpressionMaterialFunctionCall_0\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionMaterialFunctionCall_0\"\n \
			MaterialFunction=MaterialFunction'/MultiPacker/Material/UMG/MF_Icon_Color_opacity.MF_Icon_Color_opacity'\n \
			FunctionInputs(0)=(ExpressionInputId=00C7E0904EA915E8F461F482B9837515, Input=(Expression=MaterialExpressionAppendVector'MaterialGraphNode_36.MaterialExpressionAppendVector_6', InputName=\"Color_Channel\"))\n \
			FunctionInputs(1)=(ExpressionInputId=799562E845C9FBF983F12692412E2AF6, Input=(Expression=MaterialExpressionAppendVector'MaterialGraphNode_34.MaterialExpressionAppendVector_5', InputName=\"Color_Layers\"))\n \
			FunctionInputs(2)=(ExpressionInputId=857B89854DCC02960B9E14A60FE8CD6A, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_39.MaterialExpressionVectorParameter_11', InputName=\"Color_BooleansTiled\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
			FunctionInputs(3)=(ExpressionInputId=D8A9E1D14031BD43BBCE10921A3630E6, Input=(Expression=MaterialExpressionTextureObjectParameter'MaterialGraphNode_40.MaterialExpressionTextureObjectParameter_1', InputName=\"Color_Texture\"))\n \
			FunctionInputs(4)=(ExpressionInputId=23F28D134120EB5464BE7589F6B15C51, Input=(Expression=MaterialExpressionAppendVector'MaterialGraphNode_38.MaterialExpressionAppendVector_7', InputName=\"Color_SizePadding\"))\n \
			FunctionInputs(5)=(ExpressionInputId=8CA9AC9C4AC2E6870C56EE89C6AE9F68, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_23.MaterialExpressionVectorParameter_1', InputName=\"ColorInterior\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
			FunctionInputs(6)=(ExpressionInputId=84C423E542D65F6205E78FA63DE92A42, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_24.MaterialExpressionVectorParameter_2', InputName=\"ColorOutline\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
			FunctionInputs(7)=(ExpressionInputId=A65E351A403712943F29E8AC74B0B05E, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_42.MaterialExpressionScalarParameter_6', InputName=\"Color_IsSdf?\"))\n \
			FunctionInputs(8)=(ExpressionInputId=ABE6059A4FD7B7D5FC74B680318185A7, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_41.MaterialExpressionScalarParameter_5', InputName=\"Opacity_IsSdf?\"))\n \
			FunctionInputs(9)=(ExpressionInputId=C094A5B5465F00FBF172B5BC302A8253, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_22.MaterialExpressionScalarParameter_2', InputName=\"EdgeSofness\"))\n \
			FunctionInputs(10)=(ExpressionInputId=D2F3A9B547E5F2BE955A618948F61011, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_21.MaterialExpressionScalarParameter_1', InputName=\"OutlineThresold\"))\n \
			FunctionInputs(11)=(ExpressionInputId=832098AF416FF42075FA65AF95561554, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_20.MaterialExpressionScalarParameter_0', InputName=\"SdfOuline\"))\n \
			FunctionInputs(12)=(ExpressionInputId=EE8740374AC2F17A12DCDC9D9E599B6A, Input=(Expression=MaterialExpressionAppendVector'MaterialGraphNode_28.MaterialExpressionAppendVector_2', InputName=\"Opacity_Channels\"))\n \
			FunctionInputs(13)=(ExpressionInputId=8955DBE04E74F6C563D9AB9CE45CC5EB, Input=(Expression=MaterialExpressionAppendVector'MaterialGraphNode_26.MaterialExpressionAppendVector_1', InputName=\"Opacity_Layers\"))\n \
			FunctionInputs(14)=(ExpressionInputId=E4368CFB44CB086328BCC6B3024EBF17, Input=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_31.MaterialExpressionVectorParameter_6', InputName=\"Opacity_BooleansTiled\", Mask=1, MaskR=1, MaskG=1, MaskB=1))\n \
			FunctionInputs(15)=(ExpressionInputId=89BA17F74D03963A75DACC9E85FFBE9C, Input=(Expression=MaterialExpressionTextureObjectParameter'MaterialGraphNode_32.MaterialExpressionTextureObjectParameter_0', InputName=\"Opacity_Texture\"))\n \
			FunctionInputs(16)=(ExpressionInputId=EA82CF0E423A94325512378060ECB20D, Input=(Expression=MaterialExpressionAppendVector'MaterialGraphNode_30.MaterialExpressionAppendVector_3', InputName=\"Opacity_SizePadding\"))\n \
			FunctionInputs(17)=(ExpressionInputId=899C73D643F415BACD8E5895471A6812, Input=(Expression=MaterialExpressionScalarParameter'MaterialGraphNode_43.MaterialExpressionScalarParameter_3', InputName=\"SDFThresold\"))\n \
			FunctionOutputs(0)=(ExpressionOutputId=01A88CB84DD9F9920833A2A4A8667A48, Output=(OutputName=\"Emissive\"))\n \
			FunctionOutputs(1)=(ExpressionOutputId=FCD9FFE04A73D179C5E8859C3A11A25E, Output=(OutputName=\"Opacity\"))\n \
			MaterialExpressionEditorX=112\n \
			MaterialExpressionEditorY=-704\n \
			MaterialExpressionGuid=605A858C42D9D5DF7B4249A90F6C6679\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
			Outputs(0)=(OutputName=\"Emissive\")\n \
			Outputs(1)=(OutputName=\"Opacity\")\n \
		End Object\n \
		MaterialExpression=MaterialExpressionMaterialFunctionCall'MaterialExpressionMaterialFunctionCall_0'\n \
		NodePosX=112\n \
		NodePosY=-704\n \
		NodeGuid=457620E74ACCD0FFC6BCC0B280278B0D\n \
		CustomProperties Pin(PinId=6B761EB8410F9915A8E09E9A8FBAC417, PinName=\"Color_Channel (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_36 E0FA06614F36077A0148DAB80C026D56, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=D564D8A849E14898FE2FC59CDBA795A2, PinName=\"Color_Layers (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_34 70C2A1D94B2AF98F587599943AB868D9, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=CF79979D4DA0285A28D6FCA7D04E176E, PinName=\"Color_BooleansTiled (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_39 D002DADC462951762C0CFFBE915582C7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=E6C2AD7E4EF89F219FB9D2A1922E3A58, PinName=\"Color_Texture (T2d)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_40 32540BF749EBD6E4F46085B0B17BDFD7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=FB4B7A94436DF18541DF0EA4A1A5D6B0, PinName=\"Color_SizePadding (V4)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_38 8B86EA954655AD25C9836E87D327490A, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=5F094F754F6A40103CE1138A7D7968B9, PinName=\"ColorInterior (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_23 E3613C15401F0C1208C61AB2C09C0256, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=C03726D74102FF0672C3408A6FDF2F12, PinName=\"ColorOutline (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_24 F49926CD4DC631C4634440BB1695677B, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=32DD88A945F271528F9E7B85EE544786, PinName=\"Color_IsSdf? (S)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_42 C1F6BB01492AD497AA16D4A286946344, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=EC66A232432C44264165F39ADD38F5E8, PinName=\"Opacity_IsSdf? (S)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_41 92C1A2D4410BAB82D3EB4B851B94B282, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=2EB828BC415EEFFF365061B236C5D4DD, PinName=\"EdgeSofness (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_22 BB5E66F442CBE979F52EFAA552ADD9E7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=747EC2C54D57BB826CA46D8A72551F9F, PinName=\"OutlineThresold (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_21 49D179BA44F913482DE8C0BCB9489E8D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6C66EEA541A72576CAD0DDA67447643B, PinName=\"SdfOuline (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_20 F535A57C4D2E1D54370D0A8AA70104C8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=19390069498C122FDB0DD89486F58C15, PinName=\"Opacity_Channels (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_28 4443058B4D3C306C5AF621B98FBA2B0F, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=761C3999440506ECBAB1C1A3A45569B8, PinName=\"Opacity_Layers (V4)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_26 372A273B4019443895DDFE9E637A4789, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=31C6E37B40A0AB3B0AFB44AF5C5EC582, PinName=\"Opacity_BooleansTiled (V3)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_31 A47C3EBD4C88AB5C8E886BBD3EBA781D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=25BDE60E4059BA80DAAC52A00ABE1636, PinName=\"Opacity_Texture (T2d)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_32 421FA47C4C5E87898D7FD289DE87DA73, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=49883F14446AE6DD9A16708982FC8873, PinName=\"Opacity_SizePadding (V4)\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_30 130B9DA142BA97F9AB568E9DAD291346, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	";
	FString Part2 =
		"CustomProperties Pin(PinId=76FA4F8043A12C8589FB80805850F4F7, PinName=\"SDFThresold (S)\", PinType.PinCategory=\"optional\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_43 878EBCB749C1AD81A26F7D95D5DC8B21, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=55F302C84916AD790711ED8485D97658, PinName=\"Emissive\", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_Root_1 F156CCCF44142937FD2AA29358BA3FF6, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=5558C4294250C08CD8CF7B8569F09360, PinName=\"Opacity\", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_Root_1 B9AB8C6A4DB9037A26D0ECBF02B31D46, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_20\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_0\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionScalarParameter_0\"\n \
			DefaultValue= SdfOutlineName \n \
			ParameterName=\"bSdfOutline\"\n \
			ExpressionGUID=B9EDAE96433A6D795690DEB72CC97B49\n \
			MaterialExpressionEditorX=-288\n \
			MaterialExpressionEditorY=-272\n \
			MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_0'\n \
		NodePosX=-288\n \
		NodePosY=-272\n \
		bCanRenameNode=True\n \
		NodeGuid=364DB01343905CDE6C35F4B251A2686A\n \
		CustomProperties Pin(PinId=F535A57C4D2E1D54370D0A8AA70104C8, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 6C66EEA541A72576CAD0DDA67447643B, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_21\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_1\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionScalarParameter_1\"\n \
			DefaultValue= OutlineThresoldName \n \
			ParameterName=\"OutlineThresold\"\n \
			ExpressionGUID=07F82B714BBDF5726995FDA2D3D73010\n \
			MaterialExpressionEditorX=-288\n \
			MaterialExpressionEditorY=-368\n \
			MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_1'\n \
		NodePosX=-288\n \
		NodePosY=-368\n \
		bCanRenameNode=True\n \
		NodeGuid=0EB7DCD445A1908D176FC2B84F711DD0\n \
		CustomProperties Pin(PinId=49D179BA44F913482DE8C0BCB9489E8D, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 747EC2C54D57BB826CA46D8A72551F9F, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_22\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_2\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionScalarParameter_2\"\n \
			DefaultValue= EdgeSoftnessName \n \
			ParameterName=\"EdgeSoftness\"\n \
			ExpressionGUID=EFCE335F4817906C50762CB9153A791E\n \
			MaterialExpressionEditorX=-288\n \
			MaterialExpressionEditorY=-448\n \
			MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_2'\n \
		NodePosX=-288\n \
		NodePosY=-448\n \
		bCanRenameNode=True\n \
		NodeGuid=15A857D94B44572B241D48B35D4550E8\n \
		CustomProperties Pin(PinId=BB5E66F442CBE979F52EFAA552ADD9E7, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 2EB828BC415EEFFF365061B236C5D4DD, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_23\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_1\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_1\"\n \
			DefaultValue= ColorInteriorName \n \
			ParameterName=\"ColorInterior\"\n \
			ExpressionGUID=DBF135C3472823137D8E7FBEE0B00131\n \
			MaterialExpressionEditorX=-784\n \
			MaterialExpressionEditorY=-1008\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_1'\n \
		NodePosX=-784\n \
		NodePosY=-1008\n \
		bCanRenameNode=True\n \
		NodeGuid=DF464BEC475F717D9A2443BE0F15270B\n \
		CustomProperties Pin(PinId=E3613C15401F0C1208C61AB2C09C0256, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 5F094F754F6A40103CE1138A7D7968B9, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=55013D6E43AC826CA768FA8EBA15CEAF, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=C56CBEA64650437293434BAEA887349D, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=40F81AA44B1ED10A984CE9813355B6B5, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=C52FF26348F375C0F34DA8A5CC6F0EBB, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_24\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_2\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_2\"\n \
			DefaultValue= ColorOutlineName \n \
			ParameterName=\"ColorOutline\"\n \
			ExpressionGUID=417AE7644447C7A5DA73F19B54F35C21\n \
			MaterialExpressionEditorX=-784\n \
			MaterialExpressionEditorY=-816\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_2'\n \
		NodePosX=-784\n \
		NodePosY=-816\n \
		bCanRenameNode=True\n \
		NodeGuid=DDE45BA749344C58939F3E8ABFDC0812\n \
		CustomProperties Pin(PinId=F49926CD4DC631C4634440BB1695677B, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 C03726D74102FF0672C3408A6FDF2F12, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=3502D9A14F0C0199CF82FD9ABEF3CFA9, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=5FA1FCAB4DB01663CE9F01B5A4F6D180, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6C03EABC4E3FD76D8CC05695DB9E38A6, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6EEB0822462D9317FC11E48FAB7532CC, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	";
	FString Part3 =
	"Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_25\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_3\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_3\"\n \
			DefaultValue=OpacityLayerName\n \
			ParameterName=\"Opacity_Layers\"\n \
			ExpressionGUID=6930D985477BAFBD12F80287C9B1A436\n \
			MaterialExpressionEditorX=-769\n \
			MaterialExpressionEditorY=-335\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_3'\n \
		NodePosX=-769\n \
		NodePosY=-335\n \
		bCanRenameNode=True\n \
		NodeGuid=E46D21504D99978FF76B93BB31A7A6F4\n \
		CustomProperties Pin(PinId=3A5906A943E11CF56D6093AF7369097D, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_26 94277BAE4F501EF79ADDBAA961B2557D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=315C25CD4A4A881CDC42049C7E1A5A75, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=A8A7BE874AA45A5E00DFB7AAFB19279D, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=02CEFDB341B12ADC8F748F91050688CF, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=9D4C947649CCA0F15EBE0CBF5DCEF9D7, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_26 2E35EBD3439ADFBFB79EF8A1FDD453E3, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_26\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionAppendVector Name=\"MaterialExpressionAppendVector_1\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionAppendVector_1\"\n \
			A=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_25.MaterialExpressionVectorParameter_3', Mask=1, MaskR=1, MaskG=1, MaskB=1)\n \
			B=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_25.MaterialExpressionVectorParameter_3', OutputIndex=4, Mask=1, MaskA=1)\n \
			MaterialExpressionEditorX=-577\n \
			MaterialExpressionEditorY=-255\n \
			MaterialExpressionGuid=1E72FFA248477AB1583D78A3ADFA4E68\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionAppendVector'MaterialExpressionAppendVector_1'\n \
		NodePosX=-577\n \
		NodePosY=-255\n \
		NodeGuid=61141D2548EF32BBE899FA860A33AFAD\n \
		CustomProperties Pin(PinId=94277BAE4F501EF79ADDBAA961B2557D, PinName=\"A\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_25 3A5906A943E11CF56D6093AF7369097D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=2E35EBD3439ADFBFB79EF8A1FDD453E3, PinName=\"B\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_25 9D4C947649CCA0F15EBE0CBF5DCEF9D7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=372A273B4019443895DDFE9E637A4789, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 761C3999440506ECBAB1C1A3A45569B8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_27\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_4\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_4\"\n \
			DefaultValue=OpacitychannelName\n \
			ParameterName=\"Opacity_Channels\"\n \
			ExpressionGUID=907998564D2F3AF45117E29204C18260\n \
			MaterialExpressionEditorX=-769\n \
			MaterialExpressionEditorY=-527\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_4'\n \
		NodePosX=-769\n \
		NodePosY=-527\n \
		bCanRenameNode=True\n \
		NodeGuid=1159CCD84E095C73AD9AF0ACF7B5D5B3\n \
		CustomProperties Pin(PinId=E0FD06254329A7F981F263AC893745F6, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_28 A96956C04A3582D86CB7528A60F16243, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=5E24E0C64AED6B277E060A948065A428, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=DE8DD7FE4B0CE9BC9E5FF386F339E784, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=919C96074A4423E98B70179107D7FA9E, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=65A1041C47AE645D88AB6C9089B1AE1C, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_28 8FF4B95A41F5FCE0620223833EEB2821, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_28\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionAppendVector Name=\"MaterialExpressionAppendVector_2\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionAppendVector_2\"\n \
			A=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_27.MaterialExpressionVectorParameter_4', Mask=1, MaskR=1, MaskG=1, MaskB=1)\n \
			B=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_27.MaterialExpressionVectorParameter_4', OutputIndex=4, Mask=1, MaskA=1)\n \
			MaterialExpressionEditorX=-577\n \
			MaterialExpressionEditorY=-447\n \
			MaterialExpressionGuid=1E72FFA248477AB1583D78A3ADFA4E68\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionAppendVector'MaterialExpressionAppendVector_2'\n \
		NodePosX=-577\n \
		NodePosY=-447\n \
		NodeGuid=C34F94F144FD97755A79838509D52F69\n \
		CustomProperties Pin(PinId=A96956C04A3582D86CB7528A60F16243, PinName=\"A\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_27 E0FD06254329A7F981F263AC893745F6, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=8FF4B95A41F5FCE0620223833EEB2821, PinName=\"B\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_27 65A1041C47AE645D88AB6C9089B1AE1C, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=4443058B4D3C306C5AF621B98FBA2B0F, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 19390069498C122FDB0DD89486F58C15, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_29\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_5\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_5\"\n \
			DefaultValue=OpacitysizePaddingName\n \
			ParameterName=\"Opacity_SizePadding\"\n \
			ExpressionGUID=6B5957484E46A8F07D5C3780FA7C2F5D\n \
			MaterialExpressionEditorX=-769\n \
			MaterialExpressionEditorY=225\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_5'\n \
		NodePosX=-769\n \
		NodePosY=225\n \
		bCanRenameNode=True\n \
		NodeGuid=09CB78E74B12EA00C49B99B6521BC5EA\n \
	";
	FString Part4 =
		"CustomProperties Pin(PinId=F3A99EFF41120F95A3E2EABB390F45D4, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_30 F97E8DA14CF513C61C51A38C28705D14, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=916EC7834AF4007E062860A2AB23F54D, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=296D742640ED0BF74C74A1B83E1341FD, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=E54F6F6140380429F39E5784550A7241, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6E2D2E88447C0C07B3B98AABB19182F8, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_30 15B066664463CED7B5631582EBF18780, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_30\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionAppendVector Name=\"MaterialExpressionAppendVector_3\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionAppendVector_3\"\n \
			A=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_29.MaterialExpressionVectorParameter_5', Mask=1, MaskR=1, MaskG=1, MaskB=1)\n \
			B=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_29.MaterialExpressionVectorParameter_5', OutputIndex=4, Mask=1, MaskA=1)\n \
			MaterialExpressionEditorX=-577\n \
			MaterialExpressionEditorY=305\n \
			MaterialExpressionGuid=1E72FFA248477AB1583D78A3ADFA4E68\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionAppendVector'MaterialExpressionAppendVector_3'\n \
		NodePosX=-577\n \
		NodePosY=305\n \
		NodeGuid=2B99D8E14FBEF03E4F7CA1AF3EE7ACC3\n \
		CustomProperties Pin(PinId=F97E8DA14CF513C61C51A38C28705D14, PinName=\"A\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_29 F3A99EFF41120F95A3E2EABB390F45D4, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=15B066664463CED7B5631582EBF18780, PinName=\"B\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_29 6E2D2E88447C0C07B3B98AABB19182F8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=130B9DA142BA97F9AB568E9DAD291346, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 49883F14446AE6DD9A16708982FC8873, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_31\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_6\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_6\"\n \
			DefaultValue= OpacityBooleansName \n \
			ParameterName=\"Opacity_BooleansTiled\"\n \
			ExpressionGUID=22C356C5405BAE9016D25D99F1440B75\n \
			MaterialExpressionEditorX=-769\n \
			MaterialExpressionEditorY=-143\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_6'\n \
		NodePosX=-769\n \
		NodePosY=-143\n \
		bCanRenameNode=True\n \
		NodeGuid=91576F474842FF7284D6988020406551\n \
		CustomProperties Pin(PinId=A47C3EBD4C88AB5C8E886BBD3EBA781D, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 31C6E37B40A0AB3B0AFB44AF5C5EC582, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=EC5767AC41A428F2950861B74A8E352F, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=E2D0AA96429C09F754B2418982C6D6E8, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=9FEF260A47FD4155E0FDA3B9AFDAC0DB, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=C2130A2A4214DA6DCC9091BCDAB183B6, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_32\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionTextureObjectParameter Name=\"MaterialExpressionTextureObjectParameter_0\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionTextureObjectParameter_0\"\n \
			ParameterName=\"Opacity_Tex\"\n \
			ExpressionGUID=C70B045843B92D501427E3AE3F541261\n \
			Texture=Texture2D'OpacityTexName'\n \
			SamplerType=SAMPLERTYPE_LinearColor\n \
			MaterialExpressionEditorX=-769\n \
			MaterialExpressionEditorY=49\n \
			MaterialExpressionGuid=B5ADCD2C47CF1548ECDAC788CB21F861\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionTextureObjectParameter'MaterialExpressionTextureObjectParameter_0'\n \
		NodePosX=-769\n \
		NodePosY=49\n \
		bCanRenameNode=True\n \
		NodeGuid=19D8DA0440B63708815D00AF9B9C9AB3\n \
		CustomProperties Pin(PinId=421FA47C4C5E87898D7FD289DE87DA73, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 25BDE60E4059BA80DAAC52A00ABE1636, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_33\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_8\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_8\"\n \
			DefaultValue=ColorLayerName\n \
			ParameterName=\"Color_layer\"\n \
			ExpressionGUID=932FE62C4F657E21595BC5B09828E4AE\n \
			MaterialExpressionEditorX=-752\n \
			MaterialExpressionEditorY=-1793\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_8'\n \
		NodePosX=-752\n \
		NodePosY=-1793\n \
		bCanRenameNode=True\n \
		NodeGuid=66C885544334E50CBC3C6E889A56A738\n \
		CustomProperties Pin(PinId=7CEF3492454E6DDC522C0E8CBF5BE6A2, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_34 C3418E194D4ADD42E5F78E936B1731F1, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6E6464B1409F9DAB21EEC8A92B58C479, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=B43B01A440EA840FD62C359F33572582, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=CDD274BD445A0FC83CA1FAB65B3BF98C, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=0030FF8143B07CD3550601A182DAAF99, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_34 6D14C4264CFED0B25C72C2A6D61CEFBB, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	";
	FString Part5 =
	"Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_34\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionAppendVector Name=\"MaterialExpressionAppendVector_5\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionAppendVector_5\"\n \
			A=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_33.MaterialExpressionVectorParameter_8', Mask=1, MaskR=1, MaskG=1, MaskB=1)\n \
			B=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_33.MaterialExpressionVectorParameter_8', OutputIndex=4, Mask=1, MaskA=1)\n \
			MaterialExpressionEditorX=-560\n \
			MaterialExpressionEditorY=-1713\n \
			MaterialExpressionGuid=1E72FFA248477AB1583D78A3ADFA4E68\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionAppendVector'MaterialExpressionAppendVector_5'\n \
		NodePosX=-560\n \
		NodePosY=-1713\n \
		NodeGuid=F37EAA874693F05D5BB6CCA3B07F7879\n \
		CustomProperties Pin(PinId=C3418E194D4ADD42E5F78E936B1731F1, PinName=\"A\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_33 7CEF3492454E6DDC522C0E8CBF5BE6A2, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6D14C4264CFED0B25C72C2A6D61CEFBB, PinName=\"B\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_33 0030FF8143B07CD3550601A182DAAF99, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=70C2A1D94B2AF98F587599943AB868D9, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 D564D8A849E14898FE2FC59CDBA795A2, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_35\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_9\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_9\"\n \
			DefaultValue=ColorchannelName\n \
			ParameterName=\"Color_Channel\"\n \
			ExpressionGUID=6179CDED445FC4359D34469B49B4CDD1\n \
			MaterialExpressionEditorX=-752\n \
			MaterialExpressionEditorY=-1985\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_9'\n \
		NodePosX=-752\n \
		NodePosY=-1985\n \
		bCanRenameNode=True\n \
		NodeGuid=3CDF26A141E02D5ADAC1589CC84C7E58\n \
		CustomProperties Pin(PinId=5798ED734B11CCE4D4287A84C515695C, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_36 2849EF4C4066EC74FF1375B8CADE617E, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=E256B3F842A61632E6F8C1A00B64A570, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=2CA4259B4C48869D7A4298A6B9F39BF4, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6461A0FC4213C17FBF71659E866CBBBA, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=5C23598048B53843E0973F96FA9C2A1E, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_36 19ADD1CB448DC60FDCD80396F0E413B4, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_36\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionAppendVector Name=\"MaterialExpressionAppendVector_6\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionAppendVector_6\"\n \
			A=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_35.MaterialExpressionVectorParameter_9', Mask=1, MaskR=1, MaskG=1, MaskB=1)\n \
			B=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_35.MaterialExpressionVectorParameter_9', OutputIndex=4, Mask=1, MaskA=1)\n \
			MaterialExpressionEditorX=-560\n \
			MaterialExpressionEditorY=-1905\n \
			MaterialExpressionGuid=1E72FFA248477AB1583D78A3ADFA4E68\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionAppendVector'MaterialExpressionAppendVector_6'\n \
		NodePosX=-560\n \
		NodePosY=-1905\n \
		NodeGuid=9FB4E8AE43FB30D008AE8DB086E4F5EB\n \
		CustomProperties Pin(PinId=2849EF4C4066EC74FF1375B8CADE617E, PinName=\"A\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_35 5798ED734B11CCE4D4287A84C515695C, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=19ADD1CB448DC60FDCD80396F0E413B4, PinName=\"B\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_35 5C23598048B53843E0973F96FA9C2A1E, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=E0FA06614F36077A0148DAB80C026D56, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 6B761EB8410F9915A8E09E9A8FBAC417, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_37\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_10\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_10\"\n \
			DefaultValue=ColorsizePaddingName\n \
			ParameterName=\"Color_Sizepadding\"\n \
			ExpressionGUID=D94FD5434AFFB01CDFE800A51FB821A0\n \
			MaterialExpressionEditorX=-752\n \
			MaterialExpressionEditorY=-1233\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_10'\n \
		NodePosX=-752\n \
		NodePosY=-1233\n \
		bCanRenameNode=True\n \
		NodeGuid=9D4714E74932131FB04F05A2B8BE6A28\n \
		CustomProperties Pin(PinId=1883D6014D171D3F9290AA8429B5516D, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_38 233D96A046BDE6A3B5E82A9D2659CA32, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=2B982C73472CA257961716AE8FB4F45D, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=B09CD6CD46FB43B051F261B523B1FC23, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=65C422BD4ED80B04BC648CB626439636, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=6120DDD74C9CA6754A3CD3AEE04D8A96, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_38 8A4464C04B009D175DD93AB73F51543D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_38\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionAppendVector Name=\"MaterialExpressionAppendVector_7\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionAppendVector_7\"\n \
			A=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_37.MaterialExpressionVectorParameter_10', Mask=1, MaskR=1, MaskG=1, MaskB=1)\n \
			B=(Expression=MaterialExpressionVectorParameter'MaterialGraphNode_37.MaterialExpressionVectorParameter_10', OutputIndex=4, Mask=1, MaskA=1)\n \
			MaterialExpressionEditorX=-560\n \
			MaterialExpressionEditorY=-1153\n \
			MaterialExpressionGuid=1E72FFA248477AB1583D78A3ADFA4E68\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionAppendVector'MaterialExpressionAppendVector_7'\n \
		NodePosX=-560\n \
		NodePosY=-1153\n \
		NodeGuid=DFCC1B48415CA6F3C3C1739F3C034251\n \
	";
	FString Part6 =
		"CustomProperties Pin(PinId=233D96A046BDE6A3B5E82A9D2659CA32, PinName=\"A\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_37 1883D6014D171D3F9290AA8429B5516D, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=8A4464C04B009D175DD93AB73F51543D, PinName=\"B\", PinType.PinCategory=\"required\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_37 6120DDD74C9CA6754A3CD3AEE04D8A96, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=8B86EA954655AD25C9836E87D327490A, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 FB4B7A94436DF18541DF0EA4A1A5D6B0, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_39\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionVectorParameter Name=\"MaterialExpressionVectorParameter_11\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionVectorParameter_11\"\n \
			DefaultValue= ColorBooleansName \n \
			ParameterName=\"Color_BooleansTiled\"\n \
			ExpressionGUID=82D6B641403B2068B323B588390BCA75\n \
			MaterialExpressionEditorX=-752\n \
			MaterialExpressionEditorY=-1601\n \
			MaterialExpressionGuid=10F09E564A8BFA6831EE06846568EB47\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionVectorParameter'MaterialExpressionVectorParameter_11'\n \
		NodePosX=-752\n \
		NodePosY=-1601\n \
		bCanRenameNode=True\n \
		NodeGuid=D7B8C88449DFFDFE1FDB38A55CF1E713\n \
		CustomProperties Pin(PinId=D002DADC462951762C0CFFBE915582C7, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 CF79979D4DA0285A28D6FCA7D04E176E, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=40412B9E4D9AE774E53F77A8B2A2138D, PinName=\"Output2\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"red\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=3ECF67554F68EC8ED800FC93376F0216, PinName=\"Output3\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"green\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=DEDFB03F4AB22710AC6EC99E0ABFF86B, PinName=\"Output4\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"blue\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
		CustomProperties Pin(PinId=B4BD483E4F05528F35EB8FBF38FCF996, PinName=\"Output5\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"mask\", PinType.PinSubCategory=\"alpha\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_40\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionTextureObjectParameter Name=\"MaterialExpressionTextureObjectParameter_1\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionTextureObjectParameter_1\"\n \
			ParameterName=\"Color_Tex\"\n \
			ExpressionGUID=04ACCB43429F06AA2119DD86025B4A29\n \
			Texture=Texture2D'ColorTexName'\n \
			SamplerType=SAMPLERTYPE_LinearColor\n \
			MaterialExpressionEditorX=-752\n \
			MaterialExpressionEditorY=-1409\n \
			MaterialExpressionGuid=B5ADCD2C47CF1548ECDAC788CB21F861\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionTextureObjectParameter'MaterialExpressionTextureObjectParameter_1'\n \
		NodePosX=-752\n \
		NodePosY=-1409\n \
		bCanRenameNode=True\n \
		NodeGuid=095C740644B65FD981799C8DAFA00EA4\n \
		CustomProperties Pin(PinId=32540BF749EBD6E4F46085B0B17BDFD7, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 E6C2AD7E4EF89F219FB9D2A1922E3A58, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_41\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_5\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionScalarParameter_5\"\n \
			DefaultValue= OpacitySDFName \n \
			ParameterName=\"bOpacitySDF\"\n \
			ExpressionGUID=BC376E0C49FA7C49EC5EC1AAC953ED7B\n \
			MaterialExpressionEditorX=-288\n \
			MaterialExpressionEditorY=-528\n \
			MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_5'\n \
		NodePosX=-288\n \
		NodePosY=-528\n \
		bCanRenameNode=True\n \
		NodeGuid=511484CC419DBE9540F4C2B32AEDE0CE\n \
		CustomProperties Pin(PinId=92C1A2D4410BAB82D3EB4B851B94B282, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 EC66A232432C44264165F39ADD38F5E8, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_42\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_6\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionScalarParameter_6\"\n \
			DefaultValue= ColorSDFName \n \
			ParameterName=\"bColorSDF\"\n \
			ExpressionGUID=471992034394581711EE028F39A89EE8\n \
			MaterialExpressionEditorX=-288\n \
			MaterialExpressionEditorY=-608\n \
			MaterialExpressionGuid=A35A4B48497D330D9FADFEB452B64D8F\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_6'\n \
		NodePosX=-288\n \
		NodePosY=-608\n \
		bCanRenameNode=True\n \
		NodeGuid=23568EEC4007E00BC95FD8985DCAA762\n \
		CustomProperties Pin(PinId=C1F6BB01492AD497AA16D4A286946344, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 32DD88A945F271528F9E7B85EE544786, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode Name=\"MaterialGraphNode_43\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionScalarParameter Name=\"MaterialExpressionScalarParameter_3\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionScalarParameter_3\"\n \
			DefaultValue= SDFThresoldName \n \
			ParameterName=\"SDFThresold\"\n \
			ExpressionGUID=5F309C4D49448BBF5499A292C49F42E0\n \
			MaterialExpressionEditorX=-288\n \
			MaterialExpressionEditorY=-192\n \
			MaterialExpressionGuid=C3199C3C44772B6C093BB5BF2EAA4AE5\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpression=MaterialExpressionScalarParameter'MaterialExpressionScalarParameter_3'\n \
		NodePosX=-288\n \
		NodePosY=-192\n \
		bCanRenameNode=True\n \
		NodeGuid=73995D004C453208E5A5FA88D2D9A9F2\n \
		CustomProperties Pin(PinId=878EBCB749C1AD81A26F7D95D5DC8B21, PinName=\"Output\", PinFriendlyName=\" \", Direction=\"EGPD_Output\", PinType.PinCategory=\"\", PinType.PinSubCategory=\"\", PinType.PinSubCategoryObject=None, PinType.PinSubCategoryMemberReference=(), PinType.PinValueType=(), PinType.bIsMap=False, PinType.bIsSet=False, PinType.bIsArray=False, PinType.bIsReference=False, PinType.bIsConst=False, PinType.bIsWeakPointer=False, LinkedTo=(MaterialGraphNode_19 76FA4F8043A12C8589FB80805850F4F7, ), PersistentGuid=00000000000000000000000000000000, bHidden=False, bNotConnectable=False, bDefaultValueIsReadOnly=False, bDefaultValueIsIgnored=False, bAdvancedView=False, )\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode_Comment Name=\"MaterialGraphNode_Comment_2\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionComment Name=\"MaterialExpressionComment_11\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionComment_11\"\n \
			SizeX=400\n \
			SizeY=1024\n \
			Text=\"color\"\n \
			MaterialExpressionEditorX=-832\n \
			MaterialExpressionEditorY=-2048\n \
			MaterialExpressionGuid=5E6561AC4262F454CE1FEFA7EB75E8ED\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpressionComment=MaterialExpressionComment'MaterialExpressionComment_11'\n \
		NodePosX=-832\n \
		NodePosY=-2048\n \
		NodeHeight=1024\n \
		NodeComment=\"color\"\n \
		NodeGuid=A13A96E049FDE69A1A65CE8B38DB0E1C\n \
	End Object\n \
	Begin Object Class=/Script/UnrealEd.MaterialGraphNode_Comment Name=\"MaterialGraphNode_Comment_3\"\n \
		Begin Object Class=/Script/Engine.MaterialExpressionComment Name=\"MaterialExpressionComment_12\"\n \
		End Object\n \
		Begin Object Name=\"MaterialExpressionComment_12\"\n \
			SizeX=400\n \
			SizeY=1024\n \
			Text=\"opacity\"\n \
			MaterialExpressionEditorX=-816\n \
			MaterialExpressionEditorY=-576\n \
			MaterialExpressionGuid=CE2D966E49F28E8C0E430EAF0DB05F2F\n \
			Material=PreviewMaterial'/Engine/Transient.MB_Umg_1Image'\n \
		End Object\n \
		MaterialExpressionComment=MaterialExpressionComment'MaterialExpressionComment_12'\n \
		NodePosX=-816\n \
		NodePosY=-576\n \
		NodeHeight=1024\n \
		NodeComment=\"opacity\"\n \
		NodeGuid=0E15A2AA42AD85F30BF584944C01C64A\n \
	End Object\n \
	";
	OutputString = Part1.Append(Part2.Append(Part3.Append(Part4.Append(Part5.Append(Part6)))));
	return OutputString;
}

#undef LOCTEXT_NAMESPACE
