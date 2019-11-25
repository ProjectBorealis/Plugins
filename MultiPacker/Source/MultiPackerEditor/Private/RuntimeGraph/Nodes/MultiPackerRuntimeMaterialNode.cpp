/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/Nodes/MultiPackerRuntimeMaterialNode.h"
#include "MultiPackerRuntimeMatNode.h"
#include "MultiPackerEditorTypes.h"
#include <EdGraph/EdGraphPin.h>
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "MultiPackerRuntimeMaterialNode"

void UMultiPackerRuntimeMaterialNode::AllocateDefaultPins()
{
	FEdGraphPinType newPin = FEdGraphPinType();
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 18)
	newPin.PinCategory = UMultiPackerEditorTypes::SPinMaterialNode;
	CreatePin(EGPD_Output, newPin, FString("Out"), 0);
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 18)
	newPin.PinCategory = FName(*UMultiPackerEditorTypes::SPinMaterialNode);
	CreatePin(EGPD_Output, newPin, FName("Out"), 0);
#endif
}

void UMultiPackerRuntimeMaterialNode::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
}

UMultiPackerRuntimeEdGraph* UMultiPackerRuntimeMaterialNode::GetGenericGraphEdGraph()
{
	return Cast<UMultiPackerRuntimeEdGraph>(GetGraph());
}

FText UMultiPackerRuntimeMaterialNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (MultiPackerRuntimeMatNode == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		MultiPackerRuntimeMatNode->ChangeBackground(false);
		return FText::FromString(MultiPackerRuntimeMatNode->MaterialBaseInput ? MultiPackerRuntimeMatNode->MaterialBaseInput->GetName() : "Empty");
	}
}

void UMultiPackerRuntimeMaterialNode::SetGenericGraphNode(UMultiPackerRuntimeMatNode* InNode)
{
	MultiPackerRuntimeMatNode = InNode;
}

FString UMultiPackerRuntimeMaterialNode::GetNodeTitle()
{
	return (MultiPackerRuntimeMatNode->MaterialBaseInput ? MultiPackerRuntimeMatNode->MaterialBaseInput->GetName() : "Empty");
}

bool UMultiPackerRuntimeMaterialNode::SetThumbnail()
{
	return true;
}

void UMultiPackerRuntimeMaterialNode::SetMaterialInput(UMaterialInterface* Material)
{
	MultiPackerRuntimeMatNode->MaterialBaseInput = Material;
}

FText UMultiPackerRuntimeMaterialNode::GetDescription() const
{
	return FText::FromString("Texture");
}

UObject* UMultiPackerRuntimeMaterialNode::GetThumbnailAssetObject()
{
	return MultiPackerRuntimeMatNode->MaterialBaseInput;
}

UObject* UMultiPackerRuntimeMaterialNode::GetNodeAssetObject(UObject* Outer)
{
	return  MultiPackerRuntimeMatNode->MaterialBaseInput;
}

FLinearColor UMultiPackerRuntimeMaterialNode::GetBackgroundColor() const
{
	return MultiPackerRuntimeMatNode->GetBackground();
}

#undef LOCTEXT_NAMESPACE