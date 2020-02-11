/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagImportUI.h"
#include "ToolbagImporterPrivatePCH.h"

#define TOOLBAG_SETTINGS TEXT("ToolbagImporter")

UToolbagImportUI::UToolbagImportUI( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer ),
	bSuppressWarnings( false ),
	bImportLights( false ),
	bImportCameras( false ),
	bImportSky( false ),
	ImportMode( BLUEPRINT ),
	BlueprintPackage( NULL ),
	MeshesPath( "" ),
	MaterialsPath( "" ),
	TexturesPath( "" ),
	MetalnessConversionOption( ASK ),
	bDirectImport( true ),
	SlowTask(NULL),
	TBScene(0),
	SceneScaleFactor(1),
	ImportScaleFactor(1)
{
	ReadConfig ();
};

void UToolbagImportUI::ReadConfig ()
{
	{
		int32 E = static_cast<int32>(ImportMode);
		GConfig->GetInt(TOOLBAG_SETTINGS, TEXT("ImportMode"), E, GEngineIni);
		ImportMode = static_cast<ToolbagImportMode>(E);
	}
	GConfig->GetBool(TOOLBAG_SETTINGS, TEXT("SurpressWarnings"), bSuppressWarnings, GEngineIni);
	GConfig->GetBool(TOOLBAG_SETTINGS, TEXT("ImportLights"), bImportLights, GEngineIni);
	GConfig->GetBool(TOOLBAG_SETTINGS, TEXT("ImportCameras"), bImportCameras, GEngineIni);
	GConfig->GetBool(TOOLBAG_SETTINGS, TEXT("ImportSky"), bImportSky, GEngineIni);
	GConfig->GetString(TOOLBAG_SETTINGS, TEXT("MeshesPath"), MeshesPath, GEngineIni);
	GConfig->GetString(TOOLBAG_SETTINGS, TEXT("MaterialsPath"), MaterialsPath, GEngineIni);
	GConfig->GetString(TOOLBAG_SETTINGS, TEXT("TexturesPath"), TexturesPath, GEngineIni);
	{
		int32 E = static_cast<int32>(MetalnessConversionOption);
		GConfig->GetInt(TOOLBAG_SETTINGS, TEXT("MetalnessConversionOption"), E, GEngineIni);
		MetalnessConversionOption = static_cast<ToolbagMetalnessConversion>(E);
	}
	GConfig->GetBool(TOOLBAG_SETTINGS, TEXT("DirectImport"), bDirectImport, GEngineIni);
}

void UToolbagImportUI::WriteConfig()
{
	{
		int32 E = static_cast<int32>(ImportMode);
		GConfig->SetInt(TOOLBAG_SETTINGS, TEXT("ImportMode"), E, GEngineIni);
	}
	GConfig->SetBool(TOOLBAG_SETTINGS, TEXT("SurpressWarnings"), bSuppressWarnings, GEngineIni);
	GConfig->SetBool(TOOLBAG_SETTINGS, TEXT("ImportLights"), bImportLights, GEngineIni);
	GConfig->SetBool(TOOLBAG_SETTINGS, TEXT("ImportCameras"), bImportCameras, GEngineIni);
	GConfig->SetBool(TOOLBAG_SETTINGS, TEXT("ImportSky"), bImportSky, GEngineIni);
	GConfig->SetString(TOOLBAG_SETTINGS, TEXT("MeshesPath"), &MeshesPath[0], GEngineIni);
	GConfig->SetString(TOOLBAG_SETTINGS, TEXT("MaterialsPath"), &MaterialsPath[0], GEngineIni);
	GConfig->SetString(TOOLBAG_SETTINGS, TEXT("TexturesPath"), &TexturesPath[0], GEngineIni);
	int32 E = static_cast<int32>(MetalnessConversionOption);
	GConfig->SetInt(TOOLBAG_SETTINGS, TEXT("MetalnessConversionOption"), E, GEngineIni);
	GConfig->SetBool(TOOLBAG_SETTINGS, TEXT("DirectImport"), bDirectImport, GEngineIni);
}