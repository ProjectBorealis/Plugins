/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldTexture.h"
#include "SRDiffusionMicrofiber.generated.h"

UCLASS()
class USRDiffusionMicrofiber : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:
	USRDiffusionMicrofiber( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void LateSetup( UMaterial* Material ) override;
	const int GetPrio() const override
	{
		return 50;
	}; 
};