/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "SRDiffusionMicrofiber.h"
#include "SRDiffusionSkin.generated.h"
UCLASS()
class USRDiffusionSkin : public USRDiffusionMicrofiber
{
	GENERATED_BODY()
public:

	USRDiffusionSkin( const FObjectInitializer& ObjectInitializer );
	static const char* GetName ();
	void SetupMaterial ( UMaterial* Material ) override;
	void SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material ) override;
	void ConnectMatFields ( UMaterial* Material ) override;
	const int GetPrio() const override
	{
		return 25;
	}; 
protected:
	FName SubsurfaceProfileName;
	USubsurfaceProfile* SubsurfaceProfile;
};