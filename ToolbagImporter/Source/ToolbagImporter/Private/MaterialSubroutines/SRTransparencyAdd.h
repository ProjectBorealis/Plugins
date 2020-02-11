/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "SRTransparency.h"
#include "SRTransparencyAdd.generated.h"

UCLASS()
class USRTransparencyAdd : public USRTransparency
{
	GENERATED_BODY()
public:

	USRTransparencyAdd( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();

	void SetupMaterial ( UMaterial* Material ) override;
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void LateSetup( UMaterial* Material ) override;
};
