/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"

class FMultiPackerAssetEditor;
class FExtender;
class FToolBarBuilder;

class FMultiPackerAssetEditorToolbar : public TSharedFromThis<FMultiPackerAssetEditorToolbar>
{
public:
	FMultiPackerAssetEditorToolbar(TSharedPtr<FMultiPackerAssetEditor> InGenericGraphEditor)
		: GenericGraphEditor(InGenericGraphEditor) {}

	//void AddModesToolbar(TSharedPtr<FExtender> Extender);
	//void AddDebuggerToolbar(TSharedPtr<FExtender> Extender);
	void AddGenericGraphToolbar(TSharedPtr<FExtender> Extender);
	FSlateIcon GraphSettingsIconOutput;
private:
	//void FillModesToolbar(FToolBarBuilder& ToolbarBuilder);
	//void FillDebuggerToolbar(FToolBarBuilder& ToolbarBuilder);
	void FillMultiPackerToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FMultiPackerAssetEditor> GenericGraphEditor;
};
