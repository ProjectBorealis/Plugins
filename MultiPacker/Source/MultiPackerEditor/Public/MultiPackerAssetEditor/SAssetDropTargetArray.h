/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SDropTarget.h"

/**
* A widget that displays a hover cue and handles dropping assets of allowed types onto this widget
*/
class SAssetDropTargetArray : public SDropTarget
{
public:
	/** Called when a valid asset is dropped */
	DECLARE_DELEGATE_OneParam(FOnAssetDropped, UObject*);

	/** Called when we need to check if an asset type is valid for dropping */
	DECLARE_DELEGATE_RetVal_OneParam(bool, FIsAssetAcceptableForDrop, const UObject*);

	SLATE_BEGIN_ARGS(SAssetDropTargetArray)
	{ }
	/* Content to display for the in the drop target */
	SLATE_DEFAULT_SLOT(FArguments, Content)
		/** Called when a valid asset is dropped */
		SLATE_EVENT(FOnAssetDropped, OnAssetDropped)
		/** Called to check if an asset is acceptible for dropping */
		SLATE_EVENT(FIsAssetAcceptableForDrop, OnIsAssetAcceptableForDrop)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);
	
	uint8 GetRow();
	uint8 GetColumn();

protected:
	FReply OnDropped(TSharedPtr<FDragDropOperation> DragDropOperation);
	virtual bool OnAllowDrop(TSharedPtr<FDragDropOperation> DragDropOperation) const override;
	virtual bool OnIsRecognized(TSharedPtr<FDragDropOperation> DragDropOperation) const override;
private:
	TArray<UObject*> GetDroppedObject(TSharedPtr<FDragDropOperation> DragDropOperation, bool& bOutRecognizedEvent) const;
	
private:
	uint8 ArrayObjectNum;
	uint8 ArrayObjectDropTotal;
	uint8 ColumnAssetDrop = 0;
	uint8 RowAssetDrop = 0;
	/** Delegate to call when an asset is dropped */
	FOnAssetDropped OnAssetDropped;
	/** Delegate to call to check validity of the asset */
	FIsAssetAcceptableForDrop OnIsAssetAcceptableForDrop;
};
