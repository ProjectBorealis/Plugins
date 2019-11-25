/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/MultiPackerRuntimeAssetEditorToolbar.h"
#include "RuntimeGraph/MultiPackerRuntimeAssetEditor.h"
#include "MultiPackerAssetEditor/MultiPackerEditorCommands.h"


#define LOCTEXT_NAMESPACE "GenericGraphRuntimeAssetEditorToolbar"

void FMultiPackerRuntimeAssetEditorToolbar::AddGenericGraphToolbar(TSharedPtr<FExtender> Extender)
{
	check(GenericGraphEditor.IsValid());
	TSharedPtr<FMultiPackerRuntimeAssetEditor> GenericGraphEditorPtr = GenericGraphEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, GenericGraphEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP( this, &FMultiPackerRuntimeAssetEditorToolbar::FillGenericGraphToolbar ));
	GenericGraphEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FMultiPackerRuntimeAssetEditorToolbar::FillGenericGraphToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(GenericGraphEditor.IsValid());
	TSharedPtr<FMultiPackerRuntimeAssetEditor> GenericGraphEditorPtr = GenericGraphEditor.Pin();

	ToolbarBuilder.BeginSection("MultiPacker");
	{

		//Set("Kismet.Status.Unknown", new IMAGE_BRUSH("Old/Kismet2/CompileStatus_Working", Icon40x40)); SlateEditorStyle.cpp
		//Set("Kismet.Status.Error", new IMAGE_BRUSH("Old/Kismet2/CompileStatus_Fail", Icon40x40));
		//Set("Kismet.Status.Good", new IMAGE_BRUSH("Old/Kismet2/CompileStatus_Good", Icon40x40));
		const FText GraphSettingsLabelOutput = LOCTEXT("GraphSettings_LabelOut", "Apply");
		const FText GraphSettingsTipOutput = LOCTEXT("GraphSettings_ToolTipOut", "Apply");
		GraphSettingsIconOutput = FSlateIcon(FEditorStyle::GetStyleSetName(), "MaterialEditor.Apply"); //Kismet.Status.Good");
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
