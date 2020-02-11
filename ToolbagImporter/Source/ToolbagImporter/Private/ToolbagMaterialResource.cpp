/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagMaterialResource.h"
#include "ToolbagImporterPrivatePCH.h"
#include <Runtime/AssetRegistry/Public/AssetRegistryModule.h>
#include <Runtime/Engine/Classes/Materials/MaterialExpressionCustom.h>
#include "MaterialSubroutines/SRAlbedoMap.h"
#include "MaterialSubroutines/MaterialFunctionSubroutine.h"
#include "MaterialSubroutines/SRReflectivityMetalnessMap.h"
#include "MaterialSubroutines/SRReflectivitySpecularMap.h"
#include "MaterialSubroutines/SRSurfaceNormalMap.h"
#include "MaterialSubroutines/SRSurfaceParallaxMap.h"
#include "MaterialSubroutines/SRMicrosurfaceGlossMap.h"
#include "MaterialSubroutines/SRTransparencyAlphaTest.h"
#include "MaterialSubroutines/SRTransparencyDither.h"
#include "MaterialSubroutines/SRTransparencyAdd.h"
#include "MaterialSubroutines/SRDiffusionMicrofiber.h"
#include "MaterialSubroutines/SRDiffusionSkin.h"
#include "MaterialSubroutines/SRDiffusionScatter.h"
#include "MaterialSubroutines/SRSubdivisionFlat.h"
#include "MaterialSubroutines/SRSubdivisionPN.h"
#include "MaterialSubroutines/SRDisplacementHeight.h"
#include "MaterialSubroutines/SROcclusionMap.h"
#include "MaterialSubroutines/SREmissiveHeat.h"
#include "MaterialSubroutines/SREmissiveMap.h"
#include "ToolbagUtils/Structs.h"
#include <vector>
#include <algorithm>
#include <ObjectTools.h>
#include <PackageTools.h>
#include <Developer/AssetTools/Public/AssetToolsModule.h>
#include "UI/ToolbagImportUI.h"
#include "ToolbagUtils.h"
#define LOCTEXT_NAMESPACE "ToolbagImportFactory"

UMaterial* ToolbagMaterialResource::Read( ToolbagUtils::Material* ToolbagMaterial, FString TargetPath, UToolbagImportUI* ImportUI, FString& OutKey )
{
	FString MaterialName = FString( ToolbagMaterial->name );
	OutKey = FString( ToolbagMaterial->name );
	ImportUI->SlowTask->FrameMessage = FText::Format( LOCTEXT( "Toolbag_ImportingMaterial", "Importing Materials... {0}" ), FText::FromString( MaterialName ) );
	MaterialName = ObjectTools::SanitizeObjectName(MaterialName);
	UPackage* MaterialPackage = ToolbagUtils::CreatePackageForAsset<UMaterial>( TargetPath, MaterialName );
	UMaterial* Material = FindObject<UMaterial>(MaterialPackage, *MaterialName);
	if(Material == NULL)
	{
		// create an unreal material asset
		auto MaterialFactory = NewObject<UMaterialFactoryNew>();
		Material = (UMaterial*)MaterialFactory->FactoryCreateNew(
			UMaterial::StaticClass(), MaterialPackage, FName( *MaterialName ), RF_Standalone | RF_Public, NULL, GWarn );
		FAssetRegistryModule::AssetCreated( Material );
	}
	else
	{
		Material->BaseColor.Expression = NULL;
		Material->Metallic.Expression = NULL;
		Material->Specular.Expression = NULL;
		Material->Roughness.Expression = NULL;
		Material->Normal.Expression = NULL;
		Material->Opacity.Expression = NULL;
		Material->OpacityMask.Expression = NULL;
		Material->EmissiveColor.Expression = NULL;
		Material->WorldPositionOffset.Expression = NULL;
		Material->WorldDisplacement.Expression = NULL;
		Material->TessellationMultiplier.Expression = NULL;
		Material->AmbientOcclusion.Expression = NULL;

		Material->BaseColor.OutputIndex = 0;
		Material->Metallic.OutputIndex = 0;
		Material->Specular.OutputIndex = 0;
		Material->Roughness.OutputIndex = 0;
		Material->Normal.OutputIndex = 0;
		Material->Opacity.OutputIndex = 0;
		Material->OpacityMask.OutputIndex = 0;
		Material->EmissiveColor.OutputIndex = 0;
		Material->WorldPositionOffset.OutputIndex = 0;
		Material->WorldDisplacement.OutputIndex = 0;
		Material->TessellationMultiplier.OutputIndex = 0;
		Material->AmbientOcclusion.OutputIndex = 0;

		for(int i = Material->Expressions.Num() - 1; i >= 0; --i)
		{
			Material->Expressions.RemoveAt(i);
		}
	}


	ParseString( ToolbagMaterial, Material, ImportUI );

	Material->PreEditChange( NULL );
	Material->PostEditChange();
	Material->ForceRecompileForRendering();
	MaterialPackage->SetDirtyFlag( true );
	return Material;
}

