// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceToolkitHost.cpp

#include "SubstanceToolkitHost.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceToolkit.h"
#include "SubstanceSourceModule.h"

#include "EditorStyleSet.h"
#include "Toolkits/ToolkitManager.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Interfaces/IMainFrameModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "SubstanceSourceModule"

void SSubstanceSourceToolkitHost::Construct(const FArguments& InArgs, const TSharedPtr<FTabManager>& InTabManager, const FName InitAppName)
{
	EditorCloseRequest = InArgs._OnRequestClose;

	SourceIDName = InitAppName;
	MyTabManager = InTabManager;
}

void SSubstanceSourceToolkitHost::SetupInitialContent(const TSharedRef<FTabManager::FLayout>& DefaultLayout, const TSharedPtr<SDockTab>& InHostTab, const bool bCreateDefaultStandaloneMenu)
{
	//This struct is used to set up the delegate callbacks which will need a reference to a class to call the function callback from.
	struct Local
	{
		static void FillFileMenu(FMenuBuilder& MenuBuilder, TWeakPtr<FSubstanceToolkit> AssetEditorToolkitWeak)
		{
			auto AssetEditorToolkit(AssetEditorToolkitWeak.Pin().ToSharedRef());

			AssetEditorToolkit->FillDefaultFileMenuCommands(MenuBuilder);
		}

		static void FillAssetMenu(FMenuBuilder& MenuBuilder, TWeakPtr< FSubstanceToolkit > AssetEditorToolkitWeak)
		{
			auto AssetEditorToolkit(AssetEditorToolkitWeak.Pin().ToSharedRef());

			MenuBuilder.BeginSection("AssetEditorActions", LOCTEXT("ActionsHeading", "Actions"));
			{
				AssetEditorToolkit->FillDefaultAssetMenuCommands(MenuBuilder);
			}
			MenuBuilder.EndSection();
		}

		static void ExtendHelpMenu(FMenuBuilder& MenuBuilder, TWeakPtr< FSubstanceToolkit > AssetEditorToolkitWeak)
		{
			auto AssetEditorToolkit(AssetEditorToolkitWeak.Pin().ToSharedRef());

			MenuBuilder.BeginSection("HelpBrowse", NSLOCTEXT("MainHelpMenu", "Browse", "Browse"));
			{
				AssetEditorToolkit->FillDefaultHelpMenuCommands(MenuBuilder);
			}
			MenuBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	auto AssetEditorToolkit = HostedSubstanceToolkit.ToSharedRef();

	// Add asset-specific menu items to the top of the "File" menu
	MenuExtender->AddMenuExtension("FileLoadAndSave", EExtensionHook::First, AssetEditorToolkit->GetToolkitCommands(),
	                               FMenuExtensionDelegate::CreateStatic(&Local::FillFileMenu, TWeakPtr<FSubstanceToolkit>(AssetEditorToolkit)));

	MenuExtender->AddMenuExtension("HelpOnline", EExtensionHook::Before, AssetEditorToolkit->GetToolkitCommands(),
	                               FMenuExtensionDelegate::CreateStatic(&Local::ExtendHelpMenu, TWeakPtr<FSubstanceToolkit>(AssetEditorToolkit)));

	MenuExtenders.Add(MenuExtender);

	DefaultMenuWidget = SNullWidget::NullWidget;

	HostTabPtr = InHostTab;

	RestoreFromLayout(DefaultLayout);
	GenerateMenus(bCreateDefaultStandaloneMenu);
}

TSharedRef<SWidget> SSubstanceSourceToolkitHost::GetParentWidget()
{
	return AsShared();
}

void SSubstanceSourceToolkitHost::BringToFront()
{
	FGlobalTabmanager::Get()->DrawAttentionToTabManager(this->MyTabManager.ToSharedRef());
}

TSharedRef< SDockTabStack > SSubstanceSourceToolkitHost::GetTabSpot(const EToolkitTabSpot::Type TabSpot)
{
	return TSharedPtr<SDockTabStack>().ToSharedRef();
}

void SSubstanceSourceToolkitHost::OnToolkitHostingStarted(const TSharedRef< class IToolkit >& Toolkit)
{
	// Keep track of the toolkit we're hosting
	HostedToolkits.Add(Toolkit);

	// The tab manager needs to know how to spawn tabs from this toolkit
	Toolkit->RegisterTabSpawners(MyTabManager.ToSharedRef());

	if (!HostedSubstanceToolkit.IsValid())
	{
		HostedSubstanceToolkit = StaticCastSharedRef<FSubstanceToolkit>(Toolkit);
	}
	else
	{
		HostedSubstanceToolkit->OnToolkitHostingStarted(Toolkit);
	}
}

void SSubstanceSourceToolkitHost::OnToolkitHostingFinished(const TSharedRef< class IToolkit >& Toolkit)
{
	// The tab manager should forget how to spawn tabs from this toolkit
	Toolkit->UnregisterTabSpawners(MyTabManager.ToSharedRef());

	HostedToolkits.Remove(Toolkit);

	// Standalone Asset Editors close by shutting down their major tab.
	if (Toolkit == HostedSubstanceToolkit)
	{
		HostedSubstanceToolkit.Reset();

		const TSharedPtr<SDockTab> HostTab = HostTabPtr.Pin();
		if (HostTab.IsValid())
		{
			HostTab->RequestCloseTab();
		}
	}
	else if (HostedSubstanceToolkit.IsValid())
	{
		HostedSubstanceToolkit->OnToolkitHostingFinished(Toolkit);
	}
}

UWorld* SSubstanceSourceToolkitHost::GetWorld() const
{
	UE_LOG(LogInit, Warning, TEXT("IToolkitHost::GetWorld() doesn't make sense in SStandaloneAssetEditorToolkitHost currently"));
	return nullptr;
}

TSharedPtr< class FTabManager > SSubstanceSourceToolkitHost::GetTabManager() const
{
	return nullptr;
}

void SSubstanceSourceToolkitHost::RestoreFromLayout(const TSharedRef<FTabManager::FLayout>& NewLayout)
{
	const TSharedRef<SDockTab> HostTab = HostTabPtr.Pin().ToSharedRef();
	HostTab->SetCanCloseTab(EditorCloseRequest);
	HostTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &SSubstanceSourceToolkitHost::OnTabClosed));

	this->ChildSlot[SNullWidget::NullWidget];
	MyTabManager->CloseAllAreas();

	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(HostTab);
	TSharedPtr<SWidget> RestoredUI = MyTabManager->RestoreFrom(NewLayout, ParentWindow);

	checkf(RestoredUI.IsValid(), TEXT("The layout must have a primary dock area"));

	MenuOverlayWidgetContent.Reset();
	MenuWidgetContent.Reset();
	this->ChildSlot
	[
	    SNew(SVerticalBox)
	    // Menu bar area
	    + SVerticalBox::Slot()
	    .AutoHeight()
	    [
	        SNew(SOverlay)
	        // The menu bar
	        + SOverlay::Slot()
	        [
	            SAssignNew(MenuWidgetContent, SBorder)
	            .Padding(0)
	            .BorderImage(FEditorStyle::GetBrush("NoBorder"))
	            [
	                DefaultMenuWidget.ToSharedRef()
	            ]
	        ]
	        // The menu bar overlay
	        + SOverlay::Slot()
	        .HAlign(HAlign_Right)
	        [
	            SNew(SHorizontalBox)
	            + SHorizontalBox::Slot()
	            [
	                SAssignNew(MenuOverlayWidgetContent, SBorder)
	                .Padding(0)
	                .BorderImage(FEditorStyle::GetBrush("NoBorder"))
	            ]
	        ]
	    ]
	    // Viewport/Document/Docking area
	    + SVerticalBox::Slot()
	    .Padding(1.0f)

	    // Fills all leftover space
	    .FillHeight(1.0f)
	    [
	        RestoredUI.ToSharedRef()
	    ]
	];
}

void SSubstanceSourceToolkitHost::GenerateMenus(bool bForceCreateMenu)
{
}

void SSubstanceSourceToolkitHost::SetMenuOverlay(TSharedRef<SWidget> NewOverlay)
{
	MenuOverlayWidgetContent->SetContent(NewOverlay);
}

void SSubstanceSourceToolkitHost::OnTabClosed(TSharedRef<SDockTab> TabClosed) const
{
	check(TabClosed == HostTabPtr.Pin());

	ISubstanceSourceModule::Get()->OnSubstanceSourceWindowClosed();

	MyTabManager->SetMenuMultiBox(nullptr);

	//TODO:: So we will indeed need this. This is used to serailize the window when the asset is closed!
	//If we want to restore the windows when the editor is opened and the substance window was previously opened, this will be needed.

	//We aren't editing an object here so is this needed? In the case it is,
	//the goal is to find another way to do this without the object
	//	GConfig->SetInt(
	//		TEXT("AssetEditorToolkitTabLocation"),
	//		*ObjectBeingEdited->GetPathName(),
	//		static_cast<int32>(AssetEditorToolkitTabLocation),
	//		GEditorPerProjectIni
	//	);
}

#undef LOCTEXT_NAMESPACE
