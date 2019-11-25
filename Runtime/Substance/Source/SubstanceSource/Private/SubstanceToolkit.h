// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceToolkit.h

/** Description:: This is the base class for the source window interface
	Source Widgets handled by this class:
	-- LoginButton
	-- LogoutButton
	-- AvailableDownloads
	-- FileMenu
*/

// TODO:: Don't add the loading button until the database has been loaded. This shouldn't
//		  need to be handled for the logout button because the logout button shouldn't appear until
//        the login button is pressed. With loading being handled only at the start of the panel, this
//        the logout button should never appear before loading completes by default

#pragma once
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/BaseToolkit.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Docking/LayoutService.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"


class SSubstanceSourceToolkitHost;
class FExtender;
class FUICommandList;
class IToolkit;
class IToolkitHost;
class SBorder;
class SStandaloneAssetEditorToolkitHost;
class SWidget;
struct FSlateBrush;
struct FTabId;

DECLARE_DELEGATE_RetVal(bool, FRequestCloseSubstanceSource);

/** The location of the source toolkit tab - Serializes into an ini as int32 */
enum class ESubstanceToolkitTabLocation : int32
{
	Docked,
	Standalone,
};

class FSubstanceToolkit
	: public FBaseToolkit
	, public TSharedFromThis<FSubstanceToolkit>
{
public:

	/** Default constructor */
	FSubstanceToolkit() = default;

	/** Creates an initializes our editor */
	virtual void InitAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const FName AppIdentifier, const TSharedRef<FTabManager::FLayout>& StandaloneDefaultLayout,
	                             const bool bCreateDefaultStandaloneMenu, const bool bCreateDefaultToolbar, const bool bInIsToolbarFocusable = false);

	/** Used to create the tabs for this window. In this case, the tool bar menu tab */
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;

	/** Pure virtual overrides that need to be overridden in child classes */
	virtual FString GetWorldCentricTabPrefix() const override = 0;
	virtual FName GetToolkitFName() const override = 0;
	virtual FText GetBaseToolkitName() const override = 0;

	/** Parent virtual functions overridden here */
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual class FEdMode* GetEditorMode() const override;

	/** Implementation of core editor features */
	virtual FName GetEditorName() const;
	virtual void FocusWindow(UObject* ObjectToFocusOn = nullptr);
	virtual bool CloseWindow();
	virtual bool IsPrimaryEditor() const
	{
		return true;
	};
	virtual void InvokeTab(const FTabId& TabId);
	virtual TSharedPtr<FTabManager> GetAssociatedTabManager();

	/** Setup functions for the default core window elements */
	void FillDefaultFileMenuCommands(FMenuBuilder& MenuBuilder);
	void FillDefaultAssetMenuCommands(FMenuBuilder& MenuBuilder);
	void FillDefaultHelpMenuCommands(FMenuBuilder& MenuBuilder);

	//NOTE:: If we don't plan on allowing our window to doc within the level editor host or any other host, this shouldn't be needed. a
	/** Functions used to store what host was last hosting this window */
	TSharedPtr<IToolkitHost> GetPreviousWorldCentricToolkitHost();
	static void SetPreviousWorldCentricToolkitHostForNewAssetEditor(TSharedRef<IToolkitHost> ToolkitHost);

	/** Accessors for the toolkits tab manager */
	TSharedPtr<FTabManager> GetTabManager()
	{
		return TabManager;
	}

	/** IToolKitInterface - Returns if we are editing an asset */
	virtual bool IsAssetEditor() const override;

	/** IToolKitInterface - We will never be editing assets so this will always return null */
	virtual const TArray< UObject* >* GetObjectsCurrentlyBeingEdited() const override;

	/** Makes a default asset substance source toolbar */
	void GenerateToolbar();

	// Enables 'Login' button (when database is loaded)
	virtual void EnableLoginButton();

	/** Generates the substance source specific widgets*/
	void ConstructBaseWidgets();

	/** Regenerates the menu bar and toolbar widgets */
	void RegenerateMenusAndToolbars();

	/** Called at the end of RegenerateMenusAndToolbars() */
	virtual void PostRegenerateMenusAndToolbars() { }

	/** Regenerate the tool bar to add different widgets based on if the user has logged in or is logged out */
	void RegenerateToolBar();

	/** Called when another toolkit (such as a ed mode toolkit) is being hosted in this asset editor toolkit */
	virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) {}

	/** Called when another toolkit (such as a ed mode toolkit) is no longer being hosted in this asset editor toolkit */
	virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) {}

	/** Toolbar tab ID accessor */
	FName GetToolbarTabId() const
	{
		return ToolbarTabId;
	}

	void SetMenuOverlay(TSharedRef<SWidget> Widget);

protected:

	/** Called when this toolkit would close */
	virtual bool OnRequestClose()
	{
		return true;
	}

	/** @return a pointer to the brush to use for the tab icon */
	const FSlateBrush* GetDefaultTabIcon() const;

	/** Controls our internal layout */
	TSharedPtr<FTabManager> TabManager;

	/** Events! */
	virtual FReply OnLoginPressed();
	virtual FReply OnLogoutPressed();
	virtual FReply OnSubstancePressed();
	virtual FReply OnMyAssetsPressed();

private:

	/** Framework Callbacks */
	void OnUserHasLoggedIn();
	void OnUserHasLoggedOut();
	void OnSourceLoadComplete();
	void OnSourceLoadError();
	void OnCategorySwitched();
	void OnAssetPurchased(Alg::Source::AssetPtr PurchasedAsset);
	void OnSearchTextChanged(const FText& NewSearchValue);

	/** Spawns the toolbar tab */
	TSharedRef<SDockTab> SpawnTab_Toolbar(const FSpawnTabArgs& Args);

	/** Callback for persisting the Asset Editor's layout. */
	void HandleTabManagerPersistLayout(const TSharedRef<FTabManager::FLayout>& LayoutToSave)
	{
		FLayoutSaveRestore::SaveToConfig(GEditorLayoutIni, LayoutToSave);
	}

	/** Host for this toolkit */
	TWeakPtr<SSubstanceSourceToolkitHost> SubstanceHost;

	/** The widget that will house the login button, asset count, etc. */
	TSharedPtr<SVerticalBox> ToolbarWidgetContent;

	/** The toolbar root that will contain all widgets associated with the tool bar */
	TSharedPtr<SHorizontalBox> Toolbar;

	/** Login Button */
	TSharedPtr<SButton> LoginButton;

	/** Logout Button */
	TSharedPtr<SButton> LogoutButton;

	/** Logout Button */
	TSharedPtr<SButton> MyAssetsButton;

	/** Home Button */
	TSharedPtr<SButton> SubstanceButton;

	/** Input bar used for searching */
	TSharedPtr<SEditableTextBox> SearchBar;

	/** Whether the buttons on the default toolbar can receive keyboard focus */
	bool bIsToolbarFocusable;

	/**	The tab ids for all the tabs used */
	static const FName ToolbarTabId;
};
