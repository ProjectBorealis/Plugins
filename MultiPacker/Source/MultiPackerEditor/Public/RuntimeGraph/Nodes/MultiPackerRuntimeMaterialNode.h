/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "MultiPackerRuntimeMaterialNode.generated.h"

class UMultiPackerRuntimeMatNode;
class FAssetThumbnail;
class UMultiPackerRuntimeEdGraph;

UCLASS(MinimalAPI)
class UMultiPackerRuntimeMaterialNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	TSharedPtr<class FAssetThumbnail> AssetThumbnail;

	UPROPERTY(VisibleAnywhere, instanced, Category = "MultiPacker")
		UMultiPackerRuntimeMatNode* MultiPackerRuntimeMatNode;

	virtual void AllocateDefaultPins() override;
	virtual void NodeConnectionListChanged() override;

	UMultiPackerRuntimeEdGraph* GetGenericGraphEdGraph();

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const;

	void SetGenericGraphNode(UMultiPackerRuntimeMatNode* InNode);
		
	FString GetNodeTitle();
	
	bool SetThumbnail();

	void SetMaterialInput(UMaterialInterface* Material);
	
	virtual FText GetDescription() const;

	virtual UObject* GetNodeAssetObject(UObject* Outer);
	virtual UObject* GetThumbnailAssetObject();

	virtual FLinearColor GetBackgroundColor() const;
};

