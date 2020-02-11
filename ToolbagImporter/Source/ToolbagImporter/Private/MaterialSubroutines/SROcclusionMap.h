/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "SROcclusionMap.generated.h"


UCLASS()
class USROcclusionMap : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:
	USROcclusionMap( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void LateSetup ( UMaterial* Material ) override;
	const int GetPrio() const override
	{
		return 200;
	}; 
protected:
	bool TryDirectImport(UMaterial* Material) override;
};
