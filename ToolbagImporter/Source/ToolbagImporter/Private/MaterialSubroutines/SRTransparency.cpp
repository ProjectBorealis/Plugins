/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRTransparency.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Materials/MaterialExpressionFunctionInput.h>
#include "../UI/ToolbagImportUI.h"
#include <Materials/MaterialExpressionComponentMask.h>


USRTransparency::USRTransparency( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRTransparency"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRTransparency::GetName ()
{
	return "SRTransparency";
}

void USRTransparency::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldBool("Use Albedo Alpha", "Use Albedo Alpha", Material));
	MatFields.Add(new mset::MatFieldTexture("Alpha Map", "Alpha Map", Material, ImportUI));
	MatFields.Add(new mset::MatFieldEnum("Channel", "Alpha Map Channel", Material));
	MatFields.Add(new mset::MatFieldFloat("Alpha", "Alpha Scale", Material));
}

void USRTransparency::ConnectMatFields ( UMaterial* Material )
{
	UMaterialFunctionSubroutine::ConnectMatFields( Material );
	UMaterialExpression* AlbedoMapExpression = NULL;
	for(int32 i = 0; i < Material->Expressions.Num(); ++i)
	{
		
		if (Material->Expressions[i]->GetClass() != UMaterialExpressionTextureSampleParameter2D::StaticClass())
			continue;
		UMaterialExpressionTextureSampleParameter2D* Tex2DExpression = (UMaterialExpressionTextureSampleParameter2D*)Material->Expressions[i];
		if(Tex2DExpression->ParameterName.IsEqual(FName("Albedo Map")))
		{
			AlbedoMapExpression = Material->Expressions[i];
			break;
		}
	}
	if (AlbedoMapExpression != NULL)
	{
		for (int32 j = 0; j < FunctionCallExpression->FunctionInputs.Num(); ++j)
		{
			FFunctionExpressionInput* Input = &(FunctionCallExpression->FunctionInputs[j]);
			if (Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Albedo RGB" ) ))
			{
				UMaterialExpressionComponentMask* maskExpression = NewObject<UMaterialExpressionComponentMask>(Material);
				maskExpression->R = 1;
				maskExpression->G = 1;
				maskExpression->B = 1;
				maskExpression->A = 0;
				Material->Expressions.Add( maskExpression );
				maskExpression->Input.Connect( 0, AlbedoMapExpression );
				Input->Input.Connect( 0, maskExpression );
			}
			if (Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Albedo A" ) ))
			{
				UMaterialExpressionComponentMask* maskExpression = NewObject<UMaterialExpressionComponentMask>(Material);
				maskExpression->R = 0;
				maskExpression->G = 0;
				maskExpression->B = 0;
				maskExpression->A = 1;
				Material->Expressions.Add( maskExpression );
				maskExpression->Input.Connect(4 , AlbedoMapExpression );
				Input->Input.Connect( 0, maskExpression );
			}
		}
	}
}
