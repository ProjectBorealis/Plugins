// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceEditorModule.cpp

#include "SubstanceEditorModule.h"
#include "SubstanceEditorPrivatePCH.h"
#include "SubstanceEditorModule.h"
#include "AssetTypeActions_Base.h"
#include "Modules/ModuleManager.h"
#include "Factories.h"
#include "Editor.h"
#include "EdGraphUtilities.h"
#include "LevelEditor.h"
#include "EditorStyleSet.h"
#include "Misc/MessageDialog.h"

#include "SubstanceCoreModule.h"
#include "SubstanceEditor.h"
#include "SubstanceEditorClasses.h"
#include "SubstanceCoreHelpers.h"

#include "AssetTypeActions_SubstanceGraphInstance.h"
#include "AssetTypeActions_SubstanceInstanceFactory.h"

#define LOCTEXT_NAMESPACE "SubstanceEditorModule"

namespace SubstanceEditorModule
{
const FName SubstanceEditorAppIdentifier = FName(TEXT("SubstanceEditorApp"));
static FEditorDelegates::FOnPIEEvent::FDelegate OnBeginPIEDelegate;
static FEditorDelegates::FOnPIEEvent::FDelegate OnEndPIEDelegate;
static FEditorDelegates::FOnMapOpened::FDelegate OnMapOpenedDelegate;
static FDelegateHandle OnBeginPIEDelegateHandle;
static FDelegateHandle OnEndPIEDelegateHandle;
static FDelegateHandle OnMapOpenedDelegateHandle;
}

/** Create a command list to be able to add a button to callback to rebuild */
class FSubstanceEditorModuleCommands : public TCommands<FSubstanceEditorModuleCommands>
{
public:
	/** Constructor */
	FSubstanceEditorModuleCommands()
		: TCommands<FSubstanceEditorModuleCommands>(
		      "SubstanceEditorModuleCommands",
		      NSLOCTEXT("Contexts", "SubstanceEditor", "SubstanceEditor"),
		      NAME_None,
		      FEditorStyle::GetStyleSetName()) // Icon Style Set
	{
	}

	TSharedPtr<FUICommandInfo> RebuildAllSubstances;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};

void FSubstanceEditorModuleCommands::RegisterCommands()
{
	UI_COMMAND(RebuildAllSubstances, "Rebuild All Substances", "Rebuild All Substances", EUserInterfaceActionType::Button, FInputGesture());
}

class FSubstanceEditorModule : public ISubstanceEditorModule
{
public:
	TSharedPtr<FAssetTypeActions_SubstanceGraphInstance> SubstanceGraphInstanceAssetTypeActions;
	TSharedPtr<FAssetTypeActions_SubstanceInstanceFactory> SubstanceInstanceFactoryAssetTypeActions;

	/** Constructor, set up console commands and variables **/
	FSubstanceEditorModule()
	{
	}

	/** Called right after the module DLL has been loaded and the module object has been created */
	virtual void StartupModule() override
	{
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

		FModuleManager::LoadModuleChecked<FSubstanceCoreModule>("SubstanceCore");
		FModuleManager::LoadModuleChecked<FSubstanceCoreModule>("AssetTools");

		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		SubstanceGraphInstanceAssetTypeActions = MakeShareable(new FAssetTypeActions_SubstanceGraphInstance);
		SubstanceInstanceFactoryAssetTypeActions = MakeShareable(new FAssetTypeActions_SubstanceInstanceFactory);

		AssetTools.RegisterAssetTypeActions(SubstanceGraphInstanceAssetTypeActions.ToSharedRef());
		AssetTools.RegisterAssetTypeActions(SubstanceInstanceFactoryAssetTypeActions.ToSharedRef());

		//Create the call back for when we start play in editor
		SubstanceEditorModule::OnBeginPIEDelegate = FEditorDelegates::FOnPIEEvent::FDelegate::CreateStatic(&FSubstanceEditorModule::OnPieStart);
		SubstanceEditorModule::OnBeginPIEDelegateHandle = FEditorDelegates::BeginPIE.Add(SubstanceEditorModule::OnBeginPIEDelegate);

		//Create the callback for when a play in editor session is ended
		SubstanceEditorModule::OnEndPIEDelegate = FEditorDelegates::FOnPIEEvent::FDelegate::CreateStatic(&FSubstanceEditorModule::OnPieEnd);
		SubstanceEditorModule::OnEndPIEDelegateHandle = FEditorDelegates::EndPIE.Add(SubstanceEditorModule::OnEndPIEDelegate);

		//Add a menu extension to rebuild all substances
		CreateRebuildSubstancesButtonExtention();

		SubstanceEditorModule::OnMapOpenedDelegate = FEditorDelegates::FOnMapOpened::FDelegate::CreateStatic(&FSubstanceEditorModule::OnMapOpened);
		SubstanceEditorModule::OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.Add(SubstanceEditorModule::OnMapOpenedDelegate);
	}

	/** Callback for when the rebuild substance menu button is pressed */
	static void OnRebuildAllSubstances()
	{
		RebuildAlert();
	}

