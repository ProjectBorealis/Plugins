/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "SRSubdivisionFlat.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldTexture.h"
#include "SRSubdivisionPN.generated.h"

UCLASS()
class USRSubdivisionPN : public USRSubdivisionFlat
{
	GENERATED_BODY()
public:

	USRSubdivisionPN( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupMaterial( UMaterial* Material ) override;
};
