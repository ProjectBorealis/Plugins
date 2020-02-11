/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "SREmissiveHeat.generated.h"

UCLASS()
class USREmissiveHeat : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:
	USREmissiveHeat( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
};
