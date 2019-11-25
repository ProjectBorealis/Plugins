/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "Engine/DeveloperSettings.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerSettings.generated.h"


/**
 *  Default Settings for MultiPacker Graph
 *  The Settings selected to be here are the most important and global for every Asset
 */
UCLASS(config=Game, defaultconfig)
class MULTIPACKERRUNTIME_API UMultiPackerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
 
	UMultiPackerSettings()
	{
		ChannelMethod = EChannelTextureSave::CS_Atlas;
		SaveMaterialCollection = true;
		SaveDatabase = true;
		TargetDirectory.Path = "Textures/";
		DatabasePrefix = "MPDB_";
		TexturePrefix = "T_";
		MaterialcollectionPrefix = "MC_";
		TextureChannelName = "MultiPacker_CP";
	}
	
	FORCEINLINE EChannelTextureSave GetChannelMethod() const
	{
		return ChannelMethod;
	}

	FORCEINLINE bool GetSaveMaterialCollection() const
	{
		return SaveMaterialCollection;
	}

	FORCEINLINE bool GetSaveDatabase() const
	{
		return SaveDatabase;
	}

	FORCEINLINE FDirectoryPath GetTargetDirectory() const
	{
		return TargetDirectory;
	}

	FORCEINLINE FString GetDatabasePrefix() const
	{
		return DatabasePrefix;
	}
	
	FORCEINLINE FString GetTexturePrefix() const
	{
		return TexturePrefix;
	}

	FORCEINLINE FString GetMaterialcollectionPrefix() const
	{
		return MaterialcollectionPrefix;
	}

	FORCEINLINE FString GetTextureChannelName() const
	{
		return TextureChannelName;
	}
private:
	//Select between multiples modes for save the texture on channel; 
	//Multiple saves 3 Textures by Channel
	//One save 1 Texture on Channel
	//Atlas save 1 Texture on RGB/A Channels
	//One_SDF save 1 Texture SDF on Channel , This is the Global option who process all the nodes to SDF, for a granular selection you have the Boolean selection on the nodes
	UPROPERTY(config, EditAnywhere, Category = "ChannelsOptions")
		EChannelTextureSave ChannelMethod = EChannelTextureSave::CS_Atlas;
	
	//Make and save the Material Collection
	UPROPERTY(config, EditAnywhere, Category = "SaveData")
		bool SaveMaterialCollection = true;

	//Make and save the Material Collection
	UPROPERTY(config, EditAnywhere, Category = "SaveData")
		bool SaveDatabase = true;

	//Directory to save the Output Texture
	UPROPERTY(config, EditAnywhere, Category = "Location", meta = (RelativeToGameContentDir, ContentDir))
		FDirectoryPath TargetDirectory;

	UPROPERTY(EditAnywhere, Category = "Output")
		FString TextureChannelName = "MultiPacker_CP";

	//Prefix for the MultiPacker Database to use with blueprints
	UPROPERTY(config, EditAnywhere, Category = "Prefix")
		FString DatabasePrefix = "MPDB_";

	//Prefix for the Output Textures
	UPROPERTY(config, EditAnywhere, Category = "Prefix")
		FString TexturePrefix = "T_";

	//Prefix for the Material Collection
	UPROPERTY(config, EditAnywhere, Category = "Prefix")
		FString MaterialcollectionPrefix = "MC_";


};