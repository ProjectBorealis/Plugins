// --------------------------------------------------------------------------
// Handler for SGP camera types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "SceneProtocol.h"
#include "NodeHandler.h"
#include "HandlerUtils.h"

#include "CoreReaders/CameraReader.h"
#include "ContentReader.h"
#include "Names.h"
#include "StringData.h"

#include "Editor.h"
#include "AssetRegistryModule.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

#include <string>

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
  class CameraHandler : public SPNB::Unreal::IAssetHandler
  {
  public:
    virtual bool create(
        SPNB::BridgeProtocol::IPackage* package,
        const STP::Client::Response* response,
        std::string& uniqueName) override;

    virtual bool update(
        SPNB::BridgeProtocol::IPackage* package,
        const STP::Client::Response* response,
        std::string& uniqueName) override;


    // Utility functions
    static void setParameters(const STP::Readers::CameraReader& cameraReader, UCameraComponent* camComponent);
#ifdef STP_METADATA_SUPPORT
    static UMetadataAsset* createMetadata(UPackage* package, const STP::Client::Response *response, const FName& name);
#endif
    static ACameraActor* findPawn(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response);
  };

  // Statically allocated handler.
  CameraHandler _theCameraHandler;


  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createCameraHandler()
  {
    return &_theCameraHandler;
  }

  /** Factory destruction function */
  void destroyCameraHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of CameraHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(CameraHandler, STP::Names::ResponseTypeCamera, createCameraHandler, destroyCameraHandler);

  bool CameraHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();
    if (!options)
    {
      return false;
    }

    FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
    if (nodeName.IsEmpty())
    {
      return false;
    }

    FString displayName = SPNB::Unreal::stringDataToFString(response->getDisplayName());
    if (displayName.IsEmpty())
    {
      displayName = FPackageName::GetShortName(nodeName);
    }

    SceneTransmissionProtocol::Readers::CameraReader cameraReader(response->getContentReader());
    if (!cameraReader.isValid()) {
      return false;

    }

    auto cameraType = cameraReader.getType();

    UWorld* world = GEditor->GetEditorWorldContext().World();
    if (!world)
    {
      return false;
    }

    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);
    auto worldTransform = SPNB::Unreal::getWorldTransform(response);

    // TODO: Create custom ACameraActor and reimplement metadata support
    FActorSpawnParameters spawnParams;
    ACameraActor* actor = GEditor->GetEditorWorldContext().World()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), worldTransform, spawnParams);
    UCameraComponent* camComponent = actor->GetCameraComponent();

    // Rotate the camera component by -90 degrees on the Z axis to get it inline with the Modo camera
    camComponent->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

    actor->SetActorLabel(*displayName);

    // re-assign the transform to the Actor, otherwise it is concatenated against the value set in previous imports
    // see AActor::PostSpawnInitialize
    auto SceneRootComponent = actor->GetRootComponent();
    check(SceneRootComponent);
    SceneRootComponent->SetWorldTransform(worldTransform);

    // Set custom user data
    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(camComponent, NAME_None, RF_Public);
    userData->itemToken = *nodeName;
    camComponent->AddAssetUserData(userData);

    setParameters(cameraReader, camComponent);

    std::vector<UObject*> objs{ actor };
    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    return true;
  }

  bool CameraHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response, std::string& uniqueName)
  {
    ACameraActor* actor = findPawn(package, response);
    if (actor) {
      const std::string nodeName = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
      SceneTransmissionProtocol::Readers::CameraReader cameraReader(response->getContentReader());
      if (!cameraReader.isValid()) {
        return false;
      }

      // re-assign the transform to the Actor, otherwise it is concatenated against the value set in previous imports
      // see AActor::PostSpawnInitialize
      auto SceneRootComponent = actor->GetRootComponent();
      check(SceneRootComponent);
      auto worldTransform = SPNB::Unreal::getWorldTransform(response);
      SceneRootComponent->SetWorldTransform(worldTransform);

      setParameters(cameraReader, actor->GetCameraComponent());
    }

    return true;
  }

  void CameraHandler::setParameters(const STP::Readers::CameraReader& cameraReader, UCameraComponent* camComponent)
  {
    if (cameraReader.getType() == SceneTransmissionProtocol::Readers::CameraReader::Perspective) {
      camComponent->FieldOfView = cameraReader.getVerticalFov();
      camComponent->AspectRatio = cameraReader.getAspectRatio();
    }
    else if (cameraReader.getType() == SceneTransmissionProtocol::Readers::CameraReader::Orthographic) {
      camComponent->OrthoWidth = cameraReader.getHorizontalMagnification();
    }

    camComponent->OrthoNearClipPlane = cameraReader.getNearClip();
    camComponent->OrthoFarClipPlane = cameraReader.getFarClip();
  }

#ifdef STP_METADATA_SUPPORT
  UMetadataAsset* CameraHandler::createMetadata(UPackage* package, const STP::Client::Response *response, const FName& name)
  {
    UMetadataAsset* metadata = NewObject<UMetadataAsset>(package, name, RF_Standalone | RF_Public);
    SPNB::Unreal::AssetFactory::instance().FillMetadataAsset(metadata, response);
    FAssetRegistryModule::AssetCreated(metadata);
    metadata->MarkPackageDirty();
    return metadata;
  }
#endif

  ACameraActor* CameraHandler::findPawn(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response)
  {
    const std::string nodeName = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
    if (nodeName.empty())
    {
      return nullptr;
    }

    ACameraActor* actor = nullptr;

    for (TActorIterator<ACameraActor> Itr(GEditor->GetEditorWorldContext().World()); Itr; ++Itr) {
      USceneProtocolUserData* userData = Cast<USceneProtocolUserData>(Itr->GetCameraComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));
      if (userData && userData->itemToken == FName(*FString(nodeName.c_str()))) {
        actor = *Itr;
      }
    }

    return actor;
  }
} // anonymous namespace
