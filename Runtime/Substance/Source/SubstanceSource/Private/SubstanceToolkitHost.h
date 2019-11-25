// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceToolkitHost.h

/** Description:: This is our implementation of a toolkit host. This class will handle all of the window element that will be present within our tab */

#pragma once
#include "Toolkits/IToolkitHost.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SCompoundWidget.h"
#include "SubstanceToolkit.h"

class SSubstanceSourceToolkitHost :
	public IToolkitHost, public SCompoundWidget
{
public:

	/** Slate Arguments */
	SLATE_BEGIN_ARGS(SSubstanceSourceToolkitHost) = default;
	SLATE_EVENT(FRequestCloseSubstanceSource, OnRequestClose)
	SLATE_END_ARGS()

	/** Constructs the a new host for the substance toolkits */
	void Construct(const FArguments& InArgs, const TSharedPtr<FTabManager>& InTabManager, const FName InitAppName);

	/** Sets up the initial content and launches the window */
	void SetupInitialContent(const TSharedRef<FTabManager::FLayout>& DefaultLayout, const TSharedPtr<SDockTab>& InHostTab, const bool bCreateDefaultStandaloneMenu);

	/** IToolkitHost interface */
	virtual TSharedRef<class SWidget> GetParentWidget() override;
	virtual void BringToFront() override;
	virtual TSharedRef<class SDockTabStack> GetTabSpot(const EToolkitTabSpot::Type TabSpot) override;
	virtual void OnToolkitHostingStarted(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual UWorld* GetWorld() const override;
	virtual TSharedPtr<class FTabManager> GetTabManager() const override;

	/** Fills in the content by loading the associated layout or using the defaults provided. Must be called after the widget is constructed. */
	virtual void RestoreFromLayout(const TSharedRef<FTabManager::FLayout>& NewLayout);

	/** Generates the ui for all menus and tool bars, potentially forcing the menu to be created even if it shouldn't */
	void GenerateMenus(bool bForceCreateMenu);

	/** Gets all extenders that this toolkit host uses */
	TArray<TSharedPtr<FExtender>>& GetMenuExtenders()
	{
		return MenuExtenders;
	}

	/** Set a widget to use in the menu bar overlay */
	void SetMenuOverlay(TSharedRef<SWidget> NewOverlay);

private:

	/** Callback for when a tab is closed */
	void OnTabClosed(TSharedRef<SDockTab> TabClosed) const;

	/** Manages internal tab layout */
	TSharedPtr<FTabManager> MyTabManager;

	/** The widget that will house the default menu widget */
	TSharedPtr<SBorder> MenuWidgetContent;

	/** The widget that will house the overlay widgets (if any) */
	TSharedPtr<SBorder> MenuOverlayWidgetContent;

	/** The default menu widget */
	TSharedPtr< SWidget > DefaultMenuWidget;

	/** The DockTab in which we reside. */
	TWeakPtr<SDockTab> HostTabPtr;

	/** Name ID of the source module */
	FName SourceIDName;

	/** List of all of the toolkits we're currently hosting */
	TArray<TSharedPtr<class IToolkit>> HostedToolkits;

	/** The 'owning' substance toolkit we're hosting */
	TSharedPtr<class FSubstanceToolkit> HostedSubstanceToolkit;

	/** Delegate to be called to determine if we are allowed to close this toolkit host */
	FRequestCloseSubstanceSource EditorCloseRequest;

	/** The menu extenders to populate the main toolkit host menu with */
	TArray<TSharedPtr<FExtender>> MenuExtenders;
};
