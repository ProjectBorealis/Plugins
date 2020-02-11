/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRReflectivitySpecularMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Materials/MaterialExpressionFunctionOutput.h>
#include <Materials/MaterialExpressionFunctionInput.h>
#include "../UI/ToolbagImportUI.h"
#include <UI/ToolbagMetalnessConversionOptionsWindow.h>

USRReflectivitySpecularMap::USRReflectivitySpecularMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRReflectivitySpecularMap"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}};

const char* USRReflectivitySpecularMap::GetName ()
{
	return "SRReflectivitySpecularMap";
}

void USRReflectivitySpecularMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture("Specular Map", "Specular Map", Material, ImportUI));
	MatFields.Add(new mset::MatFieldEnum("Channel;specular", "Specular Channel", Material));
	MatFields.Add(new mset::MatFieldColor("Color", "Specular Color", Material));
	MatFields.Add(new mset::MatFieldFloat("Intensity", "Specular Intensity", Material));
	MetalnessConversion = ImportUI->MetalnessConversionOption;
}

void USRReflectivitySpecularMap::LateSetup ( UMaterial* Material )
{
	if (Material->Metallic.Expression == NULL)
	{
		if (MetalnessConversion == ASK)
		{
			MetalnessConversion = SToolbagMetalnessConversionOptionsWindow::ShowMetalnessOptionWindow( Material->GetName() );
		}
		if (MetalnessConversion == INSULATOR)
		{
			UMaterialExpressionScalarParameter* Expression = NewObject<UMaterialExpressionScalarParameter>( Material );
			Expression->DefaultValue = 0.f;
			Expression->ParameterName = FName( "Metalness" );
			Material->Expressions.Add( Expression );
			Material->Metallic.Connect( 0, Expression );
		}
		else if (MetalnessConversion == METAL)
		{
			UMaterialExpressionScalarParameter* Expression = NewObject<UMaterialExpressionScalarParameter>( Material );
			Expression->DefaultValue = 1.f;
			Expression->ParameterName = FName( "Metalness" );
			Material->Expressions.Add( Expression );
			Material->Metallic.Connect( 0, Expression );
		}
		else if (MetalnessConversion == CONVERT)
		{
			UMaterialExpressionScalarParameter* Expression = NewObject<UMaterialExpressionScalarParameter>( Material );
			Expression->DefaultValue = 0.2f;
			Expression->ParameterName = FName( "Metallic Threashold" );
			Material->Expressions.Add( Expression );

			for (int32 j = 0; j < FunctionCallExpression->FunctionInputs.Num(); ++j)
			{
				FFunctionExpressionInput* Input = &(FunctionCallExpression->FunctionInputs[j]);
				if (Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Metallic Threashold" ) ))
				{
					Input->Input.Connect( 0, Expression );
					Expression->DefaultValue = Input->ExpressionInput->PreviewValue.X;
				}
				if (Material->BaseColor.Expression != NULL && Input->ExpressionInput->InputName.ToString().Equals( TEXT( "Base Color" ) ))
				{
					Input->Input.Connect( Material->BaseColor.OutputIndex, Material->BaseColor.Expression );
				}
			}
			for (int32 j = 0; j < FunctionCallExpression->FunctionOutputs.Num(); ++j)
			{
				FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[j]);
				if (Output->ExpressionOutput->OutputName.ToString().Equals( TEXT( "Generated Metallic" ) ))
				{
					Material->Metallic.Connect( j, FunctionCallExpression );
				}
				if (Output->ExpressionOutput->OutputName.ToString().Equals( TEXT( "Generated Base Color" ) ))
				{
					Material->BaseColor.Connect( j, FunctionCallExpression );
				}
			}
		}
	}

}

bool USRReflectivitySpecularMap::TryDirectImport ( UMaterial* Material )
{
	if (((mset::MatFieldColor*)GetField("Color"))->getValue() == FLinearColor::White && 
		(MetalnessConversion == INSULATOR || MetalnessConversion == METAL))
	{
		if (((mset::MatFieldTexture*)GetField("Specular Map"))->getTexture() != NULL && ((mset::MatFieldFloat*)GetField("Intensity"))->getValue() == 1)
		{
			int32 Channel = ((mset::MatFieldEnum*)GetField( "Channel;specular" ))->getValue();
			Material->Specular.Connect(Channel < 4 ? Channel + 1 : 0, ((mset::MatFieldTexture*)GetField("Specular Map"))->generateExpression());
			return true;
		}
		if(((mset::MatFieldTexture*)GetField("Specular Map"))->getTexture() == NULL)
		{
			Material->Specular.Expression = ((mset::MatFieldFloat*)GetField("Intensity"))->generateExpression();
			return true;
		}
	}
	return false;
}