void ToolbagMaterialResource::FormatMaterial( UMaterial* Material )
{
	TArray<FExpressionInput*> MaterialInputs;

	if (Material->BaseColor.Expression != NULL)
		MaterialInputs.Add( &Material->BaseColor );

	if (Material->Metallic.Expression != NULL)
		MaterialInputs.Add( &Material->Metallic );

	if (Material->Specular.Expression != NULL)
		MaterialInputs.Add( &Material->Specular );

	if (Material->Roughness.Expression != NULL)
		MaterialInputs.Add( &Material->Roughness );

	if (Material->Normal.Expression != NULL)
		MaterialInputs.Add( &Material->Normal );

	if (Material->Opacity.Expression != NULL)
		MaterialInputs.Add( &Material->Opacity );

	if (Material->OpacityMask.Expression != NULL)
		MaterialInputs.Add( &Material->OpacityMask );

	if (Material->EmissiveColor.Expression != NULL)
		MaterialInputs.Add( &Material->EmissiveColor );

	if (Material->WorldPositionOffset.Expression != NULL)
		MaterialInputs.Add( &Material->WorldPositionOffset );

	if (Material->WorldDisplacement.Expression != NULL)
		MaterialInputs.Add( &Material->WorldDisplacement );

	if (Material->TessellationMultiplier.Expression != NULL)
		MaterialInputs.Add( &Material->TessellationMultiplier );

	if (Material->AmbientOcclusion.Expression != NULL)
		MaterialInputs.Add( &Material->AmbientOcclusion );

	StartFormatRecursive( MaterialInputs, Material );
}

int32 ToolbagMaterialResource::GetRequiredHeightRecursive( UMaterialExpression* Expression, TArray<UMaterialExpression*>& AlreadyTouchedExpressions )
{
	if (AlreadyTouchedExpressions.Contains( Expression ))
		return 0;
	AlreadyTouchedExpressions.Add( Expression );
	int32 NextOffset = 0;
	int32 CurrentYOffset = 0;
	for(int32 i = Expression->GetInputs().Num()-1; i >= 0; --i)
	{
		UMaterialExpression* ChildExpression = Expression->GetInput(i)->Expression;
		if(ChildExpression != NULL)
		{
			CurrentYOffset += GetRequiredHeightRecursive(ChildExpression, AlreadyTouchedExpressions) ;
		}
	}
	return FMath::Max(100, CurrentYOffset) + 20;
}

