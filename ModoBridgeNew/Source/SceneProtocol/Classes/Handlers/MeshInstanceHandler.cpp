// --------------------------------------------------------------------------
// Handler for STP mesh instance types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "SceneProtocol.h"
#include "HandlerUtils.h"

#include "MetadataUtils.h"
#ifdef STP_CUSTOM_ACTOR_CLASSES
#include "SceneProtocolStaticMeshActor.h"
#endif
#include "SceneProtocolUserData.h"
#include "NodeHandler.h"
#include "Names.h"

#include "CoreReaders/MeshInstanceReader.h"

#include "Editor.h"
#include "AssetRegistryModule.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"

#include <cassert>

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

// The handler is not publically exposed and is only accessed via the AssetFactory
// construction mechanism and the IAssetHandler interface.
namespace {

  class MeshInstanceHandler : public SPNB::Unreal::IAssetHandler
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

    static AStaticMeshActor* findActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response);
  };

  // Statically allocated handler.
  MeshInstanceHandler _theMeshInstanceHandler;


  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createMeshInstanceHandler()
  {
    return &_theMeshInstanceHandler;
  }

  /** Factory destruction function */
  void destroyMeshInstanceHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of MeshInstanceHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(MeshInstanceHandler, STP::Names::ResponseTypeMeshInstance, createMeshInstanceHandler, destroyMeshInstanceHandler);

  bool MeshInstanceHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
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

    STP::Readers::MeshInstanceReader meshInst(response->getContentReader());

    // Check if we have the original mesh mapped
    UStaticMesh* mesh = nullptr;
    const std::string originalMeshToken = SPNB::Unreal::stringDataToStdString(meshInst.getMeshToken());

    // If the original mesh token is not empty we need to find the mesh in the package mapper
    // or else bail out
    if (!originalMeshToken.empty())
    {
      UObject* object = SPNB::Unreal::PackageMapper::instance().findObject(originalMeshToken);

      if (object && object->IsA(UStaticMesh::StaticClass()))
      {
        mesh = Cast<UStaticMesh>(object);
      }

      if (!mesh)
      {
        return false;
      }
    }

    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);

    FActorSpawnParameters spawnParams;
    AStaticMeshActor* actor = nullptr;
    auto worldTransform = SPNB::Unreal::getWorldTransform(response);

    std::vector<UObject*> objects;

#ifdef STP_CUSTOM_ACTOR_CLASSES
    if (options->UseCustomActorClasses)
    {
      ASceneProtocolStaticMeshActor* sceneProtocolActor = GEditor->GetEditorWorldContext().World()->SpawnActor<ASceneProtocolStaticMeshActor>(ASceneProtocolStaticMeshActor::StaticClass(),
        worldTransform,
        spawnParams);

#ifdef STP_METADATA_SUPPORT
      if (options->ImportMetadata) {
        FString metadataName = nodeName + "_metadataAssets";
        sceneProtocolActor->Metadata = NewObject<UMetadataAsset>(upackage->package(), FName(*metadataName), RF_Standalone | RF_Public);
        SPNB::Unreal::AssetFactory::instance().FillMetadataAsset(sceneProtocolActor->Metadata, response);
        objects.push_back(sceneProtocolActor->Metadata);
      }
#endif

      actor = sceneProtocolActor;
    }
    else
#endif
    {
      actor = GEditor->GetEditorWorldContext().World()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
        worldTransform,
        spawnParams);
    }

#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
    SPNB::Unreal::attachParentActor(response, actor);
#endif

    // re-assign the transform to the Actor, otherwise it is concatenated against the value set in previous imports
    // see AActor::PostSpawnInitialize
    auto SceneRootComponent = actor->GetRootComponent();
    check(SceneRootComponent);
    SceneRootComponent->SetWorldTransform(worldTransform);

    if (actor)
    {
      objects.push_back(actor);
      uniqueName = package->name();

      UStaticMeshComponent* meshComponent = actor->GetStaticMeshComponent();
      USceneProtocolUserData* actorUserData = NewObject< USceneProtocolUserData >(meshComponent, NAME_None, RF_Public);
      actorUserData->itemToken = *nodeName;
      meshComponent->AddAssetUserData(actorUserData);

      SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objects);

      actor->Modify(true);
      actor->SetMobility(EComponentMobility::Movable);

      if (mesh)
      {
        actor->GetStaticMeshComponent()->SetStaticMesh(mesh);
      }

      actor->SetActorLabel(*displayName);

      return true;
    }

    return false;
  }

  bool MeshInstanceHandler::update(
    SPNB::BridgeProtocol::IPackage* package,
    const STP::Client::Response* response,
    std::string& uniqueName)
  {
    AStaticMeshActor* actor = findActor(package, response);
    if (actor) {
      FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
      FString displayName = SPNB::Unreal::stringDataToFString(response->getDisplayName());
      if (displayName.IsEmpty())
      {
        displayName = FPackageName::GetShortName(nodeName);
      }

      // Update the name
      if (!displayName.IsEmpty() && actor->GetName() != displayName)
      {
        actor->SetActorLabel(*displayName);
      }

      // Update the transformation
      auto worldTransform = SPNB::Unreal::getWorldTransform(response);
      auto SceneRootComponent = actor->GetRootComponent();
      check(SceneRootComponent);
      SceneRootComponent->SetWorldTransform(worldTransform);

      // Update the mesh item
      STP::Readers::MeshInstanceReader meshInst(response->getContentReader());
      const std::string originalMeshToken = SPNB::Unreal::stringDataToStdString(meshInst.getMeshToken());

      // If the original mesh token is not empty we need to find the mesh in the package mapper
      if (!originalMeshToken.empty())
      {
        UObject* object = SPNB::Unreal::PackageMapper::instance().findObject(originalMeshToken);

        if (object && object->IsA(UStaticMesh::StaticClass()))
        {
          UStaticMesh* mesh = Cast<UStaticMesh>(object);

          if (mesh)
          {
            actor->Modify(true);
            actor->GetStaticMeshComponent()->SetStaticMesh(mesh);
          }
        }
      }
    }

    return false;
  }

  AStaticMeshActor* MeshInstanceHandler::findActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response)
  {
    FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
    if (nodeName.Len() == 0)
    {
      return nullptr;
    }

    AStaticMeshActor* actor = nullptr;

    for (TActorIterator<AStaticMeshActor> Itr(GEditor->GetEditorWorldContext().World()); Itr; ++Itr) {
      USceneProtocolUserData* userData = (USceneProtocolUserData*)Itr->GetStaticMeshComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
      if (userData && userData->itemToken == FName(*nodeName)) {
        actor = *Itr;
      }
    }

    return actor;
  }
} // anonymous namespace
