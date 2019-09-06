// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "AssetAssistantEdMode.h"
#include "Modules/ModuleManager.h"
#include "Engine/DataTable.h"

class FToolBarBuilder;
class FMenuBuilder;
class FAssetAssistant : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	FAssetAssistantEdMode * EDMode;
	TArray< TSharedRef< class ITextDecorator > > CreatedDecorators;

};