void ToolbagMaterialResource::FormatRecursive( UMaterialExpression* Expression, int32 PosX, int32 PosY, TArray<UMaterialExpression*>& AlreadyTouchedExpressions2 )
{
	if (AlreadyTouchedExpressions2.Contains( Expression ))
		return;
	if (Expression->MaterialExpressionEditorX <= PosX+100)
		return;
	AlreadyTouchedExpressions2.Add( Expression );
	Expression->MaterialExpressionEditorX = PosX;
	Expression->MaterialExpressionEditorY = PosY;
	int32 HalfYOffset = 0;
	TArray<UMaterialExpression*> AlreadyTouchedExpressions;
	for(int32 i = Expression->GetInputs().Num()-1; i >= 0; --i)
	{
		UMaterialExpression* ChildExpression = Expression->GetInput(i)->Expression;
		if(ChildExpression != NULL)
		{
			HalfYOffset += GetRequiredHeightRecursive(ChildExpression, AlreadyTouchedExpressions)/2;
		}
	}
	//Run twice to center nodes
	AlreadyTouchedExpressions.Empty();
	int32 CurrentYOffset = 0;
	for(int32 i = Expression->GetInputs().Num()-1; i >= 0; --i)
	{
		UMaterialExpression* ChildExpression = Expression->GetInput(i)->Expression;
		if(ChildExpression != NULL)
		{
			FormatRecursive(ChildExpression, PosX-400, PosY + HalfYOffset - CurrentYOffset - ChildExpression->GetHeight(), AlreadyTouchedExpressions2);
			CurrentYOffset += GetRequiredHeightRecursive(ChildExpression, AlreadyTouchedExpressions);
		}
	}
}

void ToolbagMaterialResource::StartFormatRecursive ( TArray<FExpressionInput*>& MaterialInputs, UMaterial* Material )
{
	int32 HalfYOffset = 0;
	TArray<UMaterialExpression*> AlreadyTouchedExpressions;
	for(int32 i = MaterialInputs.Num()-1; i >= 0; --i)
	{
		UMaterialExpression* ChildExpression = MaterialInputs[i]->Expression;
		if(ChildExpression != NULL)
		{
			HalfYOffset += GetRequiredHeightRecursive(ChildExpression, AlreadyTouchedExpressions)/2;
		}
	}
	//Run twice to center nodes
	AlreadyTouchedExpressions.Empty();
	int32 CurrentYOffset = 0;
	for(int32 i = MaterialInputs.Num()-1; i >= 0; --i)
	{
		UMaterialExpression* ChildExpression = MaterialInputs[i]->Expression;
		if(ChildExpression != NULL)
		{
			TArray<UMaterialExpression*> AlreadyTouchedExpressions2;
			FormatRecursive(ChildExpression, -400, HalfYOffset - CurrentYOffset - ChildExpression->GetHeight(), AlreadyTouchedExpressions2);
			CurrentYOffset += GetRequiredHeightRecursive(ChildExpression, AlreadyTouchedExpressions);
		}
	}
}

