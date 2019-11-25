/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "MultiPackerAssetEditor/MultiPackerProcessCore.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerBaseNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"
#include "MultiPackerAssetEditor/MultiPackerEdGraph.h"
#include "BinPack/MaxRectsBinPack.h"
#include "Graph/MultiPacker.h"
#include "MultiPackerDataBase.h"
#include "RenderUtils.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Factory/MultiPackerFactory.h"
#include "Factory/MultiPackerDatabaseFactory.h"
#include "Factories/MaterialParameterCollectionFactoryNew.h"
#include <UObject/Object.h>
#include <UObject/UObjectGlobals.h>
#include "TileUtils/TilePointer.h"
#include "AssetRegistryModule.h"
#include <AssetToolsModule.h>
#include <ContentBrowserModule.h>

UMultiPackerProcessCore::UMultiPackerProcessCore(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMultiPackerProcessCore::CreateMaterialCollection(uint16 width, uint16 height, FAssetToolsModule& AssetToolsModule, FContentBrowserModule& ContentBrowserModule)
{
	TArray<FCollectionVectorParameter> VectorArray;
	TArray<FCollectionScalarParameter> ScalarArray;
	FCollectionVectorParameter Grid = FCollectionVectorParameter();
	Grid.ParameterName = FName(TEXT("Tiles"));
	Grid.DefaultValue = FLinearColor(HTile, VTile, width, height);
	VectorArray.Add(Grid);
	for (uint16 data = 0; data < BaseInput->NewTileData.Num(); data++)
	{
		FCollectionScalarParameter Tile = FCollectionScalarParameter();
		FString framestring = (BaseInput->NewTileData[data]->TileName.ToString()).Append(TEXT("_frame"));
		Tile.ParameterName = FName(*framestring);
		Tile.DefaultValue = BaseInput->NewTileData[data]->Frame;
		ScalarArray.Add(Tile);

		FCollectionVectorParameter channel = FCollectionVectorParameter();
		FString channelstring = (BaseInput->NewTileData[data]->TileName.ToString()).Append(TEXT("_channel"));
		FCollectionVectorParameter layer = FCollectionVectorParameter();
		FString layerstring = (BaseInput->NewTileData[data]->TileName.ToString()).Append(TEXT("_layer"));
		channel.ParameterName = FName(*channelstring);
		layer.ParameterName = FName(*layerstring);
		FCollectionVectorParameter SizePadding = FCollectionVectorParameter();
		FString sizestring = (BaseInput->NewTileData[data]->TileName.ToString()).Append(TEXT("_sizePadding"));
		SizePadding.ParameterName = FName(*sizestring);
		SizePadding.DefaultValue = BaseInput->NewTileData[data]->SizePadding_vec;
		VectorArray.Add(SizePadding);
		SetDataLinearColors(BaseInput->NewTileData[data]->ChannelTexture, channel.DefaultValue, layer.DefaultValue);
		VectorArray.Add(channel);
		VectorArray.Add(layer);
	}
	if (BaseInput->MaterialCollection == NULL)
	{
		TArray<UObject*> ObjectsToSync;
		UMaterialParameterCollectionFactoryNew* MaterialCollectionNew = NewObject<UMaterialParameterCollectionFactoryNew>();
		TArray<FString> NewPackageName = PackageName(BaseInput->MaterialcollectionPrefix, "", AssetToolsModule);
		if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(NewPackageName[1], NewPackageName[0], UMaterialParameterCollection::StaticClass(), MaterialCollectionNew))
		{
			NewAsset->AddToRoot();
			NewAsset->MarkPackageDirty();
			ObjectsToSync.Add(NewAsset);
		}
		if (ObjectsToSync.Num() > 0)
		{
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
		}

		if (ObjectsToSync.Num() > 0)
			if (UMaterialParameterCollection* NewCollection = Cast<UMaterialParameterCollection>(ObjectsToSync[0]))
			{
				BaseInput->MaterialCollection = NewCollection;
			}
	}
	if (BaseInput->MaterialCollection)
	{
		//Scalar replace or add
		for (uint16 newScalar = 0; newScalar < ScalarArray.Num(); ++newScalar)
		{
			uint16 find = -1;
			bool isFound = false;
			for (uint16 oldScalar = 0; oldScalar < BaseInput->MaterialCollection->ScalarParameters.Num(); ++oldScalar)
			{
				if (BaseInput->MaterialCollection->ScalarParameters[oldScalar].ParameterName == ScalarArray[newScalar].ParameterName)
				{
					isFound = true;
					find = oldScalar;
				}
			}
			if (isFound)
			{
				BaseInput->MaterialCollection->ScalarParameters[find].DefaultValue = ScalarArray[newScalar].DefaultValue;
			}
			else
			{
				BaseInput->MaterialCollection->ScalarParameters.Add(ScalarArray[newScalar]);
			}
		}
		//Scalar remove
		for (uint16 oldScalar = 0; oldScalar < BaseInput->MaterialCollection->ScalarParameters.Num(); ++oldScalar)
		{
			bool isFound = false;
			for (uint16 newScalar = 0; newScalar < ScalarArray.Num(); ++newScalar)
			{
				if (ScalarArray[newScalar].ParameterName == BaseInput->MaterialCollection->ScalarParameters[oldScalar].ParameterName)
				{
					isFound = true;
				}
			}
			if (!isFound)
			{
				BaseInput->MaterialCollection->ScalarParameters.RemoveAt(oldScalar, 1, true);
			}
		}
		//vector repace or add
		for (uint16 newScalar = 0; newScalar < VectorArray.Num(); ++newScalar)
		{
			uint16 find = -1;
			bool isFound = false;
			for (uint16 oldScalar = 0; oldScalar < BaseInput->MaterialCollection->VectorParameters.Num(); ++oldScalar)
			{
				if (BaseInput->MaterialCollection->VectorParameters[oldScalar].ParameterName == VectorArray[newScalar].ParameterName)
				{
					isFound = true;
					find = oldScalar;
				}
			}
			if (isFound)
			{
				BaseInput->MaterialCollection->VectorParameters[find].DefaultValue = VectorArray[newScalar].DefaultValue;
			}
			else
			{
				BaseInput->MaterialCollection->VectorParameters.Add(VectorArray[newScalar]);
			}
		}
		//vector remove
		for (uint16 oldScalar = 0; oldScalar < BaseInput->MaterialCollection->VectorParameters.Num(); ++oldScalar)
		{
			bool isFound = false;
			for (uint16 newScalar = 0; newScalar < VectorArray.Num(); ++newScalar)
			{
				if (VectorArray[newScalar].ParameterName == BaseInput->MaterialCollection->VectorParameters[oldScalar].ParameterName)
				{
					isFound = true;
				}
			}
			if (!isFound)
			{
				BaseInput->MaterialCollection->VectorParameters.RemoveAt(oldScalar, 1, true);
			}
		}
		BaseInput->MaterialCollection->PostLoad();
		BaseInput->MaterialCollection->PostEditChange();
		for (TObjectIterator<UWorld> It; It; ++It)
		{
			UWorld* CurrentWorld = *It;
			CurrentWorld->AddParameterCollectionInstance(BaseInput->MaterialCollection, true);
			CurrentWorld->SetupParameterCollectionInstances();
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 21)
			CurrentWorld->UpdateParameterCollectionInstances(true);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 21)
			CurrentWorld->UpdateParameterCollectionInstances(true, true);
#endif
			CurrentWorld->GetParameterCollectionInstance(BaseInput->MaterialCollection)->PostEditChange();
		}
		TSet<FName> ParameterNames;
		for (uint16 oldVector = 0; oldVector < BaseInput->MaterialCollection->VectorParameters.Num(); ++oldVector)
		{
			ParameterNames.Add(BaseInput->MaterialCollection->VectorParameters[oldVector].ParameterName);
		}
		for (uint16 oldScalar = 0; oldScalar < BaseInput->MaterialCollection->ScalarParameters.Num(); ++oldScalar)
		{
			ParameterNames.Add(BaseInput->MaterialCollection->ScalarParameters[oldScalar].ParameterName);
		}
		// Go through all materials in memory and recompile them if they use this material parameter collection
		for (TObjectIterator<UMaterial> It; It; ++It)
		{
			UMaterial* CurrentMaterial = *It;

			bool bRecompile = false;

			// Preview materials often use expressions for rendering that are not in their Expressions array, 
			// And therefore their MaterialParameterCollectionInfos are not up to date.
			if (CurrentMaterial->bIsPreviewMaterial)
			{
				bRecompile = true;
			}
			else
			{
				for (uint16 FunctionIndex = 0; FunctionIndex < CurrentMaterial->MaterialParameterCollectionInfos.Num() && !bRecompile; FunctionIndex++)
				{
					if (CurrentMaterial->MaterialParameterCollectionInfos[FunctionIndex].ParameterCollection == BaseInput->MaterialCollection)
					{
						TArray<UMaterialExpressionCollectionParameter*> CollectionParameters;
						CurrentMaterial->GetAllExpressionsInMaterialAndFunctionsOfType(CollectionParameters);
						for (UMaterialExpressionCollectionParameter* CollectionParameter : CollectionParameters)
						{
							if (ParameterNames.Contains(CollectionParameter->ParameterName))
							{
								bRecompile = true;
								break;
							}
						}
					}
				}
			}
			if (bRecompile)
			{
				// Propagate the change to this material
				CurrentMaterial->MarkPackageDirty();
			}
		}
		BaseInput->MaterialCollection->MarkPackageDirty();
	}
}
//Basic Calc to get the vertical and horizontal number of tiles based on the total number of tiles
void UMultiPackerProcessCore::GetOutputTileNumbers(uint16 Tiles)
{
	VTile = HTile = FWindowsPlatformMath::RoundUpToPowerOfTwo(FMath::CeilToInt(FMath::Sqrt(Tiles)));
}

