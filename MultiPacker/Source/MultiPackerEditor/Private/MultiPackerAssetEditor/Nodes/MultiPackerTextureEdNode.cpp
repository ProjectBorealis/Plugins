/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/MultiPackerTextureEdNode.h"
#include "Graph/MultiPackerTextureNode.h"
#include "MultiPackerEditorTypes.h"
#include "EdGraph/EdGraphPin.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MultiPackerBaseEnums.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"
#include "MultiPackerAssetEditor/MultiPackerEdGraph.h"
#include "TileUtils/TilePointer.h"

#define LOCTEXT_NAMESPACE "MultiPackerTextureEdNode"

UMultiPackerTextureEdNode::UMultiPackerTextureEdNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMultiPackerTextureEdNode::AllocateDefaultPins()
{
	FEdGraphPinType newPin = FEdGraphPinType();
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::PinCategory_RGB);
	CreatePin(EGPD_Output, newPin, FName(*UMultiPackerEditorTypes::PinCategory_RGB), 0);
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::PinCategory_Red);
	CreatePin(EGPD_Output, newPin, FName(*UMultiPackerEditorTypes::PinCategory_Red), 1);
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::PinCategory_Green);
	CreatePin(EGPD_Output, newPin, FName(*UMultiPackerEditorTypes::PinCategory_Green), 2);
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::PinCategory_Blue);
	CreatePin(EGPD_Output, newPin, FName(*UMultiPackerEditorTypes::PinCategory_Blue), 3);
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::PinCategory_Alpha);
	CreatePin(EGPD_Output, newPin, FName(*UMultiPackerEditorTypes::PinCategory_Alpha), 4);
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::PinCategory_RGBA);
	CreatePin(EGPD_Output, newPin, FName(*UMultiPackerEditorTypes::PinCategory_RGBA), 5);
}

void UMultiPackerTextureEdNode::PinConnectionListChanged(UEdGraphPin * Pin)
{
	for (int32 pinI = 0; pinI < Pins.Num(); ++pinI)
	{
		if ((Pins[pinI]->LinkedTo.Num() > 0)&& Pins[pinI] != Pin)
		{
			Pins[pinI]->BreakLinkTo(Pins[pinI]->LinkedTo[0]);
		}
	}
	TArray<EChannelSelectionInput> channels;
	channels.Add(EChannelSelectionInput::CSI_RGB);
	channels.Add(EChannelSelectionInput::CSI_Red);
	channels.Add(EChannelSelectionInput::CSI_Green);
	channels.Add(EChannelSelectionInput::CSI_Blue);
	channels.Add(EChannelSelectionInput::CSI_Alpha);
	channels.Add(EChannelSelectionInput::CSI_RGBA);
	for (int32 pinI = 0; pinI < Pins.Num(); ++pinI)
	{
		if (Pins[pinI] == Pin)
		{
			SetChannelInput(channels[pinI]);
			break;
		}
	}
	Super::PinConnectionListChanged(Pin);
}

bool UMultiPackerTextureEdNode::PinConnected()
{
	TArray<EChannelSelectionInput> channels;
	channels.Add(EChannelSelectionInput::CSI_RGB);
	channels.Add(EChannelSelectionInput::CSI_Red);
	channels.Add(EChannelSelectionInput::CSI_Green);
	channels.Add(EChannelSelectionInput::CSI_Blue);
	channels.Add(EChannelSelectionInput::CSI_Alpha);
	channels.Add(EChannelSelectionInput::CSI_RGBA);
	for (int32 pinI = 0; pinI < Pins.Num(); ++pinI)
	{
		if (Pins[pinI]->LinkedTo.Num() > 0)
		{
			SetChannelInput(channels[pinI]);
			return true;
		}
	}
	return false;
}


bool UMultiPackerTextureEdNode::IsObjectValid()
{
	return (GenericGraphNode->TextureInput != nullptr);
}

FText UMultiPackerTextureEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (GenericGraphNode == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		GenericGraphNode->ChangeBackground(!GenericGraphNode->NodeCleanErrors());
		return FText::FromString(GenericGraphNode->GetNodeTitle());
	}
}

void UMultiPackerTextureEdNode::SetGenericGraphNode(UMultiPackerTextureNode* InNode)
{
	GenericGraphNode = InNode;
}

void UMultiPackerTextureEdNode::SetTextureInput(UTexture* Texture)
{
	GenericGraphNode->TextureInput = Texture;
}

UObject* UMultiPackerTextureEdNode::GetThumbnailAssetObject()
{
	return GenericGraphNode->TextureInput;
}

UObject* UMultiPackerTextureEdNode::GetNodeAssetObject(UObject* Outer)
{
	return  GenericGraphNode->TextureInput;
}

FLinearColor UMultiPackerTextureEdNode::GetBackgroundColor() const
{
	return GenericGraphNode->GetBackground();
}

