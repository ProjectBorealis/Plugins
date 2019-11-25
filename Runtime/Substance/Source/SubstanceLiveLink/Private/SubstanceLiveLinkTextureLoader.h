// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: FSubstanceLiveLinkTextureLoader.h
#pragma once
#include "EditorFramework/AssetImportData.h"

/**
 * Helper class to facilitate loading textures, only importing if the texture does not exist
 */
class FSubstanceLiveLinkTextureLoader : public TSharedFromThis<FSubstanceLiveLinkTextureLoader>
{
public:
	/**
	 * Resolves a texture by a given source image name from Painter
	 * @param MapName the map name provided by painter
	 * @param Filename the source image to map to a UTexture2D
	 * @param DestinationPath if the UTexture2D was not resolved, import a new one into this path
	 * @returns The loaded UTexture2D or nullptr if an error happened
	 */
	UTexture2D* ResolveTexture(const FString& MapName, const FString& Filename, const FString& DestinationPath);

	/**
	 * Returns true if the file has changed since the last time we opened it
	 * @param Filename the filename to compare
	 * @returns true if the file has changed
	 */
	bool HasFileChanged(const FString& Filename);

private:
	/**
	 * Data structure to hold texture hash and UObject information
	 */
	struct TextureInfo
	{
		TWeakObjectPtr<UTexture2D>	Texture;
		FMD5Hash					FileHash;
	};

private:
	/**
	 * Get a TextureInfo handle for a given source image, creating one if it doesn't exist
	 * @param Filename the filename of the source image
	 * @returns the TextureInfo associated with this image
	 */
	TextureInfo& GetTextureInfo(const FString& Filename);

private:
	/** Map to associate Painter images to Texture Info */
	TMap<FString, TextureInfo> TextureInfoMap;

	/** Critical Section to restrict control for the TextureInfoMap */
	FCriticalSection TextureInfoMapCS;
};