//Based on the Enum Selected the number of channels/Masks are calc
uint16 UMultiPackerProcessCore::GetNumberOfMasks()
{
	if (BaseInput->ChannelMethod == EChannelTextureSave::CS_Atlas)
		return 1;
	uint16 Masks = BaseInput->ChannelMethod == EChannelTextureSave::CS_Multiple ? 3 : 1;
	Do_SDF = BaseInput->ChannelMethod == EChannelTextureSave::CS_OneSdf ? true : false;
	Masks *= BaseInput->Alpha ? 4 : 3;
	return Masks;
}

uint16 UMultiPackerProcessCore::GetNumberTiles()
{
	uint16 TotalTiles = 0;
	UMultiPackerEdGraph* EdGraph = Cast<UMultiPackerEdGraph>(BaseInput->EdGraph);
	for (uint16 num = 0; num < EdGraph->Nodes.Num(); ++num)
	{
		UMultiPackerBaseNode* BaseNode = Cast<UMultiPackerBaseNode>(EdGraph->Nodes[num]);
		if (BaseNode != nullptr)
		{
			if (BaseNode->PinConnected() && BaseNode->IsObjectValid())
			{
				TotalTiles += BaseNode->GetNumberTiles();
			}
		}
	}
	BaseInput->TotalTiles = TotalTiles;
	return TotalTiles;
}

