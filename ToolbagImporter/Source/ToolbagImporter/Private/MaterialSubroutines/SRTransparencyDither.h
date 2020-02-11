/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "SRTransparency.h"
#include "SRTransparencyDither.generated.h"

UCLASS()
class USRTransparencyDither : public USRTransparency
{
	GENERATED_BODY()
public:

	USRTransparencyDither( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();

	void SetupMaterial ( UMaterial* Material ) override;
	void LateSetup( UMaterial* Material ) override;
};