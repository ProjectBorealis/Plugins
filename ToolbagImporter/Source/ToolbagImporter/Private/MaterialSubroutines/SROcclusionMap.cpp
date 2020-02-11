/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SROcclusionMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Materials/MaterialExpressionFunctionOutput.h>
#include <Materials/MaterialExpressionFunctionInput.h>
#include "../UI/ToolbagImportUI.h"


USROcclusionMap::USROcclusionMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SROcclusionMap"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USROcclusionMap::GetName ()
{
	return "SROcclusionMap";
}

void USROcclusionMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add( new mset::MatFieldTexture("Occlusion Map", "Occlusion Map", Material, ImportUI));
	MatFields.Add( new mset::MatFieldEnum( "Channel;occlusion", "Occlusion Map Channel", Material ) );
	MatFields.Add( new mset::MatFieldFloat("Occlusion", "Occlusion Intensity", Material));
	MatFields.Add( new mset::MatFieldEnum("UV Set", "UV Set", Material));
	MatFields.Add( new mset::MatFieldEnum("Vertex Channel", "Vertex Channel", Material));
	MatFields.Add( new mset::MatFieldTexture("Cavity Map", "Cavity Map", Material, ImportUI));
	MatFields.Add( new mset::MatFieldEnum("Channel;cavity", "Cavity Channel", Material));
	MatFields.Add( new mset::MatFieldFloat("Diffuse Cavity", "Diffuse Cavity", Material));
	MatFields.Add( new mset::MatFieldFloat("Specular Cavity", "Specular Cavity", Material));
}

void USROcclusionMap::LateSetup ( UMaterial* Material )
{
	if (!WasImportedDirectly && ((mset::MatFieldTexture*)GetField("Cavity Map"))->getTexture() != NULL)
	{
		for (int32 j = 0; j < FunctionCallExpression->FunctionInputs.Num(); ++j)
		{
			FFunctionExpressionInput* Input = &(FunctionCallExpression->FunctionInputs[j]);
			if (Material->Specular.Expression != NULL && Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Specular" ) ))
			{
				Input->Input.Connect( Material->Specular.OutputIndex, Material->Specular.Expression );
			}
			if (Material->BaseColor.Expression != NULL && Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Base Color" ) ))
			{
				Input->Input.Connect( Material->BaseColor.OutputIndex, Material->BaseColor.Expression );
			}
		}
		for (int32 j = 0; j < FunctionCallExpression->FunctionOutputs.Num(); ++j)
		{
			FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[j]);
			if (Output->ExpressionOutput->OutputName.ToString().Equals( TEXT( "Generated Specular" ) ))
			{
				Material->Specular.Connect( j, FunctionCallExpression );
			}
			if (Output->ExpressionOutput->OutputName.ToString().Equals( TEXT( "Generated Base Color" ) ))
			{
				Material->BaseColor.Connect( j, FunctionCallExpression );
			}
		}
	}
}

bool USROcclusionMap::TryDirectImport ( UMaterial* Material )
{
	if (((mset::MatFieldFloat*)GetField("Occlusion"))->getValue() == 1 &&
		((mset::MatFieldEnum*)GetField("UV Set"))->getValue() == 0 &&
		((mset::MatFieldEnum*)GetField("Vertex Channel"))->getValue() == 0 &&
		((mset::MatFieldTexture*)GetField("Cavity Map"))->getTexture() == NULL)
	{
		if (((mset::MatFieldTexture*)GetField("Occlusion Map"))->getTexture() != NULL)
		{
			Material->AmbientOcclusion.Expression = ((mset::MatFieldTexture*)GetField("Occlusion Map"))->generateExpression();
		}
		return true;
	}
	return false;
}