TArray<FString> UMultiPackerProcessCore::PackageName(FString Prefix, FString TextureNum, FAssetToolsModule& AssetToolsModule)
{
	FString importDirectory = BaseInput->TargetDirectory.Path;
	FString ObjectName = BaseInput->TargetName;

	FString TextureName = ObjectName;
	if (ObjectName == "None")
		TextureName = "MultiPacker";
	FString NPrefix = Prefix;
	TextureName = NPrefix.Append(TextureName);
	if (TextureNum != "")
		TextureName = TextureName.Append(TextureNum);

	FString NewPackageName = TEXT("/Game/") + (importDirectory == "" ? "Textures/" : importDirectory + "/") + TextureName;

	FString Name;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(NewPackageName, TextureName, PackageName, Name);
	const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
	TArray<FString> NamesOut;
	NamesOut.Add(PackagePath);
	NamesOut.Add(TextureName);
	return NamesOut;
}

void UMultiPackerProcessCore::PopulateMap(uint16 SizeVertical, uint16 SizeHorizontal, uint16 Masks)
{
	TileMap.Reset();
	for (uint16 TileD = 0; TileD < BaseInput->NewTileData.Num(); ++TileD)
	{
		if (BaseInput->NewTileData[TileD]->TileName == FName("None"))
		{
			FString TileName = FString(TEXT("Tile_"));
			TileName = TileName.Append(FString::FromInt(TileD));
			BaseInput->NewTileData[TileD]->TileName = FName(*TileName);
		}
		
		FName TransitionalName = BaseInput->NewTileData[TileD]->TileName;
		uint16 breakWhile = 0;
		while (TileMap.Contains(TransitionalName))
		{
			++breakWhile;
			if (breakWhile == 1000)
				break;//escape
			FString NewNum = FString::FromInt(1);
			TransitionalName.AppendString(NewNum);
		}
		BaseInput->NewTileData[TileD]->Frame /= Masks;

		float Red = (float)BaseInput->NewTileData[TileD]->SizeAndPadding.x / (float)BaseInput->NewTileData[TileD]->SizeAndPadding.width;
		float Green = (float)BaseInput->NewTileData[TileD]->SizeAndPadding.y / (float)BaseInput->NewTileData[TileD]->SizeAndPadding.height;
		float Blue = (float)SizeHorizontal / (float)BaseInput->NewTileData[TileD]->SizeAndPadding.width;
		float Alpha = (float)SizeVertical / (float)BaseInput->NewTileData[TileD]->SizeAndPadding.height;
		Blue = Blue == INFINITY ? 0.0f : Blue;
		Alpha = Alpha == INFINITY ? 0.0f : Alpha;
		Red = Red == INFINITY ? 0.0f : Red;
		Green = Green == INFINITY ? 0.0f : Green;
		BaseInput->NewTileData[TileD]->SizePadding_vec = FLinearColor(Red, Green, Blue, Alpha);
		SetDataLinearColors(BaseInput->NewTileData[TileD]->ChannelTexture, BaseInput->NewTileData[TileD]->Channel_vec, BaseInput->NewTileData[TileD]->Layer_vec);
		TileMap.Add(TransitionalName, *(BaseInput->NewTileData[TileD]));
	}
	BaseInput->TileMap = TileMap;
}

