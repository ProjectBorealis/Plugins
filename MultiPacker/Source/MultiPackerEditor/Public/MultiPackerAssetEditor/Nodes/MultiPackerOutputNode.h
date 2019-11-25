/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "MultiPackerOutputNode.generated.h"

class FAssetThumbnail;
class UMultiPackerOutputNodeBase;
class UMultiPackerEdGraph;
class UTexture2D;

UCLASS(MinimalAPI)
class UMultiPackerOutputNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	void ProcessArrayThumbnail();
	void AllocateDefaultPins() override;
	void NodeConnectionListChanged() override;
	bool CanUserDeleteNode()const override;

	void SetArrayThumnailOutput(TArray<UTexture2D*> Input);
	int GetNumTexturesArray();
	TSharedRef<SWidget> GetThumbnailByNum(int Num);
	bool IsThumbRectangled(int Num);
private:
	UMultiPackerEdGraph* GetGenericGraphEdGraph() const;
	virtual FLinearColor GetNodeTitleColor() const override;
	static UEdGraphPin* PinDefault;
	bool NodeOutput = false;
	//OutputAssets
	UPROPERTY()
	TArray<UTexture2D*> ArrayTextureOutput;
	TArray<bool> ArrayRectangled;
	TArray<TSharedPtr<class FAssetThumbnail>> ArrayAssetThumbnail;
};