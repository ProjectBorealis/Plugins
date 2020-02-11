/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "../UI/ToolbagImportUI.h"
#include "SRReflectivitySpecularMap.generated.h"

UCLASS()
class USRReflectivitySpecularMap : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:
	USRReflectivitySpecularMap( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void LateSetup ( UMaterial* Material ) override;
	const int GetPrio() const override
	{
		return 100;
	}; 
protected:
	bool TryDirectImport(UMaterial* Material) override;
private:
	ToolbagMetalnessConversion MetalnessConversion;
};