#if WITH_EDITOR  
void UMultiPackerTextureEdNode::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerTextureEdNode, ChannelInput)))
	{
		BreakAllNodeLinks();
		UMultiPackerOutputNode* EdNode = NULL;
		UMultiPackerEdGraph* EdGraph = GetGenericGraphEdGraph();
		for (int i = 0; i < EdGraph->Nodes.Num(); ++i)
		{
			EdNode = Cast<UMultiPackerOutputNode>(EdGraph->Nodes[i]);
			if (EdNode != nullptr)
			{
				break;
			}
		}
		FString PinName;
		switch (ChannelInput)
		{
		default:
			PinName = UMultiPackerEditorTypes::PinCategory_RGB;
			break;
		case EChannelSelectionInput::CSI_RGB://rgb
			PinName = UMultiPackerEditorTypes::PinCategory_RGB;
			break;
		case EChannelSelectionInput::CSI_Red://red
			PinName = UMultiPackerEditorTypes::PinCategory_Red;
			break;
		case EChannelSelectionInput::CSI_Green://green
			PinName = UMultiPackerEditorTypes::PinCategory_Green;
			break;
		case EChannelSelectionInput::CSI_Blue://blue
			PinName = UMultiPackerEditorTypes::PinCategory_Blue;
			break;
		case EChannelSelectionInput::CSI_Alpha://alpha
			PinName = UMultiPackerEditorTypes::PinCategory_Alpha;
			break;
		case EChannelSelectionInput::CSI_RGBA://rgba
			PinName = UMultiPackerEditorTypes::PinCategory_RGBA;
			break;
		}
		EdNode->FindPin(FName("Output"), EGPD_Input)->MakeLinkTo(FindPin(FName(*PinName), EGPD_Output));
	}
	PrepareTextureToWork();
}

void UMultiPackerTextureEdNode::ProcessTiles()
{
	//create Initial Tile and array container
	TArray<UTilePointer*> ArrayTiles;
	UTilePointer* NewTile = NewObject<UTilePointer>(UTilePointer::StaticClass());
	//set the data from the Node UTexture
	UTexture2D* Texture = Cast<UTexture2D>(GenericGraphNode->TextureInput);
	if (Texture != nullptr)
	{
		NewTile->GenerateFromTexture(Texture, Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());
	}
	else
	{
		UTextureRenderTarget2D* RenderTarget = Cast<UTextureRenderTarget2D>(GenericGraphNode->TextureInput);
		if (RenderTarget != nullptr)
		{
			NewTile->GenerateFromRT(RenderTarget, RenderTarget->GetSurfaceWidth(), RenderTarget->GetSurfaceHeight());
		}
		else
		{
			//error
		}
	}
	Tiles.Reset();
	//channel selection input and basic resolution
	UTilePointer* Copy = NewObject<UTilePointer>(UTilePointer::StaticClass());
	NewTile->FromChannelToTexture(ChannelInput);
	Copy->ChangeResolution(GetTileSize().X, GetTileSize().Y, NewTile);
	Tiles.Add(Copy);// = ArrayTiles;
	//split stage
	if ((GenericGraphNode->TilesVertical * GenericGraphNode->TilesHorizontal) > 1)
	{
		Tiles = ArrayTiles = Copy->SplitTile(GenericGraphNode->TilesVertical, GenericGraphNode->TilesHorizontal);
	}
	else
	{
		ArrayTiles.Add(Copy);
	}
	//sdf stage
	if (IsTilesSDF())
	{
		for (UTilePointer* Tile : ArrayTiles)
		{
			//Tile->ChangeResolution(128, 128, Tile);
			Tile->SDFGenParallel(GenericGraphNode->SDF_Radius, GetTileSize().Y, GetTileSize().X);
		}
	}
	//Tile Size Final
	for (UTilePointer* Tile : ArrayTiles)
	{
		if (GetSplitTileSize() != FVector2D(Tile->TileWidth, Tile->TileHeight))
		{
			Tile->ChangeResolution(GetSplitTileSize().X, GetSplitTileSize().Y, Tile);
		}
	}
	SetDatabase();//Info database from the tiles to the final process
	//thumbnails = Tiles
	GenericGraphNode->SetDataBaseTiles(ArrayTiles);
}

FVector2D UMultiPackerTextureEdNode::GetTileSize()
{
	if (GenericGraphNode->RectangleSize)
	{
		return FVector2D((uint16)(GenericGraphNode->SizeHorizontal), (uint16)(GenericGraphNode->SizeVertical));
	}
	return FVector2D(SizeTile * GenericGraphNode->TilesHorizontal, SizeTile * GenericGraphNode->TilesVertical);
}

FVector2D UMultiPackerTextureEdNode::GetSplitTileSize()
{
	if (GenericGraphNode->RectangleSize)
	{
		return FVector2D((uint16)(GenericGraphNode->SizeHorizontal / GenericGraphNode->TilesHorizontal), (uint16)(GenericGraphNode->SizeVertical / GenericGraphNode->TilesVertical));
	}
	return FVector2D(SizeTile, SizeTile);
}

bool UMultiPackerTextureEdNode::GetMsdf()
{
	return GenericGraphNode->MSDF;
}

bool UMultiPackerTextureEdNode::IsNodeSelectedSdf()
{
	return GenericGraphNode->SDF;
}

void UMultiPackerTextureEdNode::PrepareTextureToWork()
{
	if (GenericGraphNode->TextureInput != nullptr)
	{
		GenericGraphNode->TextureInput->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		GenericGraphNode->TextureInput->SRGB = 0;
		GenericGraphNode->TextureInput->Filter = TextureFilter::TF_Nearest;
		GenericGraphNode->TextureInput->UpdateResource();
	}
}

#endif

FTileThumbDatabase UMultiPackerTextureEdNode::GetTileThumbDatabase(uint16 num)
{
	return GenericGraphNode->ATexTileData[num];
}

uint16 UMultiPackerTextureEdNode::GetNumberTiles()
{
	return GenericGraphNode->TilesHorizontal * GenericGraphNode->TilesVertical;
}

void UMultiPackerTextureEdNode::SetChannelInput(EChannelSelectionInput Input)
{
	ChannelInput = Input;
}

EChannelSelectionInput UMultiPackerTextureEdNode::GetChannelInput() const
{
	return ChannelInput;
}
#undef LOCTEXT_NAMESPACE
