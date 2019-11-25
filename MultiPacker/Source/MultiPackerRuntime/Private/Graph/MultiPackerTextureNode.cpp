/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "Graph/MultiPackerTextureNode.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TileUtils/TilePointer.h"
#include "Graph/MultiPacker.h"

#define LOCTEXT_NAMESPACE "MultiPackerNode"

UMultiPackerTextureNode::UMultiPackerTextureNode()
{
	//BackgroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
	ChangeBackground(false);
	FTileThumbDatabase newTile;
	newTile.TileName = FName("None");
	newTile.TileTexture = nullptr;
	ATexTileData.Add(newTile);
	ErrorSize = RectangleSize ? CompareSizes(UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY), UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY)) : false;
}

UMultiPackerTextureNode::~UMultiPackerTextureNode()
{
}

FString UMultiPackerTextureNode::GetNodeTitle() const
{
	return TextureInput ? TextureInput->GetName() : "Empty";
}

bool UMultiPackerTextureNode::CanSetThumbnail() const
{
	return TextureInput != NULL;
}

UMultiPacker* UMultiPackerTextureNode::GetGraph() const
{
	return Cast<UMultiPacker>(GetOuter());
}

bool UMultiPackerTextureNode::NodeCleanErrors() const
{
	if (ErrorSize && RectangleSize)
	{
		return false;
	}

	if (ErrorTiles)
	{
		return false;
	}

	if (!TextureInput->IsValidLowLevel() | !(TilesHorizontal > 0) | !(TilesVertical > 0))
	{  
		return false;
	}

	return true;
}

bool UMultiPackerTextureNode::CompareSizes(int32 Vertical, int32 Horizontal)
{
	ErrorSize = false;
	if (SizeVertical / TilesVertical > Vertical)
		ErrorSize = true;
	if (SizeHorizontal / TilesHorizontal > Horizontal)
		ErrorSize = true;
	return ErrorSize;
}

bool UMultiPackerTextureNode::CanProcess() const
{
	return BackgroundColor == FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

#if WITH_EDITOR  
void UMultiPackerTextureNode::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerTextureNode, AutoSizeFill)))
	{
		switch (AutoSizeFill)
		{
		case ESizeTexture::EMCE_Option1:
			break;
		case ESizeTexture::EMCE_Option2:
			AutoSizeFill = ESizeTexture::EMCE_Option1;
			SizeVertical = TextureInput->GetSurfaceHeight();
			SizeHorizontal = TextureInput->GetSurfaceWidth();
			break;
		default: 
			break;
		}
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerTextureNode, TextureInput)))
	{
		ChangeBackground(TextureInput == nullptr ? true : false);
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerTextureNode, AutoNameFill)))
	{
		FString Name;
		switch (AutoNameFill)
		{
		case ENameTexture::EMCE_Option1:
			break;
		case ENameTexture::EMCE_Option2:
			AutoNameFill = ENameTexture::EMCE_Option1;
			Name = TextureInput->GetName().Left(16);
			for (int data = 0; data < ATexTileData.Num(); ++data)
			{
				FString numName = Name;
				if (ATexTileData.Num() > 1)
				{
					numName = TextureInput->GetName().Left(14).Append("_" + FString::FromInt(data));
				}
				ATexTileData[data].TileName = FName(*numName);
			}
			break;
		case ENameTexture::EMCE_Option3:
			AutoNameFill = ENameTexture::EMCE_Option1;
			Name = TextureInput->GetName().Left(10).Append("_Alpha");
			for (int data = 0; data < ATexTileData.Num(); ++data)
			{
				FString numNameA = Name;
				if (ATexTileData.Num() > 1)
				{
					numNameA = TextureInput->GetName().Left(8).Append("_" + FString::FromInt(data) + "_Alpha");
				}
				ATexTileData[data].TileName = FName(*numNameA);
			}
			break;
		default:
			AutoNameFill = ENameTexture::EMCE_Option1;
			break;
		}
	}
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerTextureNode, SetSizeByParameter)))
	{
		SizeVertical = SizeHorizontal = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(SetSizeByParameter);
	}
	for (int tile = 0; tile < ATexTileData.Num(); ++tile)
	{
		ATexTileData[tile].TileName = FName(*(ATexTileData[tile].TileName.ToString().Left(16)));
	}
	//combined method to know if the tile size dont fit and its a error
	ErrorSize = RectangleSize ? CompareSizes(UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY), UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY)) : false;
	numTiles = TilesVertical * TilesHorizontal;
	if (ATexTileData.Num()>0)
	{
		for (int t = 0; t <= numTiles; ++t)
		{
			if (t>ATexTileData.Num())
			{
				FTileThumbDatabase newTile;
				newTile.TileName = FName("None");
				newTile.TileTexture = nullptr;
				ATexTileData.Add(newTile);
			}
		}
		if (ATexTileData.Num() > numTiles)
		{
			TArray<FTileThumbDatabase> A_NewTexTileData;
			for (int t = 0; t < numTiles; ++t)
			{
				A_NewTexTileData.Add(ATexTileData[t]);
			}
			ATexTileData = A_NewTexTileData;
		}
	}
	else
	{
		for (int t = 0; t < numTiles; ++t)
		{
			FTileThumbDatabase newTile;
			newTile.TileName = FName("None");
			newTile.TileTexture = nullptr;
			ATexTileData.Add(newTile);
		}
	}
}
#endif

void UMultiPackerTextureNode::SetErrorTiles(bool error)
{
	ErrorTiles = error;
}

void UMultiPackerTextureNode::SetDataBaseTiles(TArray<UTextureRenderTarget2D*> RT_Array)
{
	for (int tile = 0; tile < ATexTileData.Num() ; ++tile)
	{
		ATexTileData[tile].TileTexture = RT_Array[tile];
	}
}

void UMultiPackerTextureNode::SetDataBaseTiles(TArray<UTilePointer*> TileThumbs)
{
	Thumbnails = TileThumbs;
	for (int tile = 0; tile < ATexTileData.Num(); ++tile)
	{
		ATexTileData[tile].TileTexture = TileThumbs[tile]->TileTexture;
	}
}

void UMultiPackerTextureNode::ChangeBackground(bool error)
{
	BackgroundColor = error ? FLinearColor(1.0f, 0.0f, 0.0f, 1.0f) : FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

FLinearColor UMultiPackerTextureNode::GetBackground() const
{
	return BackgroundColor;
}

#undef LOCTEXT_NAMESPACE


