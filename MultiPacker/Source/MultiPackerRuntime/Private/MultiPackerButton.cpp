/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerButton.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MultiPackerDataBase.h"

UMultiPackerButton::UMultiPackerButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetStyle.Normal.SetResourceObject(NULL);
	WidgetStyle.Pressed.SetResourceObject(NULL);
	WidgetStyle.Hovered.SetResourceObject(NULL);
	WidgetStyle.Disabled.SetResourceObject(NULL);
	ProcessChanges();
}

#if WITH_EDITOR  
void UMultiPackerButton::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerNormalBaseStyle)))       { N_BaseStyleChanged = true;	    }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerNormalAdditionStyle)))   { N_AdditionStyleChanged = true; }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerNormalDetailStyle)))     { N_DetailStyleChanged = true;   }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerHoveredBaseStyle)))      { H_BaseStyleChanged = true;     }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerHoveredAdditionStyle)))  { H_AdditionStyleChanged = true; }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerHoveredDetailStyle)))    { H_DetailStyleChanged = true;   }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerPressedBaseStyle)))      { P_BaseStyleChanged = true;     }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerPressedAdditionStyle)))  { P_AdditionStyleChanged = true; }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerPressedDetailStyle)))    { P_DetailStyleChanged = true;   }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerDisabledBaseStyle)))     { D_BaseStyleChanged = true;     }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerDisabledAdditionStyle))) { D_AdditionStyleChanged = true; }
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerButton, LayerDisabledDetailStyle)))   { D_DetailStyleChanged = true;   }
	ProcessChanges();
}
#endif

