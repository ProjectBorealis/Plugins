/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerImage.h"
#include "MultiPackerImageCore.h"
#include "MultiPackerLayerDatabase.h"

#define LOCTEXT_NAMESPACE "MultiPackerImage"

UMultiPackerImage::UMultiPackerImage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetBrushFromMaterial(NULL);
	ProcessChanges();
}

#if WITH_EDITOR  
void UMultiPackerImage::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerImage, LayerBaseStyle)))
	{
		BaseStyleChanged = true;
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerImage, LayerAdditionStyle)))
	{
		AdditionStyleChanged = true;
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerImage, LayerDetailStyle)))
	{
		DetailStyleChanged = true;
	}
	ProcessChanges();
}
#endif

void UMultiPackerImage::SetBaseColorOutline(FLinearColor InBaseColorOutline)
{
	LayerBase.ColorOutline = BaseColorOutline = InBaseColorOutline;
	MID_Image->SetVectorParameterValue(FName(*FString("ColorOutline").Append(layer_2 ? "_1" : "")), InBaseColorOutline);
}

void UMultiPackerImage::SetBaseColorInterior(FLinearColor InBaseColorInterior)
{
	LayerBase.ColorInterior = BaseColorInterior = InBaseColorInterior;
	MID_Image->SetVectorParameterValue(FName(*FString("ColorInterior").Append(layer_2 ? "_1" : "")), InBaseColorInterior);
}

void UMultiPackerImage::SetBaseOutlineThresold(float InBaseOutlineThresold)
{
	LayerBase.OutlineThresold = BaseOutlineThresold = InBaseOutlineThresold;
	MID_Image->SetScalarParameterValue(FName(*FString("OutlineThresold").Append(layer_2 ? "_1" : "")), InBaseOutlineThresold);
}

void UMultiPackerImage::SetBaseSDFThresold(float InBaseSDFThresold)
{
	LayerBase.SDFThresold = BaseSDFThresold = InBaseSDFThresold;
	MID_Image->SetScalarParameterValue(FName(*FString("SDFThresold").Append(layer_2 ? "_1" : "")), InBaseSDFThresold);
}

void UMultiPackerImage::SetAdditionColorOutline(FLinearColor InAdditionColorOutline)
{
	LayerAddition.ColorOutline = AdditionColorOutline = InAdditionColorOutline;
	MID_Image->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_2")), InAdditionColorOutline);
}

void UMultiPackerImage::SetAdditionColorInterior(FLinearColor InAdditionColorInterior)
{
	LayerAddition.ColorInterior = AdditionColorInterior = InAdditionColorInterior;
	MID_Image->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_2")), InAdditionColorInterior);
}

void UMultiPackerImage::SetAdditionOutlineThresold(float InAdditionOutlineThresold)
{
	LayerAddition.OutlineThresold = AdditionOutlineThresold = InAdditionOutlineThresold;
	MID_Image->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_2")), InAdditionOutlineThresold);
}

void UMultiPackerImage::SetAdditionSDFThresold(float InAdditionSDFThresold)
{
	LayerAddition.SDFThresold = AdditionSDFThresold = InAdditionSDFThresold;
	MID_Image->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_2")), InAdditionSDFThresold);
}

void UMultiPackerImage::SetDetailColorOutline(FLinearColor InDetailColorOutline)
{
	LayerDetail.ColorOutline = DetailColorOutline = InDetailColorOutline;
	MID_Image->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_3")), InDetailColorOutline);
}

void UMultiPackerImage::SetDetailColorInterior(FLinearColor InDetailColorInterior)
{
	LayerDetail.ColorInterior = DetailColorInterior = InDetailColorInterior;
	MID_Image->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_3")), InDetailColorInterior);
}

void UMultiPackerImage::SetDetailOutlineThresold(float InDetailOutlineThresold)
{
	LayerDetail.OutlineThresold = DetailOutlineThresold = InDetailOutlineThresold;
	MID_Image->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_3")), InDetailOutlineThresold);
}

