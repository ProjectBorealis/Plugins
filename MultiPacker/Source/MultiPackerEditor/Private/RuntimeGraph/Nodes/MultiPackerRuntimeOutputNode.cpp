/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/Nodes/MultiPackerRuntimeOutputNode.h"
#include "MultiPackerEditorTypes.h"
#include "MultiPackerAssetEditor/MultiPackerEditorThumbnail.h"
#include "Runtime/Launch/Resources/Version.h"
#include <EdGraph/EdGraphPin.h>
#include <Engine/Texture2D.h>
#include "RuntimeGraph/MultiPackerRuntimeEdGraph.h"

#define LOCTEXT_NAMESPACE "MultiPackerOutputEdNode"

void UMultiPackerRuntimeOutputNode::AllocateDefaultPins()
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

void UMultiPackerRuntimeOutputNode::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
	Cast<UMultiPackerRuntimeEdGraph>(GetGraph())->MultiPackerNode = true;
}

bool UMultiPackerRuntimeOutputNode::CanUserDeleteNode() const
{
	return false;
}

void UMultiPackerRuntimeOutputNode::SetThumnailOutput(UTexture2D* Input)
{
	if (Input == nullptr)
		Input = TextureInput;

	if (Input)
		TextureInput = Input;

	AssetThumbnail = MakeShareable(new FAssetThumbnail(TextureInput, 128, 128, FMultiPackerEditorThumbnail::Get()));
}
#undef LOCTEXT_NAMESPACE