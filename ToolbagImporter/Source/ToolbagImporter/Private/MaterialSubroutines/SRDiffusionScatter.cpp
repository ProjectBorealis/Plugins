/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRDiffusionScatter.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Runtime/AssetRegistry/Public/AssetRegistryModule.h>
#include "../UI/ToolbagImportUI.h"
#include "../ToolbagUtils.h"



USRDiffusionScatter::USRDiffusionScatter( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRDiffusionSkin"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}};

const char* USRDiffusionScatter::GetName ()
{
	return "SRDiffusionScatter";
}

void USRDiffusionScatter::SetupMaterial ( UMaterial* Material )
{
	USRDiffusionMicrofiber::SetupMaterial( Material );
	Material->SetShadingModel( EMaterialShadingModel::MSM_SubsurfaceProfile );
	SubsurfaceProfileName = FName( *(Material->GetName() + TEXT( " Subsurface Profile" )) );
}

void USRDiffusionScatter::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	USRDiffusionMicrofiber::SetupFields( ImportUI, Material );
	MatFields.Add( new mset::MatFieldTexture( "Subdermis Map", "Subdermis Map", Material, ImportUI ) );
	MatFields.Add( new mset::MatFieldFloat( "Subdermis Scatter (mm)", "Subdermis Scatter", Material ) );
	MatFields.Add( new mset::MatFieldColor( "Subdermis Color", "Subdermis Color", Material ) );
	UPackage* SubsurfaceProfilePackage = ToolbagUtils::CreatePackageForAsset<USubsurfaceProfile>( ImportUI->AbsoluteMaterialsPath, SubsurfaceProfileName.ToString() );
	SubsurfaceProfile = NewObject<USubsurfaceProfile>(SubsurfaceProfilePackage, USubsurfaceProfile::StaticClass(), SubsurfaceProfileName, RF_Standalone | RF_Public );
	FAssetRegistryModule::AssetCreated(SubsurfaceProfile);
	ImportUI->BlueprintPackage->SetDirtyFlag(true);
}

void USRDiffusionScatter::ConnectMatFields ( UMaterial* Material )
{
	USRDiffusionMicrofiber::ConnectMatFields( Material );
	for (int32 i = 0; i < MatFields.Num(); ++i)
	{
		mset::MatField* MatField = MatFields[i];
		if(0 == strcmp(MatField->getName(), "Subdermis Scatter (mm)"))
		{
			SubsurfaceProfile->Settings.ScatterRadius = ((mset::MatFieldFloat*)MatField)->getValue() / 10;
		}
		else if(0 == strcmp(MatField->getName(), "Subdermis Color"))
		{
			SubsurfaceProfile->Settings.SubsurfaceColor = ((mset::MatFieldColor*)MatField)->getValue();
			SubsurfaceProfile->Settings.FalloffColor = ((mset::MatFieldColor*)MatField)->getValue();
		}
	}
	Material->SubsurfaceProfile = SubsurfaceProfile;
}