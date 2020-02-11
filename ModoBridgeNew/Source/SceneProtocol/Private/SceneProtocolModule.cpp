// --------------------------------------------------------------------------
// Implementation of the core SceneProtocol module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolModule.h"
#include "SceneProtocol.h"
#include "NodeHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "Handlers/HandlerUtils.h"
#include "StaticMeshExport.h"

#include "Client.h"
#include "Response.h"
#include "Names.h"
#include "CoreReaders/MeshReader.h"
#include "CoreReaders/MeshInstanceReader.h"
#include "CoreReaders/MaterialReader.h"
#include "CoreReaders/MaterialDefinitionReader.h"
#include "CoreReaders/LightReader.h"
#include "CoreReaders/CameraReader.h"
#include "CoreReaders/TextureReader.h"
#include "CoreReaders/UpdateReader.h"

#include "HAL/ThreadSafeBool.h"

#include "Misc/ScopedSlowTask.h"

#include "Editor.h"
#include "EngineGlobals.h"
#include "CoreGlobals.h"
#include "Misc/FeedbackContext.h"
#include "Slate/SceneViewport.h"
#include "ISettingsModule.h"

#include "SceneProtocolOptions.h"

#include <chrono>
#include <thread>
#include <sstream>

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "SceneProtocol"

namespace SPNB = SceneProtocol::NetworkBridge;
namespace SPNBP = SceneProtocol::NetworkBridge::BridgeProtocol;

namespace STP = SceneTransmissionProtocol;
namespace STPC = SceneTransmissionProtocol::Client;

namespace {
    // This is used to communicate from the callback to the Module. It's not
    // possible to call a module method, because the callback is from another
    // thread and Unreal only allows module access from the main thread, i.e.
    // it will kick up a fuss if the  FModuleManager::LoadModuleChecked is
    // called on the module in the callback.
    FThreadSafeBool gFetchFinished(false);
}

bool UEcheckCancel(void* userData) {
  if (GWarn->ReceivedUserCancel()) {
    return true;
  }
  return false;
}

void UEonEmpty(void* userData) {
    UE_LOG(LogSceneProtocol, Log, TEXT("Fetch finished"));
    gFetchFinished.AtomicSet(true);
}


void handleResponse(STPC::Response* response, void* userData) {
  auto nodeHandler = SPNB::Unreal::NodeHandlerManager::get();
  if (nodeHandler->userCancel) {
    return;
  }
  const auto type = response->getType();
  if (STP::String::equal(STP::Names::ResponseTypeUpdate, type)) {
    STP::Readers::UpdateReader updateReader(response->getContentReader());
    if (!updateReader.isValid()) {
      // TODO: set an error?
      return;
    }
    // Handling update response types became obsolete by allowing server 
    // to send responses without client requests feature.
    // Update type still used as makeshift heartbeat and client ID transfer
    // TODO: Revisit this, remove update type and create proper ZMQ heartbeat messaging
    //nodeHandler->addUpdate(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeMesh, type)) {
    STP::Readers::MeshReader meshReader(response->getContentReader());
    if (!meshReader.isValid()) {
      // TODO: set an error?
      return;
    }
    nodeHandler->addMesh(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeMeshInstance, type)) {
      STP::Readers::MeshInstanceReader meshInstReader(response->getContentReader());
      if (!meshInstReader.isValid()) {
          // TODO: set an error
          return;
      }
      nodeHandler->addMeshInstance(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeMaterial, type)) {
    STP::Readers::MaterialReader materialReader(response->getContentReader());
    if (!materialReader.isValid()) {
      // TODO: set an error
      return;
    }
    nodeHandler->addMaterial(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeMaterialDefinition, type)) {
      STP::Readers::MaterialDefinitionReader materialDefReader(response->getContentReader());
      if (!materialDefReader.isValid()) {
          // TODO: set an error
          return;
      }
      nodeHandler->addMaterialDefinition(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeLight, type)) {
    STP::Readers::LightReader lightReader(response->getContentReader());
    if (!lightReader.isValid()) {
      // TODO: set an error
      return;
    }
    nodeHandler->addLight(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeCamera, type)) {
    STP::Readers::CameraReader cameraReader(response->getContentReader());
    if (!cameraReader.isValid()) {
      // TODO: set an error
      return;
    }
    nodeHandler->addCamera(std::move(response));
  }
  else if (STP::String::equal(STP::Names::ResponseTypeTexture, type)) {
    STP::Readers::TextureReader textureReader(response->getContentReader());
    if (!textureReader.isValid()) {
      // TODO: set an error
      return;
    }
    nodeHandler->addTexture(std::move(response));
  }
  else {
    // If it's an unknown type, then let the custom handlers have a try.
    nodeHandler->addCustom(std::move(response));
  }

  // wait to set the status here, so that we do not interupt a node in the middle of being handled
  // otherwise a great deal more verification will be needed in AssetManager
  if (GWarn->ReceivedUserCancel()) {
    nodeHandler->userCancel = true;
    return;
  }
}