void UMultiPackerButton::ProcessChanges()
{
	ProcessBooleansLayers(TypeLayerNormal, N_layer_1, N_layer_2, N_layer_3);
	ProcessBooleansLayers(TypeLayerHovered, H_layer_1, H_layer_2, H_layer_3);
	ProcessBooleansLayers(TypeLayerPressed, P_layer_1, P_layer_2, P_layer_3);
	ProcessBooleansLayers(TypeLayerDisabled, D_layer_1, D_layer_2, D_layer_3);

	ProcessLayerDatabaseAndStyle(TypeLayerNormal, N_LayerBase, N_LayerAddition, N_LayerDetail, LayerNormalBaseStyle, LayerNormalAdditionStyle, LayerNormalDetailStyle,
		NormalBaseColorOutline, NormalBaseColorInterior, NormalBaseOutlineThresold, NormalBaseSDFThresold,
		NormalAdditionColorOutline, NormalAdditionColorInterior, NormalAdditionOutlineThresold, NormalAdditionSDFThresold,
		NormalDetailColorOutline, NormalDetailColorInterior, NormalDetailOutlineThresold, NormalDetailSDFThresold,
		N_BaseStyleChanged, N_AdditionStyleChanged, N_DetailStyleChanged);

	ProcessLayerDatabaseAndStyle(TypeLayerHovered, H_LayerBase, H_LayerAddition, H_LayerDetail, LayerHoveredBaseStyle, LayerHoveredAdditionStyle, LayerHoveredDetailStyle,
		HoveredBaseColorOutline, HoveredBaseColorInterior, HoveredBaseOutlineThresold, HoveredBaseSDFThresold,
		HoveredAdditionColorOutline, HoveredAdditionColorInterior, HoveredAdditionOutlineThresold, HoveredAdditionSDFThresold,
		HoveredDetailColorOutline, HoveredDetailColorInterior, HoveredDetailOutlineThresold, HoveredDetailSDFThresold,
		H_BaseStyleChanged, H_AdditionStyleChanged, H_DetailStyleChanged);

	ProcessLayerDatabaseAndStyle(TypeLayerPressed, P_LayerBase, P_LayerAddition, P_LayerDetail, LayerPressedBaseStyle, LayerPressedAdditionStyle, LayerPressedDetailStyle,
		PressedBaseColorOutline, PressedBaseColorInterior, PressedBaseOutlineThresold, PressedBaseSDFThresold,
		PressedAdditionColorOutline, PressedAdditionColorInterior, PressedAdditionOutlineThresold, PressedAdditionSDFThresold,
		PressedDetailColorOutline, PressedDetailColorInterior, PressedDetailOutlineThresold, PressedDetailSDFThresold,
		P_BaseStyleChanged, P_AdditionStyleChanged, P_DetailStyleChanged);

	ProcessLayerDatabaseAndStyle(TypeLayerDisabled, D_LayerBase, D_LayerAddition, D_LayerDetail, LayerDisabledBaseStyle, LayerDisabledAdditionStyle, LayerDisabledDetailStyle,
		DisabledBaseColorOutline, DisabledBaseColorInterior, DisabledBaseOutlineThresold, DisabledBaseSDFThresold,
		DisabledAdditionColorOutline, DisabledAdditionColorInterior, DisabledAdditionOutlineThresold, DisabledAdditionSDFThresold,
		DisabledDetailColorOutline, DisabledDetailColorInterior, DisabledDetailOutlineThresold, DisabledDetailSDFThresold,
		D_BaseStyleChanged, D_AdditionStyleChanged, D_DetailStyleChanged);

	if (SetAsImage)
	{
		WidgetStyle.Normal.DrawAs = ESlateBrushDrawType::Image;
		WidgetStyle.Pressed.DrawAs = ESlateBrushDrawType::Image;
		WidgetStyle.Hovered.DrawAs = ESlateBrushDrawType::Image;
		WidgetStyle.Disabled.DrawAs = ESlateBrushDrawType::Image;
	}
	MID_Normal = UMultiPackerImageCore::CreateMaterial(TypeLayerNormal, this, N_LayerBase, N_LayerAddition, N_LayerDetail);
	MID_Hovered = UMultiPackerImageCore::CreateMaterial(TypeLayerPressed, this, P_LayerBase, P_LayerAddition, P_LayerDetail);
	MID_Pressed = UMultiPackerImageCore::CreateMaterial(TypeLayerHovered, this, H_LayerBase, H_LayerAddition, H_LayerDetail);
	WidgetStyle.Normal.SetResourceObject(MID_Normal);
	WidgetStyle.Pressed.SetResourceObject(MID_Hovered);
	WidgetStyle.Hovered.SetResourceObject(MID_Pressed);
	if (SetDisabledStyle)
	{
		MID_Disabled = UMultiPackerImageCore::CreateMaterial(TypeLayerDisabled, this, D_LayerBase, D_LayerAddition, D_LayerDetail);
		WidgetStyle.Disabled.SetResourceObject(MID_Disabled);
	}
	if (MID_Normal != NULL)
	{
		SetNormalBaseColorOutline(NormalBaseColorOutline);
		SetNormalBaseColorInterior(NormalBaseColorInterior);
		SetNormalBaseOutlineThresold(NormalBaseOutlineThresold);
		SetNormalBaseSDFThresold(NormalBaseSDFThresold);
		SetNormalAdditionColorOutline(NormalAdditionColorOutline);
		SetNormalAdditionColorInterior(NormalAdditionColorInterior);
		SetNormalAdditionOutlineThresold(NormalAdditionOutlineThresold);
		SetNormalAdditionSDFThresold(NormalAdditionSDFThresold);
		SetNormalDetailColorOutline(NormalDetailColorOutline);
		SetNormalDetailColorInterior(NormalDetailColorInterior);
		SetNormalDetailOutlineThresold(NormalDetailOutlineThresold);
		SetNormalDetailSDFThresold(NormalDetailSDFThresold);
	}
	if (MID_Hovered != NULL)
	{
		SetHoveredBaseColorOutline(HoveredBaseColorOutline);
		SetHoveredBaseColorInterior(HoveredBaseColorInterior);
		SetHoveredBaseOutlineThresold(HoveredBaseOutlineThresold);
		SetHoveredBaseSDFThresold(HoveredBaseSDFThresold);
		SetHoveredAdditionColorOutline(HoveredAdditionColorOutline);
		SetHoveredAdditionColorInterior(HoveredAdditionColorInterior);
		SetHoveredAdditionOutlineThresold(HoveredAdditionOutlineThresold);
		SetHoveredAdditionSDFThresold(HoveredAdditionSDFThresold);
		SetHoveredDetailColorOutline(HoveredDetailColorOutline);
		SetHoveredDetailColorInterior(HoveredDetailColorInterior);
		SetHoveredDetailOutlineThresold(HoveredDetailOutlineThresold);
		SetHoveredDetailSDFThresold(HoveredDetailSDFThresold);
	}
	if (MID_Pressed != NULL)
	{
		SetPressedBaseColorOutline(PressedBaseColorOutline);
		SetPressedBaseColorInterior(PressedBaseColorInterior);
		SetPressedBaseOutlineThresold(PressedBaseOutlineThresold);
		SetPressedBaseSDFThresold(PressedBaseSDFThresold);
		SetPressedAdditionColorOutline(PressedAdditionColorOutline);
		SetPressedAdditionColorInterior(PressedAdditionColorInterior);
		SetPressedAdditionOutlineThresold(PressedAdditionOutlineThresold);
		SetPressedAdditionSDFThresold(PressedAdditionSDFThresold);
		SetPressedDetailColorOutline(PressedDetailColorOutline);
		SetPressedDetailColorInterior(PressedDetailColorInterior);
		SetPressedDetailOutlineThresold(PressedDetailOutlineThresold);
		SetPressedDetailSDFThresold(PressedDetailSDFThresold);
	}
	if (MID_Disabled != NULL)
	{
		SetDisabledBaseColorOutline(DisabledBaseColorOutline);
		SetDisabledBaseColorInterior(DisabledBaseColorInterior);
		SetDisabledBaseOutlineThresold(DisabledBaseOutlineThresold);
		SetDisabledBaseSDFThresold(DisabledBaseSDFThresold);
		SetDisabledAdditionColorOutline(DisabledAdditionColorOutline);
		SetDisabledAdditionColorInterior(DisabledAdditionColorInterior);
		SetDisabledAdditionOutlineThresold(DisabledAdditionOutlineThresold);
		SetDisabledAdditionSDFThresold(DisabledAdditionSDFThresold);
		SetDisabledDetailColorOutline(DisabledDetailColorOutline);
		SetDisabledDetailColorInterior(DisabledDetailColorInterior);
		SetDisabledDetailOutlineThresold(DisabledDetailOutlineThresold);
		SetDisabledDetailSDFThresold(DisabledDetailSDFThresold);
	}
}

