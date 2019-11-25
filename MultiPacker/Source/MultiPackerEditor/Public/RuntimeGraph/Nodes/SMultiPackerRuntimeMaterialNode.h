/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "RuntimeGraph/Nodes/MultiPackerRuntimeMaterialNode.h"

class SMultiPackerRuntimeMaterialNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SMultiPackerRuntimeMaterialNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMultiPackerRuntimeMaterialNode* InNode);


	//~ Begin SGraphNode Interface
	//virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// 	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// 	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// 	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// 	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	// 	virtual FReply OnMouseMove(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;
	// 	virtual void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;
	// 	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	//~ End SGraphNode Interface

	virtual FSlateColor GetErrorColor() const;
	virtual FText GetErrorMsgToolTip() const;
	virtual FSlateColor GetBorderBackgroundColor() const;
	virtual FSlateColor GetBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	virtual FText GetDescription() const;
	virtual EVisibility GetDescriptionVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

protected:
	virtual TSharedRef<SWidget> CreateNodeContentArea() override;
	TSharedPtr<SBorder> NodeBody;
	TSharedPtr<SHorizontalBox> OutputPinBox;

	UMultiPackerRuntimeMaterialNode* EdActorNode;
	FIntPoint ThumbnailSize;
private:
	void BuildThumbnailWidget();

	TSharedPtr<SErrorText> ErrorText;
};