void UMultiPackerProcessCore::SaveTextureFromTile(UTilePointer* InTile, FString TextureNum, FAssetToolsModule& AssetToolsModule)
{
	TArray<FString> NewPackageName = TexturePackageName(BaseInput->TexturePrefix, TextureNum, AssetToolsModule);
	// create a static 2d texture
	UPackage* package = CreatePackage(NULL, *NewPackageName[1]);
	package->FullyLoad();
	package->Modify();
	UTexture2D* NewTexture;
	if (InTile->TileRT)
	{
		 NewTexture = InTile->TileRT->ConstructTexture2D(package, *NewPackageName[0], Flags, BaseInput->Alpha ? CTF_Compress : CTF_ForceOpaque , NULL);
	}
	else
	if (InTile->TileTexture)
	{
		NewTexture = InTile->FillTextureOutput(package, *NewPackageName[0], Flags);
	} 
	else
	{
		//error
		return;
	}
	NewTexture->PostEditChange();
	if (NewTexture)
	{
		// package needs saving
		NewTexture->MarkPackageDirty();
		if (BaseInput->EditTexture)
		{
			NewTexture->Filter = BaseInput->T_TextureFilter;
			NewTexture->SRGB = BaseInput->Tex_SRGB;
			NewTexture->AddressX = BaseInput->Tex_AddressX;
			NewTexture->AddressY = BaseInput->Tex_AddressY;
			NewTexture->PowerOfTwoMode = BaseInput->Tex_Power;
		}
		else
		{
			NewTexture->Filter = BaseInput->ChannelMethod == EChannelTextureSave::CS_Multiple ? TF_Nearest : TF_Bilinear;
			NewTexture->SRGB = false;
			NewTexture->AddressX = TA_Clamp;
			NewTexture->AddressY = TA_Clamp;
			NewTexture->PowerOfTwoMode = ETexturePowerOfTwoSetting::PadToPowerOfTwo;
		}

		if (Do_MSDF)
		{
			NewTexture->CompressionSettings = TC_VectorDisplacementmap;
		}
		NewTexture->CompressionNoAlpha = !BaseInput->Alpha;

		FAssetRegistryModule::AssetCreated(NewTexture);
	}
	BaseInput->TextureOutput.Add(NewTexture);
}