void UMultiPackerButton::SetVarsFromStyle(FLayerDatabase InLayerBase, FLinearColor& InOutline, FLinearColor& InColor, float& InOutlineThresold, float& InSDFThresold)
{
	InOutline = InLayerBase.ColorOutline;
	InColor = InLayerBase.ColorInterior;
	InOutlineThresold = InLayerBase.OutlineThresold;
	InSDFThresold = InLayerBase.SDFThresold;
}

void UMultiPackerButton::ProcessBooleansLayers(EMultiPackerImageLayer TypeLayer, bool& Layer_1, bool& Layer_2, bool& Layer_3)
{
	Layer_1 = Layer_2 = Layer_3 = false;
	switch (TypeLayer)
	{
	case EMultiPackerImageLayer::EMCE_Option3:
		Layer_3 = true;
	case EMultiPackerImageLayer::EMCE_Option2:
		Layer_2 = true;
	case EMultiPackerImageLayer::EMCE_Option1:
		Layer_1 = true;
		break;
	}
}

void UMultiPackerButton::ProcessLayerDatabaseAndStyle(
	EMultiPackerImageLayer TypeLayer, FLayerDatabase& Base, FLayerDatabase& Addition, FLayerDatabase& Detail, 
	UMultiPackerLayerDatabase* StyleBase, UMultiPackerLayerDatabase* StyleAddition, UMultiPackerLayerDatabase* StyleDetail, 
	FLinearColor& InBOutline, FLinearColor& InBColor, float& InBOutlineThresold, float& InBSDFThresold, 
	FLinearColor& InAOutline, FLinearColor& InAColor, float& InAOutlineThresold, float& InASDFThresold, 
	FLinearColor& InDOutline, FLinearColor& InDColor, float& InDOutlineThresold, float& InDSDFThresold,
	bool& bnormal, bool& baddition, bool& bdetail)
{
	switch (TypeLayer)
	{
	case EMultiPackerImageLayer::EMCE_Option3:
		if (StyleDetail != nullptr)
		{
			Detail = UMultiPackerImageCore::ProcessStyle(Detail, StyleDetail);
			if (bdetail)
			{
				SetVarsFromStyle(Detail, InDOutline, InDColor, InDOutlineThresold, InDSDFThresold);
				//bdetail = false;
			}
		}
		Detail = UMultiPackerImageCore::ProcessChanges(Detail);
	case EMultiPackerImageLayer::EMCE_Option2:
		if (StyleAddition != nullptr)
		{
			Addition = UMultiPackerImageCore::ProcessStyle(Addition, StyleAddition);
			if (baddition)
			{
				SetVarsFromStyle(Addition, InAOutline, InAColor, InAOutlineThresold, InASDFThresold);
				//baddition = false;
			}
		}
		Addition = UMultiPackerImageCore::ProcessChanges(Addition);
	case EMultiPackerImageLayer::EMCE_Option1:
		if (StyleBase != nullptr)
		{
			Base = UMultiPackerImageCore::ProcessStyle(Base, StyleBase);
			if (bnormal)
			{
				SetVarsFromStyle(Base, InBOutline, InBColor, InBOutlineThresold, InBSDFThresold);
				//bnormal = false;
			}
		}
		Base = UMultiPackerImageCore::ProcessChanges(Base);
		break;
	}
}

