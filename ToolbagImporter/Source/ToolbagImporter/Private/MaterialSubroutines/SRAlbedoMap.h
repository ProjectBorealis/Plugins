/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "SRAlbedoMap.generated.h"

UCLASS()
class USRAlbedoMap : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()

public:
	USRAlbedoMap( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
protected:
	bool TryDirectImport(UMaterial* Material) override;
};
