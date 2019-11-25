/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/SAssetDropTargetArray.h"
#include "AssetData.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "DragAndDrop/ActorDragDropOp.h"
#include "AssetSelection.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "EditorWidgets"

void SAssetDropTargetArray::Construct(const FArguments& InArgs)
{
	OnAssetDropped = InArgs._OnAssetDropped;
	OnIsAssetAcceptableForDrop = InArgs._OnIsAssetAcceptableForDrop;

	SDropTarget::Construct(
		SDropTarget::FArguments()
		.OnDrop(this, &SAssetDropTargetArray::OnDropped)
		[
			InArgs._Content.Widget
		]);
}

uint8 SAssetDropTargetArray::GetRow()
{
	return RowAssetDrop;
}

uint8 SAssetDropTargetArray::GetColumn()
{
	return ColumnAssetDrop;
}

FReply SAssetDropTargetArray::OnDropped(TSharedPtr<FDragDropOperation> DragDropOperation)
{
	bool bUnused;
	TArray<UObject*> Objects = GetDroppedObject(DragDropOperation, bUnused);
	ArrayObjectNum = 0;
	ArrayObjectDropTotal = Objects.Num();
	int MatrixSize = FMath::CeilToInt(FMath::Sqrt(ArrayObjectDropTotal));//get the number of columns and rows
	for (UObject* Object :Objects)
	{
		ColumnAssetDrop = ArrayObjectNum / MatrixSize;
		RowAssetDrop = ArrayObjectNum - (ColumnAssetDrop * MatrixSize);
		OnAssetDropped.ExecuteIfBound(Object);
		ArrayObjectNum++;
	}

	return FReply::Handled();
}

bool SAssetDropTargetArray::OnAllowDrop(TSharedPtr<FDragDropOperation> DragDropOperation) const
{
	bool bUnused = false;
	TArray<UObject*> Objects = GetDroppedObject(DragDropOperation, bUnused);
	for (UObject* Object : Objects)
	{
		// Check and see if its valid to drop this object
		if (OnIsAssetAcceptableForDrop.IsBound())
		{
			return OnIsAssetAcceptableForDrop.Execute(Object);
		}
		else
		{
			// If no delegate is bound assume its always valid to drop this object
			return true;
		}
	}

	return false;
}

bool SAssetDropTargetArray::OnIsRecognized(TSharedPtr<FDragDropOperation> DragDropOperation) const
{
	bool bRecognizedEvent = false;
	TArray<UObject*> Object = GetDroppedObject(DragDropOperation, bRecognizedEvent);

	return bRecognizedEvent;
}
	
TArray<UObject*> SAssetDropTargetArray::GetDroppedObject(TSharedPtr<FDragDropOperation> DragDropOperation, bool& bOutRecognizedEvent) const
{
	bOutRecognizedEvent = false;
	TArray<UObject*> DropppedArray;

	
	// Asset being dragged from content browser
	if (DragDropOperation->IsOfType<FAssetDragDropOp>())
	{
		bOutRecognizedEvent = true;
		TSharedPtr<FAssetDragDropOp> DragDropOp = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOperation);

		#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
		const TArray<FAssetData>& DroppedAssets = DragDropOp->AssetData;
		#endif
		#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 17)
		const TArray<FAssetData>& DroppedAssets = DragDropOp->GetAssets();
		#endif
		#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION >= 18)
		const TArray<FAssetData>& DroppedAssets = DragDropOp->GetAssets();
		#endif
		if (DroppedAssets.Num() < 1)
		{
			return DropppedArray;
		}
		for (FAssetData AssetData : DroppedAssets)
		{
			DropppedArray.Add(AssetData.GetAsset());
		}
	}
	// Asset being dragged from some external source
	else if (DragDropOperation->IsOfType<FExternalDragOperation>())
	{
		TArray<FAssetData> DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(DragDropOperation);
		if (DroppedAssetData.Num() < 1)
		{
			return DropppedArray;
		}
		for (FAssetData DroppedAsset : DroppedAssetData)
		{
			bOutRecognizedEvent = true;
			DropppedArray.Add(DroppedAsset.GetAsset());
		}
	}
	// Actor being dragged?
	else if (DragDropOperation->IsOfType<FActorDragDropOp>())
	{
		bOutRecognizedEvent = true;
		TSharedPtr<FActorDragDropOp> ActorDragDrop = StaticCastSharedPtr<FActorDragDropOp>(DragDropOperation);
		if (ActorDragDrop->Actors.Num() < 1)
		{
			return DropppedArray;
		}
		for (TWeakObjectPtr<AActor> ActorDD : ActorDragDrop->Actors)
		{
			DropppedArray.Add(ActorDD.Get());
		}
	}

	return DropppedArray;
}

#undef LOCTEXT_NAMESPACE