void UMultiPackerButton::SynchronizeProperties()
{
	// Property update(s)
	Super::SynchronizeProperties();
	ProcessChanges();
}

void UMultiPackerButton::SetNormalBaseColorOutline(FLinearColor InBaseColorOutline)
{
	N_LayerBase.ColorOutline = NormalBaseColorOutline = InBaseColorOutline;
	MID_Normal->SetVectorParameterValue(FName(*FString("ColorOutline").Append(N_LayerBase.Multiple ? "_1" : "")), InBaseColorOutline);
}

void UMultiPackerButton::SetNormalBaseColorInterior(FLinearColor InBaseColorInterior)
{
	N_LayerBase.ColorInterior = NormalBaseColorInterior = InBaseColorInterior;
	MID_Normal->SetVectorParameterValue(FName(*FString("ColorInterior").Append(N_LayerBase.Multiple ? "_1" : "")), InBaseColorInterior);
}

void UMultiPackerButton::SetNormalBaseOutlineThresold(float InBaseOutlineThresold)
{
	N_LayerBase.OutlineThresold = NormalBaseOutlineThresold = InBaseOutlineThresold;
	MID_Normal->SetScalarParameterValue(FName(*FString("OutlineThresold").Append(N_layer_2 ? "_1" : "")), InBaseOutlineThresold);
}

void UMultiPackerButton::SetNormalBaseSDFThresold(float InBaseSDFThresold)
{
	N_LayerBase.SDFThresold = NormalBaseSDFThresold = InBaseSDFThresold;
	MID_Normal->SetScalarParameterValue(FName(*FString("SDFThresold").Append(N_layer_2 ? "_1" : "")), InBaseSDFThresold);
}

void UMultiPackerButton::SetNormalAdditionColorOutline(FLinearColor InAdditionColorOutline)
{
	N_LayerAddition.ColorOutline = NormalAdditionColorOutline = InAdditionColorOutline;
	MID_Normal->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_2")), InAdditionColorOutline);
}

void UMultiPackerButton::SetNormalAdditionColorInterior(FLinearColor InAdditionColorInterior)
{
	N_LayerAddition.ColorInterior = NormalAdditionColorInterior = InAdditionColorInterior;
	MID_Normal->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_2")), InAdditionColorInterior);
}

void UMultiPackerButton::SetNormalAdditionOutlineThresold(float InAdditionOutlineThresold)
{
	float OutlineThresold = InAdditionOutlineThresold;
	N_LayerAddition.OutlineThresold = NormalAdditionOutlineThresold = InAdditionOutlineThresold;
	MID_Normal->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_2")), InAdditionOutlineThresold);
}

void UMultiPackerButton::SetNormalAdditionSDFThresold(float InAdditionSDFThresold)
{
	N_LayerAddition.SDFThresold = NormalAdditionSDFThresold = InAdditionSDFThresold;
	MID_Normal->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_2")), InAdditionSDFThresold);
}

void UMultiPackerButton::SetNormalDetailColorOutline(FLinearColor InDetailColorOutline)
{
	N_LayerDetail.ColorOutline = NormalDetailColorOutline = InDetailColorOutline;
	MID_Normal->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_3")), InDetailColorOutline);
}

