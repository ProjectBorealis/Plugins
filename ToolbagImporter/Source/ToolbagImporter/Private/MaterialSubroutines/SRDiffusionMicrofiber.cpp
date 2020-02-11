/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRDiffusionMicrofiber.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USRDiffusionMicrofiber::USRDiffusionMicrofiber( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRDiffusionMicrofiber"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRDiffusionMicrofiber::GetName ()
{
	return "SRDiffusionMicrofiber";
}

void USRDiffusionMicrofiber::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add( new mset::MatFieldTexture( "Fuzz Map", "Fuzz Map", Material, ImportUI ));
	MatFields.Add( new mset::MatFieldFloat( "Fuzz", "Fuzz", Material ));
	MatFields.Add( new mset::MatFieldColor( "Fuzz Color", "Fuzz Color", Material ));
	MatFields.Add( new mset::MatFieldFloat( "Fuzz Scatter", "Fuzz Scatter", Material ));
	MatFields.Add( new mset::MatFieldFloat( "Fuzz Occlusion", "Fuzz Occlusion", Material ));
	MatFields.Add( new mset::MatFieldBool( "Mask Fuzz with Gloss", "Mask Fuzz with Gloss", Material ));
}


void USRDiffusionMicrofiber::LateSetup( UMaterial* Material )
{
	if (Material->BaseColor.Expression != NULL)
	{
		for (int32 j = 0; j < FunctionCallExpression->FunctionInputs.Num(); ++j)
		{
			FFunctionExpressionInput* Input = &(FunctionCallExpression->FunctionInputs[j]);
			if (Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Base Color" ) ))
			{
				Input->Input.Connect( Material->BaseColor.OutputIndex, Material->BaseColor.Expression );
			}
		}
	}
	for (int32 j = 0; j < FunctionCallExpression->FunctionOutputs.Num(); ++j)
	{
		FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[j]);
		if (Output->ExpressionOutput->OutputName.ToString().Equals( TEXT( "Combined Base Color" ) ))
		{
			Material->BaseColor.Connect( j, FunctionCallExpression );
		}
	}
}