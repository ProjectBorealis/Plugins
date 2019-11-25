/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetEditor/MultiPackerAssetEditorToolbar.h"
#include "MultiPackerAssetEditor/MultiPackerAssetEditor.h"
#include "MultiPackerAssetEditor/MultiPackerEditorCommands.h"
#include <UObject/UObjectGlobals.h>

#define LOCTEXT_NAMESPACE "GenericGraphAssetEditorToolbar"

void FMultiPackerAssetEditorToolbar::AddGenericGraphToolbar(TSharedPtr<FExtender> Extender)
{
	check(GenericGraphEditor.IsValid());
	TSharedPtr<FMultiPackerAssetEditor> GenericGraphEditorPtr = GenericGraphEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, GenericGraphEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP( this, &FMultiPackerAssetEditorToolbar::FillMultiPackerToolbar ));
	GenericGraphEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FMultiPackerAssetEditorToolbar::FillMultiPackerToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(GenericGraphEditor.IsValid());
	ToolbarBuilder.BeginSection("MultiPacker");
	{
		const FText GraphSettingsLabelOutput = LOCTEXT("GraphSettings_LabelOut", "Apply");
		const FText GraphSettingsTipOutput = LOCTEXT("GraphSettings_ToolTipOut", "Apply the Settings");
		GraphSettingsIconOutput = FSlateIcon(FEditorStyle::GetStyleSetName(), "MaterialEditor.Apply"); //"Kismet.Status.Good");
		ToolbarBuilder.AddToolBarButton(FMultiPackerEditorCommands::Get().CompileGraphNodes,
			NAME_None,
			GraphSettingsLabelOutput,
			GraphSettingsTipOutput,
			GraphSettingsIconOutput);

		const FText GraphSettingsLabel = LOCTEXT("GraphSettings_Label", "Graph Settings");
		const FText GraphSettingsTip = LOCTEXT("GraphSettings_ToolTip", "Show the Graph Settings");
		const FSlateIcon GraphSettingsIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings");
		ToolbarBuilder.AddToolBarButton(FMultiPackerEditorCommands::Get().GraphSettings,
			NAME_None,
			GraphSettingsLabel,
			GraphSettingsTip,
			GraphSettingsIcon);
	}
	ToolbarBuilder.EndSection();	
}


#undef LOCTEXT_NAMESPACE