void UMultiPackerButton::SetNormalDetailColorInterior(FLinearColor InDetailColorInterior)
{
	N_LayerDetail.ColorInterior = NormalDetailColorInterior = InDetailColorInterior;
	MID_Normal->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_3")), InDetailColorInterior);
}

void UMultiPackerButton::SetNormalDetailOutlineThresold(float InDetailOutlineThresold)
{
	N_LayerDetail.OutlineThresold = NormalDetailOutlineThresold = InDetailOutlineThresold;
	MID_Normal->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_3")), InDetailOutlineThresold);
}

void UMultiPackerButton::SetNormalDetailSDFThresold(float InDetailSDFThresold)
{
	N_LayerDetail.SDFThresold = NormalDetailSDFThresold = InDetailSDFThresold;
	MID_Normal->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_3")), InDetailSDFThresold);
}

void UMultiPackerButton::SetHoveredBaseColorOutline(FLinearColor InBaseColorOutline)
{
	H_LayerBase.ColorOutline = HoveredBaseColorOutline = InBaseColorOutline;
	MID_Hovered->SetVectorParameterValue(FName(*FString("ColorOutline").Append(H_LayerBase.Multiple ? "_1" : "")), InBaseColorOutline);
}

void UMultiPackerButton::SetHoveredBaseColorInterior(FLinearColor InBaseColorInterior)
{
	H_LayerBase.ColorInterior = HoveredBaseColorInterior = InBaseColorInterior;
	MID_Hovered->SetVectorParameterValue(FName(*FString("ColorInterior").Append(H_LayerBase.Multiple ? "_1" : "")), InBaseColorInterior);
}

void UMultiPackerButton::SetHoveredBaseOutlineThresold(float InBaseOutlineThresold)
{
	H_LayerBase.OutlineThresold = HoveredBaseOutlineThresold = InBaseOutlineThresold;
	MID_Hovered->SetScalarParameterValue(FName(*FString("OutlineThresold").Append(H_layer_2 ? "_1" : "")), InBaseOutlineThresold);
}

void UMultiPackerButton::SetHoveredBaseSDFThresold(float InBaseSDFThresold)
{
	H_LayerBase.SDFThresold = HoveredBaseSDFThresold = InBaseSDFThresold;
	MID_Hovered->SetScalarParameterValue(FName(*FString("SDFThresold").Append(H_layer_2 ? "_1" : "")), InBaseSDFThresold);
}

void UMultiPackerButton::SetHoveredAdditionColorOutline(FLinearColor InAdditionColorOutline)
{
	H_LayerAddition.ColorOutline = HoveredAdditionColorOutline = InAdditionColorOutline;
	MID_Hovered->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_2")), InAdditionColorOutline);
}

void UMultiPackerButton::SetHoveredAdditionColorInterior(FLinearColor InAdditionColorInterior)
{
	H_LayerAddition.ColorInterior = HoveredAdditionColorInterior = InAdditionColorInterior;
	MID_Hovered->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_2")), InAdditionColorInterior);
}

void UMultiPackerButton::SetHoveredAdditionOutlineThresold(float InAdditionOutlineThresold)
{
	H_LayerAddition.OutlineThresold = HoveredAdditionOutlineThresold = InAdditionOutlineThresold;
	MID_Hovered->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_2")), InAdditionOutlineThresold);
}

void UMultiPackerButton::SetHoveredAdditionSDFThresold(float InAdditionSDFThresold)
{
	H_LayerAddition.SDFThresold = HoveredAdditionSDFThresold = InAdditionSDFThresold;
	MID_Hovered->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_2")), InAdditionSDFThresold);
}

void UMultiPackerButton::SetHoveredDetailColorOutline(FLinearColor InDetailColorOutline)
{
	H_LayerDetail.ColorOutline = HoveredDetailColorOutline = InDetailColorOutline;
	MID_Hovered->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_3")), InDetailColorOutline);
}

void UMultiPackerButton::SetHoveredDetailColorInterior(FLinearColor InDetailColorInterior)
{
	H_LayerDetail.ColorInterior = HoveredDetailColorInterior = InDetailColorInterior;
	MID_Hovered->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_3")), InDetailColorInterior);
}

