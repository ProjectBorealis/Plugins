// --------------------------------------------------------------------------
// Implementation of SceneProtocol plugin editor module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#include "SceneProtocolEditorModule.h"
#include "SceneProtocolEditor.h"

#include "Editor.h"
#include "LevelEditor.h"
#include "Editor/EditorEngine.h"
#include "Engine/Selection.h" 

#include "SceneProtocolEditorStyle.h"
#include "SceneProtocolEditorCommands.h"

#include "SceneProtocolModule.h"
#include "SceneProtocolStaticMeshActor.h"
#include "AssetRegistryModule.h"
#include "EngineUtils.h"
#include "ActorFactories/ActorFactoryStaticMesh.h"

#define LOCTEXT_NAMESPACE "SceneProtocolEditor"


namespace SPNBU = SceneProtocol::NetworkBridge::Unreal;

void SPNBU::FSceneProtocolEditorModule::StartupModule()
{
  FSceneProtocolEditorStyle::Initialize();
  FSceneProtocolEditorStyle::ReloadTextures();

  FSceneProtocolEditorCommands::Register();

  PluginCommands = MakeShareable(new FUICommandList);

  // Client fetch actions
  PluginCommands->MapAction(
    FSceneProtocolEditorCommands::Get().FetchSceneAction,
    FExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::DoFetchScene),
	FCanExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::CanDoFetchScene));

  PluginCommands->MapAction(
    FSceneProtocolEditorCommands::Get().CancelFetchAction,
    FExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::DoCancelFetch),
    FCanExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::CanDoCancelFetch));

  // Client thread start and stop actions
  PluginCommands->MapAction(
    FSceneProtocolEditorCommands::Get().ClientStartAction,
    FExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::DoStartClient),
    FCanExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::CanDoStartClient));

  PluginCommands->MapAction(
    FSceneProtocolEditorCommands::Get().ClientStopAction,
    FExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::DoStopClient),
    FCanExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::CanDoStopClient));

  // Client push actions
  PluginCommands->MapAction(
    FSceneProtocolEditorCommands::Get().PushAllItems,
    FExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::DoPushAll),
    FCanExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::CanDoPushAll));

  PluginCommands->MapAction(
    FSceneProtocolEditorCommands::Get().PushSelectedItems,
    FExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::DoPushSelected),
    FCanExecuteAction::CreateRaw(this, &FSceneProtocolEditorModule::CanDoPushSelected));

  FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
  {
    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
    MenuExtender->AddMenuExtension("FileActors", EExtensionHook::After,
      PluginCommands,
      FMenuExtensionDelegate::CreateRaw(this,
        &FSceneProtocolEditorModule::AddMenuExtension));

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
  }

  {
    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension("Content", EExtensionHook::After,
      PluginCommands,
      FToolBarExtensionDelegate::CreateRaw(this,
        &FSceneProtocolEditorModule::AddToolbarExtension));

    LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
  }

  // add a command to swap out the static mesh actors
  if (!IsRunningCommandlet())
  {
	  cmdSwapOutCustomMeshClasses = IConsoleManager::Get().RegisterConsoleCommand(
		  TEXT("modobridge.swapOutCustomMeshes"),
		  TEXT("Converts any SceneProtocolStaticMeshActors into standard StaticMeshActors"),
		  FConsoleCommandDelegate::CreateStatic(SwapOutCustomMeshClasses),
		  ECVF_Default
	  );
  }

}

void SPNBU::FSceneProtocolEditorModule::ShutdownModule()
{
  FSceneProtocolEditorStyle::Shutdown();
  FSceneProtocolEditorCommands::Unregister();
}


void SPNBU::FSceneProtocolEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
  Builder.AddSubMenu(
    LOCTEXT("SceneProtocolMenuTitle", "Modo Bridge"),
    LOCTEXT("SceneProtocolMenuTip", "Foundry: Modo - Unreal network bridge"),
    FNewMenuDelegate::CreateRaw(this, &FSceneProtocolEditorModule::AddSceneProtocolMenuItems));
}


void SPNBU::FSceneProtocolEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
  Builder.AddComboButton(FUIAction(),
    FOnGetContent::CreateRaw(this, &FSceneProtocolEditorModule::CreateSceneProtocolMenu),
    LOCTEXT("SceneProtocolMenuTitle", "Modo Bridge"),
    LOCTEXT("SceneProtocolMenuTip", "Foundry: Modo - Unreal network bridge"),
    FSlateIcon(FSceneProtocolEditorStyle::GetStyleSetName(), "SceneProtocolEditor.SceneProtocolIcon"));
}

TSharedRef<SWidget> SPNBU::FSceneProtocolEditorModule::CreateSceneProtocolMenu()
{
  FMenuBuilder SceneProtocolMenu( /*bInShouldCloseWindowAfterMenuSelection=*/ true, PluginCommands);
  AddSceneProtocolMenuItems(SceneProtocolMenu);
  return SceneProtocolMenu.MakeWidget();
}


