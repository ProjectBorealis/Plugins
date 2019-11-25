/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/SMultiPackerAssetEditorDrop.h"

FReply SMultiPackerAssetEditorDrop::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	PanelCoordDropPosition = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
	return SAssetDropTargetArray::OnDrop(MyGeometry, DragDropEvent);
}

void SMultiPackerAssetEditorDrop::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SAssetDropTargetArray::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Enable mouse integration while dragging
	if (GetDragOverlayVisibility().IsVisible())
	{
		SetVisibility(EVisibility::Visible);
	}
	else
	{
		SetVisibility(EVisibility::SelfHitTestInvisible);
	}
}
