/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "SGraphNode.h"
#include "MultiPackerAssetEditor/Nodes/MultiPackerOutputNode.h"

class SMultiPackerOutputNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SMultiPackerOutputNode) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UMultiPackerOutputNode* InNode);

	//~ Begin SGraphNode Interface
	//virtual void UpdateGraphNode() override;
	void CreatePinWidgets() override;
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// 	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// 	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// 	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// 	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	// 	virtual FReply OnMouseMove(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;
	// 	virtual void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;
	// 	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	//~ End SGraphNode Interface
	
	EVisibility GetDragOverMarkerVisibility() const;

	FText GetDescription() const;
	EVisibility GetDescriptionVisibility() const;
	
	const FSlateBrush* GetNameIcon() const;

protected:
	TSharedRef<SWidget> CreateNodeContentArea() override;
	TSharedRef<SWrapBox> CreateThumbnailContentArea();
	TSharedPtr<SBorder> NodeBody;
	TSharedPtr<SHorizontalBox> OutputPinBox;

	UMultiPackerOutputNode* EdActorNode;
};
