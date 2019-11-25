// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkTextureLoader.cpp
#include "SubstanceLiveLinkTextureLoader.h"
#include "SubstanceLiveLinkPrivatePCH.h"
#include "AssetToolsModule.h"
#include "EditorReimportHandler.h"
#include "IAssetTools.h"
#include "ObjectTools.h"

UTexture2D* FSubstanceLiveLinkTextureLoader::ResolveTexture(const FString& MapName, const FString& Filename, const FString& DestinationPath)
{
	TextureInfo& TextureInfoObj = GetTextureInfo(Filename);
	UTexture2D* Texture = TextureInfoObj.Texture.Get();

	//if Texture is null, let's see if we can find the object by name
	if (Texture == nullptr)
	{
		FString Name = ObjectTools::SanitizeObjectName(FPaths::GetBaseFilename(Filename));
		FString PackageName = FPaths::Combine(*DestinationPath, *Name);

		if (UPackage* Pkg = CreatePackage(nullptr, *PackageName))
		{
			Pkg->FullyLoad();

			if (UObject* Object = StaticFindObject(UTexture2D::StaticClass(), Pkg, *Name, true))
			{
				Texture = CastChecked<UTexture2D>(Object);
			}
		}
	}

	//try to do a quick reimport if the texture already exists
	if (Texture)
	{
		TextureInfoObj.Texture = Texture;

		//verify filenames match
		TArray<FString> Filenames;

		Texture->AssetImportData->ExtractFilenames(Filenames);

		if (Filenames.Num() == 1 && Filenames[0] == Filename)
		{
			if (FReimportManager::Instance()->Reimport(Texture, false, false, Filename))
			{
				return Texture;
			}
		}
	}

	//full reimport if we don't have enough cached information
	UAutomatedAssetImportData* AssetImportData = NewObject<UAutomatedAssetImportData>();
	AssetImportData->GroupName = TEXT("SubstanceLiveLink");
	AssetImportData->Filenames.Add(Filename);
	AssetImportData->DestinationPath = DestinationPath;
	AssetImportData->bReplaceExisting = true;

	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TArray<UObject*> Objects = AssetTools.ImportAssetsAutomated(AssetImportData);

	for (const auto& Obj : Objects)
	{
		if (UTexture2D* CastTexture = CastChecked<UTexture2D>(Obj))
		{
			TextureInfo& TextureInfoRef = GetTextureInfo(Filename);
			TextureInfoRef.Texture = CastTexture;

			//make sure our ORM map is not SRGB
			if (MapName == TEXT("OcclusionRoughnessMetallic"))
			{
				CastTexture->CompressionSettings = TC_Masks;
				CastTexture->SRGB = false;
			}

			return CastTexture;
		}
	}

	UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unable to import texture \"%s\" from Substance Painter"), *Filename);
	return nullptr;
}

bool FSubstanceLiveLinkTextureLoader::HasFileChanged(const FString& Filename)
{
	FMD5Hash Hash = FMD5Hash::HashFile(*Filename);
	TextureInfo& TextureInfoRef = GetTextureInfo(Filename);

	if (TextureInfoRef.FileHash == Hash)
	{
		return false;
	}

	TextureInfoRef.FileHash = Hash;
	return true;
}

FSubstanceLiveLinkTextureLoader::TextureInfo& FSubstanceLiveLinkTextureLoader::GetTextureInfo(const FString& Filename)
{
	FScopeLock Lock(&TextureInfoMapCS);

	if (TextureInfo* FindInfo = TextureInfoMap.Find(Filename))
	{
		return *FindInfo;
	}

	TextureInfo& Info = TextureInfoMap.Emplace(Filename);
	Info.Texture = nullptr;
	return Info;
}
