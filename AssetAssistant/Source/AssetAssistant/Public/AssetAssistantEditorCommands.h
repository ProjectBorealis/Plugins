// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "AssetAssistantStyle.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/Commands.h"

enum class EToolMode
{
	Find = 1,
	Modify = 2,	
	Macro = 4,
	Extra = 8,
	About = 16,
	Result = 32,
	Options = 64
};

class FAssetAssistantEditorCommands : public TCommands<FAssetAssistantEditorCommands>
{
public:
	FAssetAssistantEditorCommands()
		: TCommands<FAssetAssistantEditorCommands>(TEXT("AssetAssistant"), NSLOCTEXT("Contexts", "AssetAssistant", "Instance Tool Plugin"), NAME_None, FAssetAssistantStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	// Mode
	TSharedPtr<FUICommandInfo> FindMode;
	TSharedPtr<FUICommandInfo> ModifyMode;
	TSharedPtr<FUICommandInfo> MacroMode;
	TSharedPtr<FUICommandInfo> ExtraMode;
	TSharedPtr<FUICommandInfo> AboutMode;

	// Map
	TMap < EToolMode, TSharedPtr<FUICommandInfo>> CommandMap;
};
