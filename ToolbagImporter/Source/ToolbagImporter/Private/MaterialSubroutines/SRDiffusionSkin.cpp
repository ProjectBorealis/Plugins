/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRDiffusionSkin.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Runtime/AssetRegistry/Public/AssetRegistryModule.h>
#include "../UI/ToolbagImportUI.h"
#include "../ToolbagUtils.h"



USRDiffusionSkin::USRDiffusionSkin( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRDiffusionSkin"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}};

const char* USRDiffusionSkin::GetName ()
{
	return "SRDiffusionSkin";
}

void USRDiffusionSkin::SetupMaterial ( UMaterial* Material )
{
	USRDiffusionMicrofiber::SetupMaterial( Material );
	Material->SetShadingModel( EMaterialShadingModel::MSM_SubsurfaceProfile );
	SubsurfaceProfileName = FName( *(Material->GetName() + TEXT( " Subsurface Profile" )) );
}

void USRDiffusionSkin::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	USRDiffusionMicrofiber::SetupFields( ImportUI, Material );
	MatFields.Add( new mset::MatFieldTexture( "Subdermis Map", "Subdermis Map", Material, ImportUI ) );
	MatFields.Add( new mset::MatFieldFloat( "Subdermis Scatter", "Subdermis Scatter", Material ) );
	MatFields.Add( new mset::MatFieldColor( "Subdermis Color", "Subdermis Color", Material ) );
	UPackage* SubsurfaceProfilePackage = ToolbagUtils::CreatePackageForAsset<USubsurfaceProfile>( ImportUI->AbsoluteMaterialsPath, SubsurfaceProfileName.ToString());
	SubsurfaceProfile = NewObject<USubsurfaceProfile>(SubsurfaceProfilePackage, USubsurfaceProfile::StaticClass(), SubsurfaceProfileName, RF_Standalone | RF_Public );
	FAssetRegistryModule::AssetCreated(SubsurfaceProfile);
	// Set the dirty flag so this package will get saved later
	ImportUI->BlueprintPackage->SetDirtyFlag(true);
}

void USRDiffusionSkin::ConnectMatFields ( UMaterial* Material )
{
	USRDiffusionMicrofiber::ConnectMatFields( Material );
	for (int32 i = 0; i < MatFields.Num(); ++i)
	{
		mset::MatField* MatField = MatFields[i];
		if(0 == strcmp(MatField->getName(), "Subdermis Scatter"))
		{
			SubsurfaceProfile->Settings.ScatterRadius = ((mset::MatFieldFloat*)MatField)->getValue() * 50;
		}
		else if(0 == strcmp(MatField->getName(), "Subdermis Color"))
		{
			SubsurfaceProfile->Settings.SubsurfaceColor = ((mset::MatFieldColor*)MatField)->getValue() * FVector(0.25, 0.2667, 0.4);
			SubsurfaceProfile->Settings.FalloffColor = ((mset::MatFieldColor*)MatField)->getValue();
		}
	}
	Material->SubsurfaceProfile = SubsurfaceProfile;
}