bool ToolbagMaterialResource::ParseString( ToolbagUtils::Material* ToolbagMaterial, UMaterial* Material, UToolbagImportUI* ImportUI )
{

	Material->Modify();
	std::vector<UMaterialFunctionSubroutine*> Subroutines;
	UMaterialFunctionSubroutine* Subroutine = 0;
	for( int i = 0; i < ToolbagUtils::Material::SUBROUTINE_MAX_ENUM; ++i )
	{
		ToolbagUtils::MaterialSubroutine* ToolbagSubroutine = ToolbagMaterial->Subroutines[i];
		if(ToolbagSubroutine == NULL)
			continue;
		if (0 == strcmp( ToolbagSubroutine->name, USRAlbedoMap::GetName() ))
			Subroutine = NewObject<USRAlbedoMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRReflectivityMetalnessMap::GetName() ))
			Subroutine = NewObject<USRReflectivityMetalnessMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRReflectivitySpecularMap::GetName() ))
			Subroutine = NewObject<USRReflectivitySpecularMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRSurfaceNormalMap::GetName() ))
			Subroutine = NewObject<USRSurfaceNormalMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRMicrosurfaceGlossMap::GetName() ))
			Subroutine = NewObject<USRMicrosurfaceGlossMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRSurfaceParallaxMap::GetName() ))
			Subroutine = NewObject<USRSurfaceParallaxMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRTransparencyAlphaTest::GetName() ))
			Subroutine = NewObject<USRTransparencyAlphaTest>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRTransparencyDither::GetName() ))
			Subroutine = NewObject<USRTransparencyDither>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRTransparencyAdd::GetName() ))
			Subroutine = NewObject<USRTransparencyAdd>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRSubdivisionFlat::GetName() ))
			Subroutine = NewObject<USRSubdivisionFlat>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRSubdivisionPN::GetName() ))
			Subroutine = NewObject<USRSubdivisionPN>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRDisplacementHeight::GetName() ))
			Subroutine = NewObject<USRDisplacementHeight>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRDiffusionMicrofiber::GetName() ))
			Subroutine = NewObject<USRDiffusionMicrofiber>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRDiffusionSkin::GetName() ))
			Subroutine = NewObject<USRDiffusionSkin>();
		else if (0 == strcmp( ToolbagSubroutine->name, USRDiffusionScatter::GetName() ))
			Subroutine = NewObject<USRDiffusionScatter>();
		else if (0 == strcmp( ToolbagSubroutine->name, USROcclusionMap::GetName() ))
			Subroutine = NewObject<USROcclusionMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USREmissiveMap::GetName() ))
			Subroutine = NewObject<USREmissiveMap>();
		else if (0 == strcmp( ToolbagSubroutine->name, USREmissiveHeat::GetName() ))
			Subroutine = NewObject<USREmissiveHeat>();
		else if (
			0 == strcmp( ToolbagSubroutine->name, "SRDiffusionLambertian" ) || 
			0 == strcmp( ToolbagSubroutine->name, "SRReflectionGGX" ) || 
			0 == strcmp( ToolbagSubroutine->name, "SRReflectionBlinnPhong" ) || 
			0 == strcmp( ToolbagSubroutine->name, "SRMerge" )
			)
			continue;
		else
		{
			if(!ImportUI->bSuppressWarnings)
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("ToolbagImport_NotSupported", "The Material Subroutine {0} is currently not supported for Unreal import."), FText::FromString(UMaterialFunctionSubroutine::CreateNiceName(FString(ToolbagSubroutine->name)))));
			}
			UE_LOG( LogToolbag, Warning, TEXT( "Unable to find Implementation of MaterialSubroutine %s." ), *FString( ToolbagSubroutine->name ) );
		}
		if(Subroutine != 0)
		{
			Subroutine->SetupFields(ImportUI, Material);
			Subroutine->ParseFields(ToolbagSubroutine, Material);
			Subroutines.push_back( Subroutine );
		}
		Subroutine = 0;
	}

	std::sort( Subroutines.begin(), Subroutines.end(), UMaterialFunctionSubroutine::SortCompare );

	for(auto it = Subroutines.begin(); it != Subroutines.end(); ++it)
	{
		(*it)->SetupMaterial(Material);
	}
	for(auto it = Subroutines.begin(); it != Subroutines.end(); ++it)
	{
		(*it)->GenerateExpressions(ImportUI, Material);
	}
	for(auto it = Subroutines.begin(); it != Subroutines.end(); ++it)
	{
		(*it)->ConnectMatFields(Material);
	}
	for(auto it = Subroutines.begin(); it != Subroutines.end(); ++it)
	{
		(*it)->LateSetup(Material);
	}
	for(auto it = Subroutines.begin(); it != Subroutines.end(); ++it)
	{
		(*it)->Cleanup();
		(*it)->ConditionalBeginDestroy();;
	}
	FormatMaterial(Material);
	Material->MarkPackageDirty();
	return true;
}

#undef LOCTEXT_NAMESPACE