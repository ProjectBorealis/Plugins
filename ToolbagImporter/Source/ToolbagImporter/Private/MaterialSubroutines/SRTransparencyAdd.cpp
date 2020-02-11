/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRTransparencyAdd.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Materials/MaterialExpressionFunctionOutput.h>
#include "../UI/ToolbagImportUI.h"


USRTransparencyAdd::USRTransparencyAdd( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRTransparency"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRTransparencyAdd::GetName ()
{
	return "SRTransparencyAdd";
}

void USRTransparencyAdd::SetupMaterial ( UMaterial* Material )
{
	USRTransparency::SetupMaterial( Material );
	//Material->SetShadingModel( EMaterialShadingModel::MSM_Unlit );
	Material->BlendMode = EBlendMode::BLEND_Additive;
}

void USRTransparencyAdd::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	USRTransparency::SetupFields( ImportUI, Material );
	MatFields.Add(new mset::MatFieldColor("Tint", "Alpha Tint", Material));
	MatFields.Add(new mset::MatFieldBool("Include Diffuse", "Tint By Diffuse", Material));
}

void USRTransparencyAdd::LateSetup ( UMaterial* Material )
{
	USRTransparency::LateSetup( Material );
	for (int i = 0; i < FunctionCallExpression->Outputs.Num(); ++i)
	{
		FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[i]);
		if(Output->ExpressionOutput->OutputName.ToString().Equals(TEXT("AlphaOut")))
		{
			Material->Opacity.Connect(i, FunctionCallExpression);
		}
		else if(Output->ExpressionOutput->OutputName.ToString().Equals(TEXT("ColorOut")))
		{
			Material->BaseColor.Connect(i, FunctionCallExpression);
		}
	}
}