namespace {
  void registerEventHandlers()
  {
    SPNB::Unreal::PackageMapper::instance().registerEventHandlers();
  }
}

void SPNB::Unreal::SceneProtocolModule::StartupModule()
{
  if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->RegisterSettings("Project", "Plugins", "SceneProtocol",
      LOCTEXT("SceneProtocolSettingsName", "SceneProtocol Bridge Settings"),
      LOCTEXT("SceneProtocolSettingsDesc", "Configure how to connect this project to the SceneProtocol platform"),
      GetMutableDefault<USceneProtocolOptions>());
  }

  _totalTime = 0.f;
  _tickDelegate = FTickerDelegate::CreateRaw(this, &SPNB::Unreal::SceneProtocolModule::Tick);
  _tickDelegateHandle = FTicker::GetCoreTicker().AddTicker(_tickDelegate);

  _clientStarted = false;
  _fetching = false;

  // TODO: requires investigation in future versions of UE, > 4.15
  // invoking the destructor for the Client object causes
  // zmq_ctx_destroy causes a crash, appearing to be related to the use of new and delete
  // within ZeroMQ ('delete this' in zmq::ctx_t::terminate), but how that is shoehorned
  // into the UE allocators is causing problems
  // therefore leaking the entire Client for now
  _client = new STP::Client::Client;

  FCoreDelegates::OnFEngineLoopInitComplete.AddStatic(registerEventHandlers);
}

bool SPNB::Unreal::SceneProtocolModule::Tick(float DeltaTime)
{
  _totalTime += DeltaTime;
  if (_totalTime > 0.05) {
    NodeHandlerManager::get()->poll();
    _totalTime = 0.f;
  }

  if (_fetching && gFetchFinished) {
      _fetching = false;
      gFetchFinished.AtomicSet(false);
  }
  return true;
}

void SPNB::Unreal::SceneProtocolModule::ShutdownModule()
{
  FTicker::GetCoreTicker().RemoveTicker(_tickDelegateHandle);

  if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->UnregisterSettings("Project", "Plugins", "SceneProtocol");
  }
}


int SPNB::Unreal::SceneProtocolModule::FetchScene(const USceneProtocolOptions* options)
{
  if (nullptr == GEditor->GetEditorWorldContext().World())
  {
    UE_LOG(LogSceneProtocol, Error, TEXT("No world specified"));
    return FetchResults::NoWorld;
  }

  //FScopedSlowTask ProgressTask(1.0f, LOCTEXT("SceneProtocolImport", "Importing from SceneProtocol..."));

#ifndef PLATFORM_LINUX
  if (!options->Mode == ESceneProtocolReceiveMode::File) {
    UE_LOG(LogSceneProtocol, Error, TEXT("Sorry, File mode is not currently available on this platform"));
    return 3;
  }
#endif

  //ProgressTask.EnterProgressFrame(0.05f, LOCTEXT("Connecting", "Connecting..."));
  const int cancelCheckInterval = 1000; // check for user cancel once every second

  auto nodeManager = NodeHandlerManager::get();
  nodeManager->userCancel = false;
  nodeManager->setOptions(options);

  if (IsRunningCommandlet())
  {
    _client->close();
    if (_client->getStatus () != STPC::Client::StatusOK)
      _client->open (TCHAR_TO_ANSI (*options->Server), options->Port, options->Timeout, cancelCheckInterval);
    
    // must wait on the entire fetch, or the commandlet will exit early
    STPC::Client::HostInterface hostInterface(handleResponse);
    hostInterface.checkCancel = UEcheckCancel;
    _client->fetch(hostInterface);
    // now handle all imported objects
    NodeHandlerManager::get()->poll();
  }
  else if (_client->isRunning())
  {
    _client->fetchToken("");
    _fetching = true;
    gFetchFinished.AtomicSet(false);
  }
  else
  {
    return FetchResults::ClientNotRunning;
  }

  if (_client->getStatus() == STPC::Client::Status::StatusError) {
    return FetchResults::TransmissionFailure;
  }
  if (nodeManager->userCancel) {
    return FetchResults::UserCancel;
  }
  return FetchResults::Success;
}

void SPNB::Unreal::SceneProtocolModule::CancelFetch()
{
  // The ability to cancel is based on using a background fetch.
  // Otherwise, the UI/gamethread won't ever be able to poll the event loop, as it's busy on the synchronous fetch.
  if (_client && _client->isRunning())
  {
    _client->cancelFetch();
    _fetching = false;
    gFetchFinished.AtomicSet(true);
  }
}

