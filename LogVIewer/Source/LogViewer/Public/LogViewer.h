// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "LogViewerOutputDevice.h"

class FToolBarBuilder;
class FMenuBuilder;
class SDockTab;
class FParsedLogLine;

class FLogViewerModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void PluginButtonClicked();

	//Cross tabs functionality - cosmetic only, main functionality is based on ini file state
	bool IsDefaultCategoriesEnabled() const { return bDefaultEnabled; }
	void DefaultCategoriesEnable() { bDefaultEnabled = true; }
	void DefaultCategoriesDisable() { bDefaultEnabled = false; }
	
private:
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> CreateLogViewerPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	void RegisterDefaultLogViewerTab(FTabManager& InTabManager);

	TSharedRef<SDockTab> SpawnDefaultLogViewerTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnEmptyLogViewerTab();

	TSharedRef<SDockTab> OnSpawnTabNewDocument(const FSpawnTabArgs& Args);

	void OnOpenNewTabClicked(TSharedRef<SDockTab> DockTab, ETabActivationCause InActivationCause);

	TSharedRef<SDockTab> SpawnSpecialTab();

	bool OnSpecialTabTryClose();
	//Best way I found to prevent application from undesired behavior where you can't create new tabs
	//Marking tab as main doesn't work, because UE can't be closed if at least one noncloseable tab exist (they made exception for MainTab of the engine, so epic)
	//Reopening doesn't work, since I can't reach docking area to insert tab to the 0 index. Insert tab can't find proper existing tab for me
	//So the only approach is to cache current tab, access Parent window through it and close the window. Irony is that if parent window is Engine window, the whole engine will be close.
	void OnSpecialTabClosed(TSharedRef<SDockTab> Tab);

	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FLogViewerOutputDevice> LogViewerOutputDevice;

	TSharedPtr<class FTabManager> TabManager;
	TSharedPtr<FTabManager::FLayout> TabManagerLayout;

	static FName TabNameOpenNewTab;
	static FName TabNameDefaultEngineOutput;

	TWeakPtr<SDockTab> PluginTab;
	TWeakPtr<SDockTab> SpecialTab;

	bool bDefaultEnabled = false;
};
