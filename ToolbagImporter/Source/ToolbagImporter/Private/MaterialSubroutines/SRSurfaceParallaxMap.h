/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "SRSurfaceNormalMap.h"
#include "SRSurfaceParallaxMap.generated.h"

UCLASS()
class USRSurfaceParallaxMap : public USRSurfaceNormalMap
{
	GENERATED_BODY()
public:
	USRSurfaceParallaxMap( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void LateSetup( UMaterial* Material ) override;
protected:
	bool TryDirectImport(UMaterial* Material) override;
};