void UMultiPackerButton::SetHoveredDetailOutlineThresold(float InDetailOutlineThresold)
{
	H_LayerDetail.OutlineThresold = HoveredDetailOutlineThresold = InDetailOutlineThresold;
	MID_Hovered->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_3")), InDetailOutlineThresold);
}

void UMultiPackerButton::SetHoveredDetailSDFThresold(float InDetailSDFThresold)
{
	H_LayerDetail.SDFThresold = HoveredDetailSDFThresold = InDetailSDFThresold;
	MID_Hovered->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_3")), InDetailSDFThresold);
}

void UMultiPackerButton::SetPressedBaseColorOutline(FLinearColor InBaseColorOutline)
{
	P_LayerBase.ColorOutline = PressedBaseColorOutline = InBaseColorOutline;
	MID_Pressed->SetVectorParameterValue(FName(*FString("ColorOutline").Append(P_LayerBase.Multiple ? "_1" : "")), InBaseColorOutline);
}

void UMultiPackerButton::SetPressedBaseColorInterior(FLinearColor InBaseColorInterior)
{
	P_LayerBase.ColorInterior = PressedBaseColorInterior = InBaseColorInterior;
	MID_Pressed->SetVectorParameterValue(FName(*FString("ColorInterior").Append(P_LayerBase.Multiple ? "_1" : "")), InBaseColorInterior);
}

void UMultiPackerButton::SetPressedBaseOutlineThresold(float InBaseOutlineThresold)
{
	P_LayerBase.OutlineThresold = PressedBaseOutlineThresold = InBaseOutlineThresold;
	MID_Pressed->SetScalarParameterValue(FName(*FString("OutlineThresold").Append(P_layer_2 ? "_1" : "")), InBaseOutlineThresold);
}

void UMultiPackerButton::SetPressedBaseSDFThresold(float InBaseSDFThresold)
{
	P_LayerBase.SDFThresold = PressedBaseSDFThresold = InBaseSDFThresold;
	MID_Pressed->SetScalarParameterValue(FName(*FString("SDFThresold").Append(P_layer_2 ? "_1" : "")), InBaseSDFThresold);
}

void UMultiPackerButton::SetPressedAdditionColorOutline(FLinearColor InAdditionColorOutline)
{
	P_LayerAddition.ColorOutline = PressedAdditionColorOutline = InAdditionColorOutline;
	MID_Pressed->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_2")), InAdditionColorOutline);
}

void UMultiPackerButton::SetPressedAdditionColorInterior(FLinearColor InAdditionColorInterior)
{
	P_LayerAddition.ColorInterior = PressedAdditionColorInterior = InAdditionColorInterior;
	MID_Pressed->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_2")), InAdditionColorInterior);
}

void UMultiPackerButton::SetPressedAdditionOutlineThresold(float InAdditionOutlineThresold)
{
	P_LayerAddition.OutlineThresold = PressedAdditionOutlineThresold = InAdditionOutlineThresold;
	MID_Pressed->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_2")), InAdditionOutlineThresold);
}

void UMultiPackerButton::SetPressedAdditionSDFThresold(float InAdditionSDFThresold)
{
	P_LayerAddition.SDFThresold = PressedAdditionSDFThresold = InAdditionSDFThresold;
	MID_Pressed->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_2")), InAdditionSDFThresold);
}

void UMultiPackerButton::SetPressedDetailColorOutline(FLinearColor InDetailColorOutline)
{
	P_LayerDetail.ColorOutline = PressedDetailColorOutline = InDetailColorOutline;
	MID_Pressed->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_3")), InDetailColorOutline);
}

void UMultiPackerButton::SetPressedDetailColorInterior(FLinearColor InDetailColorInterior)
{
	P_LayerDetail.ColorInterior = PressedDetailColorInterior = InDetailColorInterior;
	MID_Pressed->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_3")), InDetailColorInterior);
}

void UMultiPackerButton::SetPressedDetailOutlineThresold(float InDetailOutlineThresold)
{
	P_LayerDetail.OutlineThresold = PressedDetailOutlineThresold = InDetailOutlineThresold;
	MID_Pressed->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_3")), InDetailOutlineThresold);
}

void UMultiPackerButton::SetPressedDetailSDFThresold(float InDetailSDFThresold)
{
	P_LayerDetail.SDFThresold = PressedDetailSDFThresold = InDetailSDFThresold;
	MID_Pressed->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_3")), InDetailSDFThresold);
}

