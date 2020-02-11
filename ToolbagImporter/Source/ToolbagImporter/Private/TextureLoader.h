/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"


class UToolbagImportUI;
struct TextureDesc;
namespace ToolbagUtils
{
	struct Texture;
}
class TextureLoader
{
public:
	static UTexture* CreateTexture(ToolbagUtils::Texture* texture, unsigned flags, FString PackagePath, UToolbagImportUI* ImportUI, bool isNormal = false );
	static UTexture* CreateTexture( ToolbagUtils::Texture* texture, FString Extension, FString TextureName, UPackage* Package, TArray<uint8> DataBinary, UToolbagImportUI* ImportUI, bool isNormal = false );

};