void UMultiPackerProcessCore::SaveDataBase(FString TargetName, FAssetToolsModule& AssetToolsModule, FContentBrowserModule& ContentBrowserModule, uint16 width, uint16 height)
{
	UMultiPackerDataBase* NewDataBase = NewObject<UMultiPackerDataBase>(); //NULL;

	if (BaseInput->DataBase == NULL)
	{
		TArray<UObject*> ObjectsToSync;
		UMultiPackerDatabaseFactory* DataBaseFactory = NewObject<UMultiPackerDatabaseFactory>();
		TArray<FString> NewPackageName = PackageName(BaseInput->DatabasePrefix, "", AssetToolsModule);

		if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(NewPackageName[1], NewPackageName[0], UMultiPackerDataBase::StaticClass(), DataBaseFactory))
		{
			BaseInput->DataBase = Cast<UMultiPackerDataBase>(NewAsset);

			NewAsset->AddToRoot();
			NewAsset->MarkPackageDirty();
			ObjectsToSync.Add(NewAsset);
		}
		if (ObjectsToSync.Num() > 0)
		{
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
		}
	}
	NewDataBase = BaseInput->DataBase;
	NewDataBase->VTile = VTile;
	NewDataBase->HTile = HTile;
	NewDataBase->Do_SDF = Do_SDF;
	NewDataBase->Alpha = BaseInput->Alpha;
	NewDataBase->BinPack = BaseInput->RectangleTiles;
	NewDataBase->ChannelMethod = BaseInput->ChannelMethod;
	NewDataBase->layer_bools = FLinearColor(BaseInput->RectangleTiles ? 1 : 0,
		(BaseInput->ChannelMethod == EChannelTextureSave::CS_One || BaseInput->ChannelMethod == EChannelTextureSave::CS_OneSdf) ? 1 : 0,
		(BaseInput->ChannelMethod == EChannelTextureSave::CS_Multiple) ? 1 : 0,
		0);
	NewDataBase->Tiles_Size = FLinearColor(HTile, VTile, width, height);
	NewDataBase->TextureOutput = BaseInput->TextureOutput;
	NewDataBase->TileMap = TileMap;
	NewDataBase->TileSize = TileSize;
	NewDataBase->MaterialCollection = BaseInput->MaterialCollection;
	BaseInput->DataBase->MarkPackageDirty();
}

void UMultiPackerProcessCore::SetGraph(UMultiPacker* EditingGraph)
{
	BaseInput = EditingGraph;
}

