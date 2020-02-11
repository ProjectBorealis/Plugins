/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "MaterialFunctionSubroutine.h"
#include "ToolbagImporterPrivatePCH.h"
#include "../mset/MatField.h"
#include <Materials/MaterialExpressionFunctionOutput.h>
#include <Materials/MaterialExpressionFunctionInput.h>
#include <Materials/MaterialExpressionAdd.h>
#include "../UI/ToolbagImportUI.h"
#include <locale>

UMaterialFunctionSubroutine::UMaterialFunctionSubroutine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString UMaterialFunctionSubroutine::CreateNiceName(FString Name)
{
	Name.RemoveFromStart("SR", ESearchCase::CaseSensitive);
	Name = FName::NameToDisplayString(Name, false);
	return Name;
}

void UMaterialFunctionSubroutine::ParseFields( ToolbagUtils::MaterialSubroutine* toolbagSubroutine, UMaterial* Material )
{
	//read the subroutine fields
	for (int32 i = 0; i < MatFields.Num(); ++i)
	{
		mset::MatField* MatField = MatFields[i];
		MatField->readValue( toolbagSubroutine );
	}
}

void UMaterialFunctionSubroutine::GenerateExpressions (UToolbagImportUI* ImportUI, UMaterial* Material)
{
	if (ImportUI->bDirectImport)
	{
		WasImportedDirectly = TryDirectImport( Material );
	}
	else
	{
		WasImportedDirectly = false;
	}
	if(!WasImportedDirectly)
	{
		FunctionCallExpression = NewObject<UMaterialExpressionMaterialFunctionCall>(Material);
		FunctionCallExpression->SetMaterialFunction( MaterialFunction ); //( NULL, NULL, MaterialFunction );
		Material->Expressions.Add( FunctionCallExpression );

		for (int32 j = 0; j < FunctionCallExpression->FunctionOutputs.Num(); ++j)
		{
			FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[j]);
			ConnectOutput( Output->ExpressionOutput->OutputName.ToString(), FunctionCallExpression, j, Material );
		}
	}
}

void UMaterialFunctionSubroutine::ConnectMatFields ( UMaterial* Material )
{
	if (!WasImportedDirectly)
	{
		if (FunctionCallExpression == NULL)
			return;
		for (int32 i = 0; i < MatFields.Num(); ++i)
		{
			mset::MatField* MatField = MatFields[i];
			for (int32 j = 0; j < FunctionCallExpression->FunctionInputs.Num(); ++j)
			{
				FFunctionExpressionInput* Input = &(FunctionCallExpression->FunctionInputs[j]);
				MatField->connectTo( Input->ExpressionInput->InputName.ToString(), &(Input->Input) );
			}
		}
	}
}

mset::MatField* UMaterialFunctionSubroutine::GetField(const char* name)
{
	for(int32 i = 0; i < MatFields.Num(); ++i)
	{
		if (std::strcmp( MatFields[i]->getName(), name ) == 0)
			return MatFields[i];
	}
	return NULL;
}

void UMaterialFunctionSubroutine::ConnectOutput ( FString OutputName, UMaterialExpression* Expression, int32 Index, UMaterial* Material )
{
	if(OutputName == "BaseColor")
		Material->BaseColor.Connect(Index, Expression);
	else if(OutputName == "Metallic")
		Material->Metallic.Connect(Index, Expression);
	else if(OutputName == "Specular")
		Material->Specular.Connect(Index, Expression);
	else if(OutputName == "Roughness")
		Material->Roughness.Connect(Index, Expression);
	else if(OutputName == "Normal")
		Material->Normal.Connect(Index, Expression);
	else if(OutputName == "EmissiveColor")
	{
		if(Material->EmissiveColor.Expression == NULL)
		{
			Material->EmissiveColor.Connect(Index, Expression);
		}
		else
		{
			// Emissive is used for some lighting effects, so we simply add it together
			UMaterialExpressionAdd* AddExpression = NewObject<UMaterialExpressionAdd>( Material );
			Material->Expressions.Add( AddExpression );
			AddExpression->A.Connect( Material->EmissiveColor.OutputIndex, Material->EmissiveColor.Expression);
			AddExpression->B.Connect( Index, Expression);
			Material->EmissiveColor.Expression = AddExpression;

		}
	}
	else if(OutputName == "Opacity")
		Material->Opacity.Connect(Index, Expression);
	else if(OutputName == "OpacityMask")
		Material->OpacityMask.Connect(Index, Expression);
	else if(OutputName == "WorldPositionOffset")
		Material->WorldPositionOffset.Connect(Index, Expression);
	else if(OutputName == "WorldDisplacement")
		Material->WorldDisplacement.Connect(Index, Expression);
	else if(OutputName == "TessellationMultiplier")
		Material->TessellationMultiplier.Connect(Index, Expression);
	else if(OutputName == "SubsurfaceColor")
		Material->SubsurfaceColor.Connect(Index, Expression);
	else if(OutputName == "ClearCoat")
		Material->ClearCoat.Connect(Index, Expression);
	else if(OutputName == "ClearCoatRoughness")
		Material->ClearCoatRoughness.Connect(Index, Expression);
	else if(OutputName == "AmbientOcclusion")
		Material->AmbientOcclusion.Connect(Index, Expression);
	else if(OutputName == "Refraction")
		Material->Refraction.Connect(Index, Expression);
}


void UMaterialFunctionSubroutine::Cleanup ()
{
	for ( int32 i = 0; i < MatFields.Num(); ++i )
	{
		delete(MatFields[i]);
	}
}