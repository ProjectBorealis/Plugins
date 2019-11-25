/* Copyright 2019 @TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerTab.h"
#include "MultiPackerChannelTab.h"
#include "MultiPackerTabStyle.h"
#include "MultiPackerCommands.h"
#include "LevelEditor.h"
#include <PropertyEditorModule.h>
#include <EditorFontGlyphs.h>
#include <Toolkits/AssetEditorToolkit.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include "MultiPackerSettings.h"

static const FName MultiPackerTabName("MultiPacker");

const FName ViewportTabId(TEXT("MultiPackerChannelTab"));
#define LOCTEXT_NAMESPACE "MultiPackerTab"

void FMultiPackerTab::StartupModule()
{
	// register menu extensions
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	PluginCommands = MakeShareable(new FUICommandList);

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FMultiPackerTabStyle::Initialize();
	FMultiPackerTabStyle::ReloadTextures();
	FMultiPackerCommands::Register();
	
	PluginCommands->MapAction(
		FMultiPackerCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FMultiPackerTab::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FMultiPackerTab::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Compile", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FMultiPackerTab::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FMultiPackerTab::ShutdownModule()
{
	// unregister menu extensions
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FMultiPackerTabStyle::Shutdown();
	FMultiPackerCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MultiPackerTabName);
}

void FMultiPackerTab::PluginButtonClicked()
{
	TSharedPtr<IToolkitHost> EditWithinLevelEditor;
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	TSharedRef<FMultiPackerChannelTab> NewGraphEditor(new FMultiPackerChannelTab());
	NewGraphEditor->InitGenericGraphAssetEditor(Mode, EditWithinLevelEditor);
}

void FMultiPackerTab::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FMultiPackerCommands::Get().OpenPluginWindow);
}

void FMultiPackerTab::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FMultiPackerCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMultiPackerTab, MultiPackerTab)