int SPNB::Unreal::SceneProtocolModule::StartClient(const USceneProtocolOptions* options)
{
  const int cancelCheckInterval = 1000; // check for user cancel once every second

  // Refresh package maps
  SPNB::Unreal::PackageMapper::instance().clearMaps();
  SPNB::Unreal::PackageMapper::instance().generatePackageMaps();

  auto nodeManager = NodeHandlerManager::get();
  nodeManager->userCancel = false;
  nodeManager->setOptions(options);

  _client->open(TCHAR_TO_ANSI(*options->Server), options->Port, options->Timeout, cancelCheckInterval);
  if (_client->getStatus() == STPC::Client::Status::StatusError) {
    return FetchResults::TransmissionFailure;
  }

  STPC::Client::HostInterface hostInterface(handleResponse);
  hostInterface.checkCancel = UEcheckCancel;
  hostInterface.onEmpty = UEonEmpty;
  _client->startClient(hostInterface);

  _clientStarted = true;

  return FetchResults::Success;
}

void SPNB::Unreal::SceneProtocolModule::StopClient()
{
  _client->stopClient();
  _client->close();
  _clientStarted = false;
  _fetching = false;
}

int SPNB::Unreal::SceneProtocolModule::PushItems(bool selected)
{
  if (_client->getStatus() != STPC::Client::Status::StatusOK) {
    return FetchResults::ClientNotRunning;
  }

  SPNB::Unreal::StaticMeshExporter  exporter;
  std::vector<AStaticMeshActor*>    meshActors;

  TArray<UStaticMesh*>      staticMeshArray;
  TArray<AStaticMeshActor*> staticMeshActorArray;
  TArray<UMaterial*>        materialArray;

  float totalSlowTaskProgress = 100.0;
  int scopesNum = 4;
  float progressPerScope = totalSlowTaskProgress / scopesNum;
  FScopedSlowTask slowTask(totalSlowTaskProgress, NSLOCTEXT("UnrealBridge", "UnrealBridge_PushCommand", "Push command..."));
  slowTask.MakeDialog();

  {
    if (selected)
      exporter.getSelectedMeshActors(meshActors);
    else
      exporter.getAllMeshActors(meshActors);

    float progress = progressPerScope / meshActors.size();

    // Find unique static meshes
    for (size_t i = 0; i < meshActors.size(); i++) {
      slowTask.EnterProgressFrame(progress, NSLOCTEXT("UnrealBridge", "UnrealBridge_PushCommand", "Collecting items for push command..."));

      UStaticMesh* staticMeshPtr = meshActors[i]->GetStaticMeshComponent()->GetStaticMesh();
      if (staticMeshPtr == nullptr)
        continue;

      auto arraySize = staticMeshArray.Num();
      staticMeshArray.AddUnique(staticMeshPtr);

      if (arraySize < staticMeshArray.Num())
      {
        staticMeshActorArray.Add(meshActors[i]);
      }
    }
  }

  {
    float progress = progressPerScope / staticMeshActorArray.Num();

    // Send static meshes as MeshResponses
    for (size_t i = 0; i < staticMeshActorArray.Num(); i++) {
      STP::Client::Request* request = exporter.generateMeshRequest(staticMeshActorArray[i], materialArray);

      slowTask.EnterProgressFrame(progress, NSLOCTEXT("UnrealBridge", "UnrealBridge_PushCommand", "Pushing: Meshes..."));

      if (request)
      {
        if (_client->getStatus() == STPC::Client::Status::StatusOK) {
          _client->fetchRequest(request);
        }
        else
        {
          return FetchResults::ClientNotRunning;
        }
      }
    }
  }

  {
    float progress = progressPerScope / meshActors.size();

    // Send mesh instances for each actor
    for (size_t i = 0; i < meshActors.size(); i++) {
      STP::Client::Request* request = exporter.generateMeshInstanceRequest(meshActors[i]);

      slowTask.EnterProgressFrame(progress, NSLOCTEXT("UnrealBridge", "UnrealBridge_PushCommand", "Pushing: Actors..."));

      if (request)
      {
        if (_client->getStatus() == STPC::Client::Status::StatusOK) {
          _client->fetchRequest(request);
        }
        else
        {
          return FetchResults::ClientNotRunning;
        }
      }
    }
  }

  {
    float progress = progressPerScope / materialArray.Num();

    for (UMaterial* mat : materialArray) {
      STP::Client::Request* request = exporter.generateMaterialRequest(mat);

      slowTask.EnterProgressFrame(progress, NSLOCTEXT("UnrealBridge", "UnrealBridge_PushCommand", "Pushing: Materials..."));

      if (request)
      {
        if (_client->getStatus() == STPC::Client::Status::StatusOK) {
          _client->fetchRequest(request);
        }
        else
        {
          return FetchResults::ClientNotRunning;
        }
      }
    }
  }

  return FetchResults::Success;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(SPNB::Unreal::SceneProtocolModule, SceneProtocol)

