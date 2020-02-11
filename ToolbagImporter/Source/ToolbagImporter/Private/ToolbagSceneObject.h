/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"

class UToolbagImportUI;


namespace ToolbagUtils
{
	struct SceneObject;
}
class ToolbagSceneObject
{
public:
	ToolbagSceneObject(ToolbagUtils::SceneObject* ToolbagSceneObject) : TSO(ToolbagSceneObject)
	{}
	virtual ~ToolbagSceneObject()
	{}

	enum
	{
		OUTLINE_COLLAPSED =			(1 << 0),
		OUTLINE_PERMANENT =			(1 << 1),
		VIEWPORT_TRANSFORMABLE =	(1 << 2),
		OUTLINE_MARKED_HIDDEN =		(1 << 3),
		OUTLINE_LOCKED =			(1 << 4),
		OUTLINE_FIXED_PARENT =		(1 << 5),
		OUTLINE_NO_CHILDREN =		(1 << 6),
		VIEWPORT_SELECTS_PARENT =	(1 << 7),
		EDITOR_NO_DUPLICATION =		(1 << 9),
		DUPLICATION_TEMPLATE =		(1 << 31),
	};

	virtual USCS_Node* Read( USCS_Node* Parent, UToolbagImportUI* ImportUI);
	virtual AActor* Read( AActor* Parent, UToolbagImportUI* ImportUI);
protected:
	virtual UClass* GetComponentClass()
	{
		return USceneComponent::StaticClass();
	}
	virtual void ParseComponent( UToolbagImportUI* ImportUI, USceneComponent* Component, FString Name )
	{}
	ToolbagUtils::SceneObject* TSO;
	FString name;
};
