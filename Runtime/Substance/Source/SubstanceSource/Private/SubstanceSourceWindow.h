// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceWindow.h

/** Description:: This class will be the core window class and will contain all of the functionality for the window directly. */
#pragma once
#include "SubstanceSourceModule.h"
#include "SubstanceToolkit.h"
#include "EditorUndoClient.h"


#include "ISubstanceSource.h"
#include <substance/source/database.h>

class FSubstanceSourceModule;
class SSubstanceSourcePanel;

class FSubstanceSourceWindow : public FSubstanceToolkit, public FGCObject, public FEditorUndoClient
{
public:

	/** Destructor */
	virtual ~FSubstanceSourceWindow() override;

	/** Handles the creation and cleanup of Editor window tabs */
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	/** FGCObject interface - Informing the garbage collector of non UObject classes*/
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	/** Entry point for initializing this window */
	void InitSubstanceSourceWindow(const TSharedPtr<class IToolkitHost>& InitToolkitHost);

protected:

	/** Event handlers for various UI actions */
	FReply OnLoginPressed() override;
	FReply OnLogoutPressed() override;
	FReply OnSubstancePressed() override;

private:

	/** Creates all internal widgets for the tabs to point at */
	void CreateInternalWidgets();

	/** Builds the non default toolbar widgets for the window */
	void ExtendToolbar();

	/**	Binds our UI commands to delegates */
	void BindCommands();

	/**	Caches the specified tab for later retrieval */
	void AddToSpawnedToolPanels(const FName& TabIdentifier, const TSharedRef<SDockTab>& SpawnedTab);

	/**	Spawns the viewport tab */
	TSharedRef<SDockTab> SpawnTab_SubstanceSourceWindow(const FSpawnTabArgs& Args);

private:
	/** List of open tool panels; used to ensure only one exists at any one time */
	TMap<FName, TWeakPtr<SDockTab>> SpawnedToolPanels;

	/** Viewport */
	TSharedPtr<SSubstanceSourcePanel> SubstanceSourcePanel;

	/**	The tab label for the substance source that will be displayed on the tab */
	static const FName SubstanceSourceTabId;
};