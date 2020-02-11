/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "SRTransparency.generated.h"

UCLASS()
class USRTransparency : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:

	USRTransparency( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void ConnectMatFields( UMaterial* Material ) override;
	const int GetPrio() const override
	{
		return 500;
	}; 
};