void SPNBU::FSceneProtocolEditorModule::AddSceneProtocolMenuItems(FMenuBuilder& Builder)
{
  Builder.AddMenuEntry(FSceneProtocolEditorCommands::Get().FetchSceneAction);
  Builder.AddMenuEntry(FSceneProtocolEditorCommands::Get().CancelFetchAction);
  Builder.AddMenuEntry(FSceneProtocolEditorCommands::Get().ClientStartAction);
  Builder.AddMenuEntry(FSceneProtocolEditorCommands::Get().ClientStopAction);
  Builder.AddMenuEntry(FSceneProtocolEditorCommands::Get().PushAllItems);
  Builder.AddMenuEntry(FSceneProtocolEditorCommands::Get().PushSelectedItems);
}



void SPNBU::FSceneProtocolEditorModule::DoFetchScene()
{
  if (!SceneProtocolModule::IsAvailable()) {

    FText DialogText = LOCTEXT("Unable to load bridge",
      "Unable to load SceneProtocol module.");

    FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    return;
  }

  SceneProtocolModule& bridge = SceneProtocolModule::Get();

  USceneProtocolOptions* options = GetMutableDefault<USceneProtocolOptions>();
  if (options) {

    //Undo begin for scene fetch
    GEditor->BeginTransaction(FText::FromString("Import SceneProtocol Scene"));
    const int status = bridge.FetchScene(options);
    //Undo end for scene fetch
    GEditor->EndTransaction();
    if (status > 0) {
      FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BridgeFetchError",
        "An error occurred during scene retrieval, please check the log for details."));
      GEditor->UndoTransaction();
    }
  }
}

void SPNBU::FSceneProtocolEditorModule::DoCancelFetch()
{
  if (!SceneProtocolModule::IsAvailable()) {

    FText DialogText = LOCTEXT("Unable to load bridge",
      "Unable to load SceneProtocol module.");

    FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    return;
  }
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  bridge.CancelFetch();
}

void SPNBU::FSceneProtocolEditorModule::DoStartClient()
{
  if (!SceneProtocolModule::IsAvailable()) {

    FText DialogText = LOCTEXT("Unable to load bridge",
      "Unable to load SceneProtocol module.");

    FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    return;
  }

  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  USceneProtocolOptions* options = GetMutableDefault<USceneProtocolOptions>();
  if (options) {
    const int status = bridge.StartClient(options);
    if (status > 0) {
      FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BridgeStartClientError",
        "An error occurred during startup of the client thread, please check the log for details."));
    }
  }
}

void SPNBU::FSceneProtocolEditorModule::DoStopClient()
{
  if (!SceneProtocolModule::IsAvailable()) {

    FText DialogText = LOCTEXT("Unable to load bridge",
      "Unable to load SceneProtocol module.");

    FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    return;
  }
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  bridge.StopClient();
}

void SPNBU::FSceneProtocolEditorModule::DoPushAll()
{
  if (!SceneProtocolModule::IsAvailable()) {

    FText DialogText = LOCTEXT("Unable to load bridge",
      "Unable to load SceneProtocol module.");

    FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    return;
  }

  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  bridge.PushItems(false);
}

void SPNBU::FSceneProtocolEditorModule::DoPushSelected()
{
  if (!SceneProtocolModule::IsAvailable()) {

    FText DialogText = LOCTEXT("Unable to load bridge",
      "Unable to load SceneProtocol module.");

    FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    return;
  }
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  bridge.PushItems(true);
}


bool SPNBU::FSceneProtocolEditorModule::CanDoFetchScene()
{
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  return bridge.IsClientStarted() && !bridge.IsFetching();
}

bool SPNBU::FSceneProtocolEditorModule::CanDoCancelFetch()
{
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  return bridge.IsClientStarted() && bridge.IsFetching();
}

bool SPNBU::FSceneProtocolEditorModule::CanDoStartClient()
{
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  return !bridge.IsClientStarted();
}

bool SPNBU::FSceneProtocolEditorModule::CanDoStopClient()
{
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  return bridge.IsClientStarted();
}

bool SPNBU::FSceneProtocolEditorModule::CanDoPushAll()
{
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  return bridge.IsClientStarted();
}

bool SPNBU::FSceneProtocolEditorModule::CanDoPushSelected()
{
  SceneProtocolModule& bridge = SceneProtocolModule::Get();
  if (!bridge.IsClientStarted()) {
    return false;
  }

  USelection* selection = GEditor->GetSelectedActors();
  return selection->Num() > 0;
}

void SPNBU::FSceneProtocolEditorModule::SwapOutCustomMeshClasses()
{
	// get all sp actors
	// get mesh asset from SPSM
	// get fassetdata - i.e get the asset disk location
	// run replace function

	if (GWorld != NULL)
	{
		if (GWorld->IsEditorWorld() && !GWorld->IsPlayInEditor())
		{
			ULevel* desiredLevel = GWorld->GetCurrentLevel();

			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			for (TActorIterator<ASceneProtocolStaticMeshActor> ActorItr(desiredLevel->OwningWorld); ActorItr; ++ActorItr)
			{
				ASceneProtocolStaticMeshActor *Mesh = *ActorItr;
				UActorFactory* Factory = GEditor->FindActorFactoryByClass(UActorFactoryStaticMesh::StaticClass());
				UObject* outer = Mesh->GetStaticMeshComponent()->GetStaticMesh();
				FAssetData Asset = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*(outer->GetPathName())));

				TArray<AActor*> ThisActor;
				ThisActor.Add(Mesh);
				GEditor->ReplaceActors(Factory, Asset, ThisActor);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(SPNBU::FSceneProtocolEditorModule, SceneProtocolEditor)
