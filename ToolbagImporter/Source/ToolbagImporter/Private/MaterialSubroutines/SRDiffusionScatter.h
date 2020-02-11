/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "SRDiffusionMicrofiber.h"
#include "SRDiffusionScatter.generated.h"
UCLASS()
class USRDiffusionScatter : public USRDiffusionMicrofiber
{
	GENERATED_BODY()
public:

	USRDiffusionScatter( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupMaterial ( UMaterial* Material ) override;
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void ConnectMatFields ( UMaterial* Material ) override;
protected:
	FName SubsurfaceProfileName;
	USubsurfaceProfile* SubsurfaceProfile;
};