void UMultiPackerProcessCore::ProcessNodes(UMultiPacker* Graph)
{
	//Initial Variables
	BaseInput = Graph;
	Output = GetTilesFromNodes();
	Flags = BaseInput->Flags | RF_Public | RF_Standalone;
	//Variables Canvas
	bool IsRectangled = BaseInput->RectangleTiles;
	uint16 TextureOutputSizeVertical = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(BaseInput->OutputSizeY);
	uint16 TextureOutputSizeHorizontal = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(BaseInput->OutputSizeX);
	uint16 Tile_Size = UMultiPackerBaseEnums::GetTextureSizeOutputTile(BaseInput->TextureTileSizeOutput);
	uint16 Masks = GetNumberOfMasks();
	if (!IsRectangled)
	{
		bool Atlas = Masks == 1 ? true : false;
		uint16 ImagesNum = Output.Num();
		TileSize = ImagesNum;
		uint16 Tiles = ceil(ImagesNum / Masks);
		Tiles = Tiles * Masks < ImagesNum ? (Tiles + 1) : Tiles;
		GetOutputTileNumbers(Tiles);
		TextureOutputSizeVertical = VTile * Tile_Size;
		TextureOutputSizeHorizontal = HTile * Tile_Size;
	}
	else
	{
		Output = UTilePointer::SortArrayTiles(Output);
	}
	//BinPack
	Output = TileBinPack(Output, TextureOutputSizeVertical, TextureOutputSizeHorizontal, Masks);
	PopulateMap(TextureOutputSizeVertical, TextureOutputSizeHorizontal, Masks);
	BaseInput->TextureOutput.Empty();
	//Save Assets
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	for (uint16 multipleTextures = 0; multipleTextures < Output.Num(); ++multipleTextures)
	{
		FString AddName = FString(TEXT(""));
		if (multipleTextures > 0)
		{
			AddName = FString(TEXT("_"));
			AddName = AddName.Append(FString::FromInt(multipleTextures));
		}
		SaveTextureFromTile(Output[multipleTextures], AddName, AssetToolsModule);
	}
	//debug
	BaseInput->TilePointers = Output;
	//search the Outputnode
	UMultiPackerEdGraph* EdGraph = Cast<UMultiPackerEdGraph>(BaseInput->EdGraph);
	for (uint16 num = 0; num < EdGraph->Nodes.Num(); ++num)
	{
		UMultiPackerOutputNode* OutputNode = Cast<UMultiPackerOutputNode>(EdGraph->Nodes[num]);
		if (OutputNode != nullptr)
		{
			OutputNode->SetArrayThumnailOutput(BaseInput->TextureOutput);
		}
	}
	if (BaseInput->SaveMaterialCollection)
		CreateMaterialCollection(TextureOutputSizeHorizontal, TextureOutputSizeVertical, AssetToolsModule, ContentBrowserModule);

	if (BaseInput->SaveDatabase)
		SaveDataBase(BaseInput->TargetName, AssetToolsModule, ContentBrowserModule, TextureOutputSizeHorizontal, TextureOutputSizeVertical);
}

TArray<UTilePointer*> UMultiPackerProcessCore::GetTilesFromNodes()
{
	TArray<UTilePointer*> A_Output;
	uint16 Tile_Size = UMultiPackerBaseEnums::GetTextureSizeOutputTile(BaseInput->TextureTileSizeOutput);
	UMultiPackerEdGraph* EdGraph = Cast<UMultiPackerEdGraph>(BaseInput->EdGraph);
	
	for (uint16 num = 0; num < EdGraph->Nodes.Num(); ++num)
	{
		UMultiPackerBaseNode* BaseNode = Cast<UMultiPackerBaseNode>(EdGraph->Nodes[num]);
		if (BaseNode != nullptr)
		{
			if (BaseNode->PinConnected() && BaseNode->IsObjectValid())
			{
				A_Output.Append(BaseNode->GetTiles(Tile_Size, BaseNode->OverridePadding ? BaseNode->TilePadding : BaseInput->TilePadding));
			}
		}
	}
	return A_Output;
}

