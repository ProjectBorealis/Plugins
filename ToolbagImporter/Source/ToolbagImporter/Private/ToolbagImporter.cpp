/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/

#include "ToolbagImporter.h"
#include "ToolbagImporterPrivatePCH.h"

#define LOCTEXT_NAMESPACE "FToolbagImporterModule"
DEFINE_LOG_CATEGORY(LogToolbag);

bool FToolbagImporterModule::bIsDllLoaded;

bool FToolbagImporterModule::IsDllLoaded()
{
	return bIsDllLoaded;
}

void FToolbagImporterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	DLLReadScene = NULL;
	DLLReadMaterial = NULL;
	DLLDestroyScene = NULL;
	DLLDestroyMaterial = NULL;
	bIsDllLoaded = false;
	FString filePath = FPaths::Combine(*FPaths::EnginePluginsDir(), TEXT("Marketplace/ToolbagImporter/Libs"), TEXT("ToolbagUtils.dll")); // Concatenate the plugins folder and the DLL file.
	if (!FPaths::FileExists(filePath))
	{
		filePath = FPaths::Combine(*FPaths::EnginePluginsDir(), TEXT("ToolbagImporter/Libs"), TEXT("ToolbagUtils.dll")); // Concatenate the plugins folder and the DLL file.
	}
	if (!FPaths::FileExists(filePath))
	{
		filePath = FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("Marketplace/ToolbagImporter/Libs"), TEXT("ToolbagUtils.dll")); // Concatenate the plugins folder and the DLL file.
	}
	if (!FPaths::FileExists(filePath))
	{
		filePath = FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("ToolbagImporter/Libs"), TEXT("ToolbagUtils.dll")); // Concatenate the plugins folder and the DLL file.
	}
	if (FPaths::FileExists(filePath))
	{
		void *DLLHandle;
		DLLHandle = FPlatformProcess::GetDllHandle(*filePath);
		
		if (DLLHandle != NULL)
		{
			FString ReadSceneName = "ReadScene";
			FString ReadMaterialName = "ReadMaterial";
			FString DestroySceneName = "DestroyScene";
			FString DestroyMaterialName = "DestroyMaterial";
			DLLReadScene = (_readScene)FPlatformProcess::GetDllExport(DLLHandle, *ReadSceneName);
			DLLReadMaterial = (_readMaterial)FPlatformProcess::GetDllExport(DLLHandle, *ReadMaterialName);
			DLLDestroyScene = (_destroyScene)FPlatformProcess::GetDllExport(DLLHandle, *DestroySceneName);
			DLLDestroyMaterial = (_destroyMaterial)FPlatformProcess::GetDllExport(DLLHandle, *DestroyMaterialName);
			if(DLLReadScene != NULL && DLLReadMaterial != NULL && DLLDestroyScene != NULL && DLLDestroyMaterial != NULL )
			{
				bIsDllLoaded = true;
			}
		}
	}
}

void FToolbagImporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	bIsDllLoaded = false;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToolbagImporterModule, ToolbagImporter)