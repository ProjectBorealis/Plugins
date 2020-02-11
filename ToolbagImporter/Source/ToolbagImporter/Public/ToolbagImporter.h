/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/

#pragma once

//#include "ModuleManager.h"
DECLARE_LOG_CATEGORY_EXTERN(LogToolbag, Log, All);

namespace ToolbagUtils
{
	struct Scene;
	struct Material;
}

typedef ToolbagUtils::Scene* (*_readScene)(const char* scenePath, const void* data, long long dataSize);
typedef ToolbagUtils::Material* (*_readMaterial)(const char* materialPath, const void* data, long long dataSize);

typedef void (*_destroyScene)(ToolbagUtils::Scene*);
typedef void (*_destroyMaterial)(ToolbagUtils::Material*);
class FToolbagImporterModule : public IModuleInterface
{
public:
	_readScene DLLReadScene;
	_readMaterial DLLReadMaterial;
	_destroyScene DLLDestroyScene;
	_destroyMaterial DLLDestroyMaterial;
	static bool IsDllLoaded();
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
protected:
	static bool bIsDllLoaded;
};