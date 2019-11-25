/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "MultiPackerRuntimeOutputNode.generated.h"

class UMultiPackerRuntimeEdGraph;
class FAssetThumbnail;
class UTexture2D;

UCLASS(MinimalAPI)
class UMultiPackerRuntimeOutputNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	TSharedPtr<class FAssetThumbnail> AssetThumbnail;

	void SetThumnailOutput(UTexture2D* Input);
	void AllocateDefaultPins() override;
	void NodeConnectionListChanged() override;
	bool CanUserDeleteNode()const override;

private:
	UTexture2D* TextureInput;
};