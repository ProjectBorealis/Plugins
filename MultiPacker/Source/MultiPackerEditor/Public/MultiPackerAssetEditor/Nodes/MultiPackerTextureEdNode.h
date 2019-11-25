/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "MultiPackerAssetEditor/Nodes/MultiPackerBaseNode.h"
#include "MultiPackerTextureEdNode.generated.h"

class UMultiPackerTextureNode;

UCLASS(MinimalAPI)
class UMultiPackerTextureEdNode : public UMultiPackerBaseNode
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, instanced, Category = "MultiPacker")
	UMultiPackerTextureNode* GenericGraphNode;

	UPROPERTY(EditAnywhere, Category = "TextureNode")
	EChannelSelectionInput ChannelInput = EChannelSelectionInput::CSI_RGB;
	
	void AllocateDefaultPins() override;
	void PinConnectionListChanged(UEdGraphPin * Pin) override;
	bool PinConnected() override;
	bool IsObjectValid() override;
	FTileThumbDatabase GetTileThumbDatabase(uint16 num) override;
	uint16 GetNumberTiles() override;

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const;
	void SetGenericGraphNode(UMultiPackerTextureNode* InNode);
	void SetTextureInput(UTexture* Texture);

	void SetChannelInput(EChannelSelectionInput Input);
	EChannelSelectionInput GetChannelInput() const;
	UObject* GetNodeAssetObject(UObject* Outer);
	UObject* GetThumbnailAssetObject();
	FLinearColor GetBackgroundColor() const;

#if WITH_EDITOR  
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	//TilePointer Side class
	void ProcessTiles() override;
	FVector2D GetTileSize();
	FVector2D GetSplitTileSize();
#endif

	bool GetMsdf() override;
	bool IsNodeSelectedSdf() override;
private:

	void PrepareTextureToWork();
};
