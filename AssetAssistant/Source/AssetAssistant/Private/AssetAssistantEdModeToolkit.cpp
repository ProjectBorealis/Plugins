// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEdModeToolkit.h"

#define LOCTEXT_NAMESPACE "FAssetAssistantEdModeToolkit"

void FAssetAssistantEdModeToolkit::Init(const TSharedPtr<class IToolkitHost>& InitToolkitHost)
{
	AssetAssistantEditorWidget = SNew(SAssetAssistantEditor, SharedThis(this));
	auto NameToCommandMap = FAssetAssistantEditorCommands::Get().CommandMap;
	TSharedRef<FUICommandList> CommandList = GetToolkitCommands();

#define MAP_MODE(ModeName) CommandList->MapAction(NameToCommandMap.FindChecked(ModeName), FUIAction(FExecuteAction::CreateSP(this, &FAssetAssistantEdModeToolkit::OnChangeMode, ModeName), FCanExecuteAction::CreateSP(this, &FAssetAssistantEdModeToolkit::IsModeEnabled, ModeName), FIsActionChecked::CreateSP(this, &FAssetAssistantEdModeToolkit::IsModeActive, ModeName)));
	MAP_MODE(EToolMode::Find);
	MAP_MODE(EToolMode::Modify);
	MAP_MODE(EToolMode::Macro);
	MAP_MODE(EToolMode::Extra);
#undef MAP_MODE

	FModeToolkit::Init(InitToolkitHost);
}

FName FAssetAssistantEdModeToolkit::GetToolkitFName() const
{
	return FName("AssetAssistantEditor");
}

FText FAssetAssistantEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("AssetAssistantEditor", "DisplayName", "Instance Tool");
}

class FAssetAssistantEdMode* FAssetAssistantEdModeToolkit::GetEditorMode() const
{
	return (FAssetAssistantEdMode*)GLevelEditorModeTools().GetActiveMode(FAssetAssistantEdMode::EM_AssetAssistantEdModeId);
}

void FAssetAssistantEdModeToolkit::OnChangeMode(EToolMode InMode)
{
	GetEditorMode()->SetCurrentToolMode(InMode);
}

bool FAssetAssistantEdModeToolkit::IsModeEnabled(EToolMode InMode) const
{
	return true;
}

bool FAssetAssistantEdModeToolkit::IsModeActive(EToolMode InMode) const
{
	return GetEditorMode()->GetCurrentToolMode() == InMode;
}

TSharedRef<FAssetAssistantEdModeToolkit> FAssetAssistantEdModeToolkit::getShared()
{
	return SharedThis(this);
}

#undef LOCTEXT_NAMESPACE
