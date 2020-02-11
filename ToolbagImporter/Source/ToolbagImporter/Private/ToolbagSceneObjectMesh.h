/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagSceneObject.h"


namespace ToolbagUtils {
struct MeshSceneObject;
}

class UToolbagImportUI;

class ToolbagSceneObjectMesh : public ToolbagSceneObject
{
public:
	ToolbagSceneObjectMesh( ToolbagUtils::SceneObject* ToolbagSceneObject, TMap<FString, UMaterialInterface*>& Materials ) : ToolbagSceneObject(ToolbagSceneObject), Materials( Materials )
	{}
protected:
	virtual UClass* GetComponentClass()
	{
		return UStaticMeshComponent::StaticClass();
	}
	void ParseComponent(  UToolbagImportUI* ImportUI, USceneComponent* Component, FString Name ) override;

	static UStaticMesh* ReadMesh( ToolbagUtils::MeshSceneObject* ToolbagMeshSceneObject, FString Name, UToolbagImportUI* ImportUI, TMap<FString, UMaterialInterface*>& Materials );
protected:
	TMap<FString, UMaterialInterface*>& Materials;
};