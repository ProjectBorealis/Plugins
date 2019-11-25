/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"

class FMultiPackerRuntimeAssetEditor;
class FExtender;
class FToolBarBuilder;

class FMultiPackerRuntimeAssetEditorToolbar : public TSharedFromThis<FMultiPackerRuntimeAssetEditorToolbar>
{
public:
	FMultiPackerRuntimeAssetEditorToolbar(TSharedPtr<FMultiPackerRuntimeAssetEditor> InGenericGraphEditor)
		: GenericGraphEditor(InGenericGraphEditor) {}

	//void AddModesToolbar(TSharedPtr<FExtender> Extender);
	//void AddDebuggerToolbar(TSharedPtr<FExtender> Extender);
	void AddGenericGraphToolbar(TSharedPtr<FExtender> Extender);
	FSlateIcon GraphSettingsIconOutput;
private:
	//void FillModesToolbar(FToolBarBuilder& ToolbarBuilder);
	//void FillDebuggerToolbar(FToolBarBuilder& ToolbarBuilder);
	void FillGenericGraphToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FMultiPackerRuntimeAssetEditor> GenericGraphEditor;
};