TArray<UTilePointer*> UMultiPackerProcessCore::TileBinPack(TArray<UTilePointer*> InputTiles, uint16 SizeVertical, uint16 SizeHorizontal, uint16 Masks)
{
	UMaxRectsBinPack* BinPack = NewObject<UMaxRectsBinPack>();
	TArray<UMaxRectsBinPack*> A_BinPack;
	BinPack->Init(SizeHorizontal, SizeVertical);
	A_BinPack.Add(BinPack);
	for (UTilePointer* Tile : InputTiles)
	{
		//New Size With Padding
		uint16 NewTileWidth = Tile->TileWidth + (Tile->TilePadding.Y * 2);
		uint16 NewTileHeight = Tile->TileHeight + (Tile->TilePadding.X * 2);
		for (uint16 Textures = 0; Textures < A_BinPack.Num(); ++Textures)
		{
			Tile->TileDatabase.SizeAndPadding = A_BinPack[Textures]->Insert(NewTileWidth, NewTileHeight, BaseInput->RectangleMethod);
			if (Tile->TileDatabase.SizeAndPadding.height != 0)
			{
				Tile->TileDatabase.Frame = Textures;//"trasitional value layer, later its ok" / Masks;
				break;
			}
		}
		if (Tile->TileDatabase.SizeAndPadding.height == 0 || Tile->TileDatabase.SizeAndPadding.height > SizeHorizontal)
		{
			UMaxRectsBinPack* NewBinPack = NewObject<UMaxRectsBinPack>();
			NewBinPack->Init(SizeHorizontal, SizeVertical);
			A_BinPack.Add(NewBinPack);
			uint16 lastIndex = A_BinPack.Num() - 1;
			Tile->TileDatabase.SizeAndPadding = A_BinPack[lastIndex]->Insert(NewTileWidth, NewTileHeight, BaseInput->RectangleMethod);
			Tile->TileDatabase.Frame = lastIndex;//"trasitional value layer, later its ok" / Masks;
		}
		//get out the padding from the data to get the new real position
		Tile->TileDatabase.SizeAndPadding = GetSizePaddingWithoutPadding(Tile->TileDatabase.SizeAndPadding, Tile->TilePadding);
	}
	TArray<UTilePointer*> TileBinPack;
	bool bAlpha = BaseInput->Alpha;
	//for loop to set every texture node on a new UTile
	for (uint16 Layer = 0; Layer < A_BinPack.Num(); ++Layer)
	{
			UTilePointer* NewTile = NewObject<UTilePointer>();
			UTextureRenderTarget2D* RT_Layer = UMultiPackerBaseEnums::GenerateRenderTarget(SizeHorizontal, SizeVertical, bAlpha);
			TArray<UTilePointer*> TileLayer;
			for (UTilePointer* Tile : InputTiles)
			{
				if (Tile->TileDatabase.Frame == Layer)
				{
					TileLayer.Add(Tile);
				}
			}
			NewTile->GenerateAndSetArrayTilesOnRenderTarget(GEditor->GetEditorWorldContext().World(), RT_Layer, TileLayer, SizeHorizontal, SizeVertical);
			TileBinPack.Add(NewTile);
	}
	//Collapse the RT via Layers if its the way
	TArray<UTilePointer*> Tiletexture;
	Tiletexture = UTilePointer::DoFinalTextures(TileBinPack, Masks, BaseInput->Alpha);
	BaseInput->NewTileData.Reset(TileBinPack.Num());
	for (UTilePointer* TileBP : TileBinPack)
	{
		for (UTilePointer* Tile : TileBP->TileBinPack)
		{
			TSharedRef<FTileDatabase> Node(new FTileDatabase());
			TSharedRef<FTileDatabase> NewTile = Node;
			NewTile->SetFromObject(Tile->TileDatabase);
			BaseInput->NewTileData.Add(NewTile);
		}
	}
	return Tiletexture;
}

FRectSizePadding UMultiPackerProcessCore::GetSizePaddingWithoutPadding(FRectSizePadding InPadding, FVector2D InTilePadding)
{
	InPadding.height = InPadding.height - (InTilePadding.X * 2);
	InPadding.width = InPadding.width - (InTilePadding.Y * 2);
	InPadding.x = InPadding.x + InTilePadding.X;
	InPadding.y = InPadding.y + InTilePadding.Y;
	return InPadding;
}

