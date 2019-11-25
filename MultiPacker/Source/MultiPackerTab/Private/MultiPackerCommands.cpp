/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "MultiPackerCommands.h"

#define LOCTEXT_NAMESPACE "FMultiPackerCommands"

void FMultiPackerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "MultiPacker", "Bring up MultiPacker Channel Pack Tab", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