	static void OnMapOpened(const FString& Filename, bool bAsTemplate)
	{
		RebuildAlert();
	}

	static void OnWorldInit(UWorld*, const UWorld::InitializationValues)
	{
	}

	static void RebuildAlert()
	{
		if (Substance::Helpers::SubstancesRequireUpdate())
		{
			EAppReturnType::Type RebuildMessage = FMessageDialog::Open(EAppMsgType::YesNoCancel,
				FText::FromString("Substance Graph data is missing or has been updated, would you like to rebuild the substance graphs now?\nPlease cancel and backup your projects before continuing."),
				nullptr);

			if (RebuildMessage == EAppReturnType::Yes)
				Substance::Helpers::RebuildAllSubstanceGraphInstances();
			else if (RebuildMessage == EAppReturnType::Cancel)
				FPlatformMisc::RequestExit(0);
		}
	}

	static void OnPieEnd(const bool bIsSimulating)
	{
		Substance::Helpers::EndPIE();
	}

	static void OnPieStart(const bool bIsSimulating)
	{
		Substance::Helpers::StartPIE();
	}

	/** Called before the module is unloaded, right before the module object is destroyed */
	virtual void ShutdownModule() override
	{
		FEditorDelegates::BeginPIE.Remove(SubstanceEditorModule::OnBeginPIEDelegateHandle);
		FEditorDelegates::EndPIE.Remove(SubstanceEditorModule::OnEndPIEDelegateHandle);
		FEditorDelegates::OnMapOpened.Remove(SubstanceEditorModule::OnMapOpenedDelegateHandle);

		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();

		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

			AssetTools.UnregisterAssetTypeActions(
			    SubstanceGraphInstanceAssetTypeActions.ToSharedRef());

			AssetTools.UnregisterAssetTypeActions(
			    SubstanceInstanceFactoryAssetTypeActions.ToSharedRef());
		}
	}

	virtual TSharedRef<ISubstanceEditor> CreateSubstanceEditor(const TSharedPtr< IToolkitHost >& InitToolkitHost, USubstanceGraphInstance* Graph) override
	{
		TSharedRef<FSubstanceEditor> NewSubstanceEditor(new FSubstanceEditor());
		NewSubstanceEditor->InitSubstanceEditor(InitToolkitHost, Graph);
		return NewSubstanceEditor;
	}

	virtual void PostLoadCallback() override
	{
		RebuildAlert();
	}

	/** Gets the extensibility managers for outside entities to extend static mesh editor's menus and tool bars */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager()
	{
		return MenuExtensibilityManager;
	}
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager()
	{
		return ToolBarExtensibilityManager;
	}

private:

	FReply RebuildAllSubstances()
	{

		return FReply::Handled();
	}

	/** Command list used for rebuild substances button*/
	TSharedPtr<FUICommandList> EditorCommandList;

	/** Creates and adds the button that will rebuild all substances */
	void CreateRebuildSubstancesButtonExtention()
	{
		FSubstanceEditorModuleCommands::Register();

		// Create access point to Substance utility (in LevelEditor Build Options menu)
		EditorCommandList = MakeShareable(new FUICommandList);

		//Get our command list
		const FSubstanceEditorModuleCommands& Commands = FSubstanceEditorModuleCommands::Get();

		//Map the callback for the RebuildAllSubstances UICommand
		EditorCommandList->MapAction(
		    Commands.RebuildAllSubstances,
		    FExecuteAction::CreateStatic(&FSubstanceEditorModule::OnRebuildAllSubstances),
		    FCanExecuteAction()
		);

		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		//Create a callback to create the extender when the menu is spawned
		FLevelEditorModule::FLevelEditorMenuExtender BuildMenuExtender =
		    FLevelEditorModule::FLevelEditorMenuExtender::CreateRaw(this, &FSubstanceEditorModule::OnExtendBuildMenu);

		//Add this callback to the list of the build menu extenders
		LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Add(BuildMenuExtender);
	}

	//Static function to add a substance section to the menu and to add our menu entry
	static void AddBuildSubstancesButton(FMenuBuilder& MenuBuilder)
	{
		if (Substance::Helpers::SubstancesRequireUpdate())
		{
			MenuBuilder.BeginSection("Substance", LOCTEXT("Substance", "Substance"));
			{
				MenuBuilder.AddMenuEntry(FSubstanceEditorModuleCommands::Get().RebuildAllSubstances);
			}
			MenuBuilder.EndSection();
		}
	}

	//Function callback for building a menu extender
	TSharedRef<FExtender> OnExtendBuildMenu(const TSharedRef<FUICommandList> CommandList)
	{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) && WITH_EDITOR
		TSharedRef<FExtender> Extender(new FExtender());
		Extender->AddMenuExtension("LevelEditorVerification",
		                           EExtensionHook::After,
		                           EditorCommandList,
		                           FMenuExtensionDelegate::CreateStatic(&AddBuildSubstancesButton));
#endif
		return Extender;
	}

	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};

IMPLEMENT_MODULE(FSubstanceEditorModule, SubstanceEditor);

#undef LOCTEXT_NAMESPACE //"SubstanceEditorModule"


