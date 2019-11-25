/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "MultiPackerAssetEditor/SAssetDropTargetArray.h"

class SMultiPackerAssetEditorDrop : public SAssetDropTargetArray
{
public:
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FVector2D GetPanelCoordDropPosition() const { return PanelCoordDropPosition; }

private:
	FVector2D PanelCoordDropPosition;
};