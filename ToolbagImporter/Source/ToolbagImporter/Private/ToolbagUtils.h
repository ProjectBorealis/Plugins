/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include <Editor/UnrealEd/Public/ObjectTools.h>
#include <Editor/UnrealEd/Public/PackageTools.h>
#include <Developer/AssetTools/Public/AssetToolsModule.h>
#include <Runtime/AssetRegistry/Public/AssetRegistryModule.h>
#include <Developer/AssetTools/Public/IAssetTools.h>
#include "ToolbagUtils/Structs.h"

namespace ToolbagUtils
{
	template <typename AssetType>
	UPackage* CreatePackageForAsset(FString Path, FString AssetName)
	{

		AssetName = ObjectTools::SanitizeObjectName(AssetName);

		// set where to place the textures
		FString BasePackageName = Path / AssetName;
		BasePackageName = PackageTools::SanitizePackageName(BasePackageName);

		AssetType* ExistingAsset = NULL;
		UPackage* AssetPackage = NULL;
		// First check if the asset already exists.
		{
			FString ObjectPath = BasePackageName + TEXT(".") + AssetName;
			ExistingAsset = LoadObject<AssetType>(NULL, *ObjectPath);
		}


		if( !ExistingAsset )
		{
			const FString Suffix(TEXT(""));

			FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
			FString FinalPackageName;
			AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, Suffix, FinalPackageName, AssetName);

			AssetPackage = CreatePackage(NULL, *FinalPackageName);
		}
		else
		{
			AssetPackage = ExistingAsset->GetOutermost();
		}

		return AssetPackage;
	}


	float CalculateScaleFactor( ToolbagUtils::SceneUnit unit )
	{
		switch(unit)
		{
		case ToolbagUtils::MILLIMETER:
			return 0.1f;
		case ToolbagUtils::CENTIMETER:
			return 1.0f;
		case ToolbagUtils::METER:
			return 100.0f;
		case ToolbagUtils::KILOMETER:
			return 1000000.0f;
		case ToolbagUtils::INCH:
			return 2.54f;
		case ToolbagUtils::FOOT:
			return 30.48f;
		case ToolbagUtils::YARD:
			return 91.44f;
		case ToolbagUtils::MILE:
			return 160934.0f;
		default: 
			return 1.0f;
		}
	}
}