void UMultiPackerImage::SetDetailSDFThresold(float InDetailSDFThresold)
{
	LayerDetail.SDFThresold = DetailSDFThresold = InDetailSDFThresold;
	MID_Image->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_3")), InDetailSDFThresold);
}

void UMultiPackerImage::ProcessChanges()
{
	layer_1 = layer_2 = layer_3 = false;
	switch (TypeLayer)
	{
	case EMultiPackerImageLayer::EMCE_Option3:
		layer_3 = true;
	case EMultiPackerImageLayer::EMCE_Option2:
		layer_2 = true;
	case EMultiPackerImageLayer::EMCE_Option1:
		layer_1 = true;
		break;
	}
	switch (TypeLayer)
	{
	case EMultiPackerImageLayer::EMCE_Option3:
		if (LayerDetailStyle != nullptr)
		{
			LayerDetail = UMultiPackerImageCore::ProcessStyle(LayerDetail, LayerDetailStyle);
			if (DetailStyleChanged)
			{
				SetVarsFromStyle(LayerDetail, DetailColorOutline, DetailColorInterior, DetailOutlineThresold, DetailSDFThresold);
				//DetailStyleChanged = false;
			}
		}
		LayerDetail = UMultiPackerImageCore::ProcessChanges(LayerDetail);
	case EMultiPackerImageLayer::EMCE_Option2:
		if (LayerAdditionStyle != nullptr)
		{
			LayerAddition = UMultiPackerImageCore::ProcessStyle(LayerAddition, LayerAdditionStyle);
			if (AdditionStyleChanged)
			{
				SetVarsFromStyle(LayerAddition, AdditionColorOutline, AdditionColorInterior, AdditionOutlineThresold, AdditionSDFThresold);
				//AdditionStyleChanged = false;
			}
		}
		LayerAddition = UMultiPackerImageCore::ProcessChanges(LayerAddition);
	case EMultiPackerImageLayer::EMCE_Option1:
		if (LayerBaseStyle != nullptr)
		{
			LayerBase = UMultiPackerImageCore::ProcessStyle(LayerBase, LayerBaseStyle);
			if (BaseStyleChanged)
			{
				SetVarsFromStyle(LayerBase, BaseColorOutline, BaseColorInterior, BaseOutlineThresold, BaseSDFThresold);
				//BaseStyleChanged = false;
			}
		}
		LayerBase = UMultiPackerImageCore::ProcessChanges(LayerBase);
		break;
	}
	MID_Image = UMultiPackerImageCore::CreateMaterial(TypeLayer, this, LayerBase, LayerAddition, LayerDetail);
	SetBrushFromMaterial(MID_Image);
	if (MID_Image != NULL)
	{
		SetBaseColorOutline(BaseColorOutline);
		SetBaseColorInterior(BaseColorInterior);
		SetBaseOutlineThresold(BaseOutlineThresold);
		SetBaseSDFThresold(BaseSDFThresold);
		SetAdditionColorOutline(AdditionColorOutline);
		SetAdditionColorInterior(AdditionColorInterior);
		SetAdditionOutlineThresold(AdditionOutlineThresold);
		SetAdditionSDFThresold(AdditionSDFThresold);
		SetDetailColorOutline(DetailColorOutline);
		SetDetailColorInterior(DetailColorInterior);
		SetDetailOutlineThresold(DetailOutlineThresold);
		SetDetailSDFThresold(DetailSDFThresold);
	}
}

void UMultiPackerImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ProcessChanges();
}

void UMultiPackerImage::SetVarsFromStyle(FLayerDatabase InLayerBase, FLinearColor& InOutline, FLinearColor& InColor, float& InOutlineThresold, float& InSDFThresold)
{
	InOutline = InLayerBase.ColorOutline;
	InColor = InLayerBase.ColorInterior;
	InOutlineThresold = InLayerBase.OutlineThresold;
	InSDFThresold = InLayerBase.SDFThresold;
}

#undef LOCTEXT_NAMESPACE