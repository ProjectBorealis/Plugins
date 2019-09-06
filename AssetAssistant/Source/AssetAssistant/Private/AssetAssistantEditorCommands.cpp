// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "AssetAssistantEditorCommands.h"

#define LOCTEXT_NAMESPACE "AssetAssistantEditorCommands"

void FAssetAssistantEditorCommands::RegisterCommands()
{
	// Plugin commands
	UI_COMMAND(FindMode, "Mode - Find", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Find, FindMode);
	UI_COMMAND(ModifyMode, "Mode - Modify", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Modify, ModifyMode);
	UI_COMMAND(MacroMode, "Mode - Macro", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Macro, MacroMode);
	UI_COMMAND(ExtraMode, "Mode - About", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Extra, ExtraMode);
}

#undef LOCTEXT_NAMESPACE