void UMultiPackerButton::SetDisabledBaseColorOutline(FLinearColor InBaseColorOutline)
{
	D_LayerBase.ColorOutline = DisabledBaseColorOutline = InBaseColorOutline;
	MID_Disabled->SetVectorParameterValue(FName(*FString("ColorOutline").Append(D_LayerBase.Multiple ? "_1" : "")), InBaseColorOutline);
}

void UMultiPackerButton::SetDisabledBaseColorInterior(FLinearColor InBaseColorInterior)
{
	D_LayerBase.ColorInterior = DisabledBaseColorInterior = InBaseColorInterior;
	MID_Disabled->SetVectorParameterValue(FName(*FString("ColorInterior").Append(D_LayerBase.Multiple ? "_1" : "")), InBaseColorInterior);
}

void UMultiPackerButton::SetDisabledBaseOutlineThresold(float InBaseOutlineThresold)
{
	D_LayerBase.OutlineThresold = DisabledBaseOutlineThresold = InBaseOutlineThresold;
	MID_Disabled->SetScalarParameterValue(FName(*FString("OutlineThresold").Append(D_layer_2 ? "_1" : "")), InBaseOutlineThresold);
}

void UMultiPackerButton::SetDisabledBaseSDFThresold(float InBaseSDFThresold)
{
	D_LayerBase.SDFThresold = DisabledBaseSDFThresold = InBaseSDFThresold;
	MID_Disabled->SetScalarParameterValue(FName(*FString("SDFThresold").Append(D_layer_2 ? "_1" : "")), InBaseSDFThresold);
}

void UMultiPackerButton::SetDisabledAdditionColorOutline(FLinearColor InAdditionColorOutline)
{
	D_LayerAddition.ColorOutline = DisabledAdditionColorOutline = InAdditionColorOutline;
	MID_Disabled->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_2")), InAdditionColorOutline);
}

void UMultiPackerButton::SetDisabledAdditionColorInterior(FLinearColor InAdditionColorInterior)
{
	D_LayerAddition.ColorInterior = DisabledAdditionColorInterior = InAdditionColorInterior;
	MID_Disabled->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_2")), InAdditionColorInterior);
}

void UMultiPackerButton::SetDisabledAdditionOutlineThresold(float InAdditionOutlineThresold)
{
	D_LayerAddition.OutlineThresold = DisabledAdditionOutlineThresold = InAdditionOutlineThresold;
	MID_Disabled->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_2")), InAdditionOutlineThresold);
}

void UMultiPackerButton::SetDisabledAdditionSDFThresold(float InAdditionSDFThresold)
{
	D_LayerAddition.SDFThresold = DisabledAdditionSDFThresold = InAdditionSDFThresold;
	MID_Disabled->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_2")), InAdditionSDFThresold);
}

void UMultiPackerButton::SetDisabledDetailColorOutline(FLinearColor InDetailColorOutline)
{
	D_LayerDetail.ColorOutline = DisabledDetailColorOutline = InDetailColorOutline;
	MID_Disabled->SetVectorParameterValue(FName(*FString("ColorOutline").Append("_3")), InDetailColorOutline);
}

void UMultiPackerButton::SetDisabledDetailColorInterior(FLinearColor InDetailColorInterior)
{
	D_LayerDetail.ColorInterior = DisabledDetailColorInterior = InDetailColorInterior;
	MID_Disabled->SetVectorParameterValue(FName(*FString("ColorInterior").Append("_3")), InDetailColorInterior);
}

void UMultiPackerButton::SetDisabledDetailOutlineThresold(float InDetailOutlineThresold)
{
	D_LayerDetail.OutlineThresold = DisabledDetailOutlineThresold = InDetailOutlineThresold;
	MID_Disabled->SetScalarParameterValue(FName(*FString("OutlineThresold").Append("_3")), InDetailOutlineThresold);
}

void UMultiPackerButton::SetDisabledDetailSDFThresold(float InDetailSDFThresold)
{
	D_LayerDetail.SDFThresold = DisabledDetailSDFThresold = InDetailSDFThresold;
	MID_Disabled->SetScalarParameterValue(FName(*FString("SDFThresold").Append("_3")), InDetailSDFThresold);
}
