/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.h"
#include "SRSubdivisionFlat.generated.h"

UCLASS()
class USRSubdivisionFlat : public UMaterialFunctionSubroutine
{
	GENERATED_BODY()
public:
	USRSubdivisionFlat( const FObjectInitializer& ObjectInitializer );

	static const char* GetName ();
	void SetupMaterial( UMaterial* Material ) override;
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
protected:
	bool TryDirectImport(UMaterial* Material) override;
};