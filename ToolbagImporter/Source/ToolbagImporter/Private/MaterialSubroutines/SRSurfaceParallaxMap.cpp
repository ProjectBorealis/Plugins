/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRSurfaceParallaxMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USRSurfaceParallaxMap::USRSurfaceParallaxMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRSurfaceParallaxMap"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}};

const char* USRSurfaceParallaxMap::GetName ()
{
	return "SRSurfaceParallaxMap";
}

void USRSurfaceParallaxMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	USRSurfaceNormalMap::SetupFields( ImportUI, Material );
	MatFields.Add(new mset::MatFieldTexture("Height Map", "Parallax Map", Material, ImportUI));
	MatFields.Add(new mset::MatFieldEnum("Channel", "Parallax Channel", Material));
	MatFields.Add(new mset::MatFieldFloat("Depth", "Parallax Depth", Material));
	MatFields.Add(new mset::MatFieldFloat("Depth Center", "Parallax Depth Center", Material));
}

void USRSurfaceParallaxMap::LateSetup( UMaterial* Material )
{
	USRSurfaceNormalMap::LateSetup( Material );
	int32 BumpOffsetIndex = -1;
	for (int32 j = 0; j < FunctionCallExpression->FunctionOutputs.Num(); ++j)
	{
		FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[j]);
		if(Output->ExpressionOutput->OutputName.ToString().Equals("DefinedByCode"))
		{
			BumpOffsetIndex = j;
			break;
		}
	}
	if (BumpOffsetIndex == -1)
		return;
	UMaterialExpressionTextureSampleParameter2D* TextureExpression;
	UMaterialExpression* Expression = 0;
	for(int32 i = 0; i < Material->Expressions.Num(); ++i)
	{
		Expression = Material->Expressions[i];
		if(Expression->GetClass() == UMaterialExpressionTextureSampleParameter2D::StaticClass())
		{
			TextureExpression = (UMaterialExpressionTextureSampleParameter2D*) Expression;
			if( 0 != TextureExpression->ParameterName.Compare(FName("Parallax Map")) &&
				TextureExpression->Coordinates.Expression == NULL)
			{
				TextureExpression->Coordinates.Connect( BumpOffsetIndex, FunctionCallExpression );
			}
		}
	}
}

bool USRSurfaceParallaxMap::TryDirectImport ( UMaterial* Material )
{
	return false;
}