TArray<FString> UMultiPackerProcessCore::TexturePackageName(FString Prefix, FString TextureNum, FAssetToolsModule& AssetToolsModule)
{
	FString importDirectory = BaseInput->TargetDirectory.Path;
	FString ObjectName = BaseInput->TargetName;
	// last minute sanitizing, just in case we missed one
	ObjectName = ObjectName.Replace(TEXT("*"), TEXT("X"));
	ObjectName = ObjectName.Replace(TEXT("?"), TEXT("Q"));
	ObjectName = ObjectName.Replace(TEXT("!"), TEXT("I"));
	ObjectName = ObjectName.Replace(TEXT("."), TEXT("-"));
	ObjectName = ObjectName.Replace(TEXT("&"), TEXT("_"));
	ObjectName = ObjectName.Replace(TEXT(" "), TEXT("_"));
	// last minute sanitizing, just in case we missed one
	Prefix = Prefix.Replace(TEXT("*"), TEXT("X"));
	Prefix = Prefix.Replace(TEXT("?"), TEXT("Q"));
	Prefix = Prefix.Replace(TEXT("!"), TEXT("I"));
	Prefix = Prefix.Replace(TEXT("."), TEXT("-"));
	Prefix = Prefix.Replace(TEXT("&"), TEXT("_"));
	Prefix = Prefix.Replace(TEXT(" "), TEXT("_"));
	FString TextureName = ObjectName;
	if (ObjectName == "None")
		TextureName = "MultiPacker";
	FString NPrefix = Prefix;
	TextureName = NPrefix.Append(TextureName);
	if (TextureNum != "")
		TextureName = TextureName.Append(TextureNum);
	if (TextureName.Contains("/"))
		TextureName = TextureName.Mid(TextureName.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);
	FString NewPackageName = importDirectory.EndsWith(TEXT("/"), ESearchCase::IgnoreCase) ? importDirectory : importDirectory + "/";
	NewPackageName = NewPackageName + TextureName;
	if (!NewPackageName.StartsWith(TEXT("/Game/"), ESearchCase::IgnoreCase))
	{
		NewPackageName = TEXT("/Game/") + NewPackageName;
	}
	FString Name;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(NewPackageName, TextureName, PackageName, Name);
	TArray<FString> NamesOut;
	NamesOut.Add(TextureName);
	NamesOut.Add(PackageName);
	return NamesOut;
}

void UMultiPackerProcessCore::SetDataLinearColors(EChannelOutput ChannelOutput, FLinearColor &Channel, FLinearColor &Layer)
{
	Channel = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	Layer = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	switch (BaseInput->ChannelMethod)
	{
	case EChannelTextureSave::CS_Multiple://Multi
		switch (ChannelOutput)
		{
		case EChannelOutput::Channel_Red_1://Red1
			Channel = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			Layer = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Red_2://Red2
			Channel = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			Layer = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Red_3://Red3
			Channel = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			Layer = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Green_1://Green1
			Channel = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			Layer = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Green_2://Green2
			Channel = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			Layer = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Green_3://Green3
			Channel = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			Layer = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Blue_1://Blue1
			Channel = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			Layer = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Blue_2://Blue2
			Channel = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			Layer = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Blue_3://Blue3
			Channel = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			Layer = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Alpha_1://Alpha1
			Channel = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Layer = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Alpha_2://Alpha2
			Channel = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Layer = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Alpha_3://Alpha3
			Channel = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			Layer = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		}
		break;
	case EChannelTextureSave::CS_OneSdf://One_SDF
	case EChannelTextureSave::CS_One://One
		switch (ChannelOutput)
		{
		case EChannelOutput::Channel_Red://Red
			Channel = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Green://Green
			Channel = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Blue://Blue
			Channel = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		case EChannelOutput::Channel_Alpha://Alpha
			Channel = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		}
		break;
	case EChannelTextureSave::CS_Atlas://Atlas_RGB-A
		break;
	}
}
