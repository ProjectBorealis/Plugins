/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "TextureLoader.h"
#include "ToolbagImporterPrivatePCH.h"
#include <Editor/UnrealEd/Public/ObjectTools.h>
#include <Editor/UnrealEd/Public/PackageTools.h>
#include <Developer/AssetTools/Public/AssetToolsModule.h>
#include <Runtime/AssetRegistry/Public/AssetRegistryModule.h>
#include "ToolbagUtils.h"
#include "UI/ToolbagImportUI.h"
#include "mset/MatFieldTexture.h"
#define LOCTEXT_NAMESPACE "ToolbagImportFactory"

UTexture* TextureLoader::CreateTexture( ToolbagUtils::Texture* texture, FString Extension, FString TextureName, UPackage* Package, TArray<uint8> DataBinary, UToolbagImportUI* ImportUI, bool isNormal )
{

	UTexture* UnrealTexture = FindObject<UTexture>(Package, *TextureName);
	if(UnrealTexture != NULL)
	{
		return UnrealTexture;
	}
	if (DataBinary.Num()>0)
	{
		const uint8* PtrTexture = DataBinary.GetData();
		auto TextureFact = NewObject<UTextureFactory>();
		TextureFact->AddToRoot();

		// save texture settings if texture exist
		TextureFact->SuppressImportOverwriteDialog();

		// Unless the normal map setting is used during import, 
		//	the user has to manually hit "reimport" then "recompress now" button
		if ( isNormal )
		{
			TextureFact->LODGroup = TEXTUREGROUP_WorldNormalMap;
			TextureFact->CompressionSettings = TC_Normalmap;
		}
		const TCHAR* TextureType = *Extension;
		FFeedbackContext* FeedbackContext = GWarn;
		UnrealTexture = (UTexture*)TextureFact->FactoryCreateBinary(
			UTexture::StaticClass(), Package, *TextureName, 
			RF_Standalone|RF_Public, NULL, TextureType, 
			PtrTexture, PtrTexture+DataBinary.Num(), FeedbackContext );
		if ( UnrealTexture != NULL )
		{
			UTexture2D* Texture2D = Cast< UTexture2D>(UnrealTexture);
			if(Texture2D != NULL)
			{
				switch(texture->wrapMode)
				{
				case ToolbagUtils::Texture::TEXTURE_WRAP_CLAMP:
					Texture2D->AddressX = TextureAddress::TA_Clamp;
					Texture2D->AddressY = TextureAddress::TA_Clamp;
					break;
				case ToolbagUtils::Texture::TEXTURE_WRAP_REPEAT:
					Texture2D->AddressX = TextureAddress::TA_Wrap;
					Texture2D->AddressY = TextureAddress::TA_Wrap;
					break;
				}
			}
			UnrealTexture->SRGB = texture->sRGB ? 1 : 0;
			UnrealTexture->Filter = texture->filterMode == ToolbagUtils::Texture::TEXTURE_FILTER_NEAREST ? TF_Nearest : TF_Bilinear;

			// Notify the asset registry
			FAssetRegistryModule::AssetCreated(UnrealTexture);

			// Set the dirty flag so this package will get saved later
			Package->SetDirtyFlag(true);
		}
		else
		{
			if(!ImportUI->bSuppressWarnings)
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("ToolbagImport_TextureNotSupported", "The Texture Format of {0}.{1} is not supported by UE4."), FText::FromString(TextureName), FText::FromString(Extension)));
			}
		}
		TextureFact->RemoveFromRoot();
	}
	return UnrealTexture;
}

UTexture* TextureLoader::CreateTexture(ToolbagUtils::Texture* texture, unsigned flags, FString PackagePath, UToolbagImportUI* ImportUI, bool isNormal )
{
	FString FileBasePath = FPaths::GetPath(UFactory::GetCurrentFilename());
	// create an unreal texture asset
	FString Extension = FPaths::GetExtension(texture->TexturePath).ToLower();
	// name the texture with file name
	FString TextureName = FPaths::GetBaseFilename(texture->TexturePath);
	UPackage* TexturePackage = ToolbagUtils::CreatePackageForAsset<UTexture>( PackagePath, TextureName );

	FString Filename = FileBasePath / texture->TexturePath;
	TArray<uint8> DataBinary;
	if ( ! FFileHelper::LoadFileToArray( DataBinary, *Filename ))
	{
		Filename = texture->TexturePath;
		if (!FFileHelper::LoadFileToArray( DataBinary, *Filename ))
		{
			UE_LOG( LogToolbag, Warning, TEXT( "Unable to find Texture file %s." ), *Filename );
			return NULL;
		}
	}

	UE_LOG(LogToolbag, Verbose, TEXT("Loading texture file %s"),*Filename);
	return CreateTexture( texture, Extension, TextureName, TexturePackage, DataBinary, ImportUI, isNormal );
}

#undef LOCTEXT_NAMESPACE