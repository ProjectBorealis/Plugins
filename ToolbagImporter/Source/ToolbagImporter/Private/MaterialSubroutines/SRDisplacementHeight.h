/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "SRDisplacementHeight.generated.h"

UCLASS()
class USRDisplacementHeight : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:
	USRDisplacementHeight( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void GenerateExpressions( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
};