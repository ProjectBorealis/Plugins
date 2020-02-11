/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagImportUI.generated.h"

enum ToolbagMetalnessConversion
{
	INSULATOR,
	METAL,
	CONVERT,
	ASK,
	METALNESS_CONVERSION_MAX
};

enum ToolbagImportMode
{
	INTO_SCENE,
	BLUEPRINT,
	ONLY_MATERIALS,
	IMPORT_MODE_MAX
};

UCLASS(config=EditorPerProjectUserSettings, AutoExpandCategories=(FTransform), HideCategories=Object, MinimalAPI)
class UToolbagImportUI : public UObject
{
	GENERATED_BODY()
public:

	UToolbagImportUI( const FObjectInitializer& ObjectInitializer );

	void ReadConfig();
	void WriteConfig();

	bool bSuppressWarnings;

	bool bImportLights;

	bool bImportCameras;

	bool bImportSky;

	ToolbagImportMode ImportMode;

	bool bOnlyMaterials;

	UPackage* BlueprintPackage;

	FString MeshesPath;
	FString AbsoluteMeshesPath;

	FString MaterialsPath;
	FString AbsoluteMaterialsPath;

	FString TexturesPath;
	FString AbsoluteTexturesPath;

	ToolbagMetalnessConversion MetalnessConversionOption;
	bool bDirectImport;
	FScopedSlowTask* SlowTask;

	ToolbagUtils::Scene* TBScene;
	float SceneScaleFactor;
	float ImportScaleFactor;
};
