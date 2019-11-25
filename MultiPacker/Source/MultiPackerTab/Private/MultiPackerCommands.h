/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once
#include "Framework/Commands/Commands.h"
#include "MultiPackerTabStyle.h"

#define LOCTEXT_NAMESPACE "FMultiPackerCommands"

class FMultiPackerCommands : public TCommands<FMultiPackerCommands>
{
public:

	FMultiPackerCommands()
		: TCommands<FMultiPackerCommands>(TEXT("MultiPacker"), NSLOCTEXT("Compile", "MultiPacker", "MultiPacker Plugin"), NAME_None, FMultiPackerTabStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};