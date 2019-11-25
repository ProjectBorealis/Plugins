/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"
#include "MultiPackerEditorTypes.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "Runtime/Launch/Resources/Version.h"
#include <EdGraph/EdGraphPin.h>
#include <UObject/UObjectGlobals.h>
#include <GraphEditorSettings.h>
#include <Engine/Texture2D.h>
#include "AssetThumbnail.h"

#define LOCTEXT_NAMESPACE "MultiPackerOutputEdNode"

void UMultiPackerOutputNode::AllocateDefaultPins()
{
	FEdGraphPinType newPin = FEdGraphPinType();
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	newPin.PinCategory = UMultiPackerEditorTypes::SPinOutputNode;
	CreatePin(EGPD_Input, newPin, FString("Output"), 0);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::SPinOutputNode);
	CreatePin(EGPD_Input, newPin, FName("Output"), 0);
#endif
}

void UMultiPackerOutputNode::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
	GetGenericGraphEdGraph()->MultiPackerNode = true;
}

UMultiPackerEdGraph* UMultiPackerOutputNode::GetGenericGraphEdGraph() const
{
	return Cast<UMultiPackerEdGraph>(GetGraph());
}

bool UMultiPackerOutputNode::CanUserDeleteNode() const
{
	return NodeOutput;
}

void UMultiPackerOutputNode::SetArrayThumnailOutput(TArray<UTexture2D*> Input)
{
	ArrayTextureOutput = Input;
	ProcessArrayThumbnail();
}

int UMultiPackerOutputNode::GetNumTexturesArray()
{
	return ArrayTextureOutput.Num();
}

TSharedRef<SWidget> UMultiPackerOutputNode::GetThumbnailByNum(int Num)
{
	//something inside me say this is a bomb waiting to explode
	return ArrayAssetThumbnail[Num].Get()->MakeThumbnailWidget();
}

bool UMultiPackerOutputNode::IsThumbRectangled(int Num)
{
	return ArrayRectangled[Num];
}

FLinearColor UMultiPackerOutputNode::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->ResultNodeTitleColor;
}

void UMultiPackerOutputNode::ProcessArrayThumbnail()
{
	ArrayAssetThumbnail.Empty(ArrayTextureOutput.Num());
	ArrayRectangled.Empty(ArrayTextureOutput.Num());
	for (UTexture2D* Texture : ArrayTextureOutput)
	{
		FAssetData ValueAsset = FAssetData(Texture);
		bool TextureRectangled = Texture->GetSurfaceWidth() != Texture->GetSurfaceHeight() ? true : false;
		ArrayRectangled.Add(TextureRectangled);
		ArrayAssetThumbnail.Add(MakeShareable(new FAssetThumbnail(ValueAsset, TextureRectangled ? 64 : 128, 128, FMultiPackerEditorThumbnail::Get())));
	}
}

#undef LOCTEXT_NAMESPACE