/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagSceneObject.h"

class UToolbagImportUI;

class ToolbagSceneObjectLight : public ToolbagSceneObject
{
public:
	ToolbagSceneObjectLight(ToolbagUtils::SceneObject* ToolbagSceneObject) : ToolbagSceneObject(ToolbagSceneObject)
	{}
	enum LightType
	{
		DIRECTIONAL =	0,
		SPOT =			1,
		OMNI =			2,
	};

protected:
	UClass* GetComponentClass() override;
	void ParseComponent(  UToolbagImportUI* ImportUI, USceneComponent* Component, FString Name ) override;
};

#pragma once
