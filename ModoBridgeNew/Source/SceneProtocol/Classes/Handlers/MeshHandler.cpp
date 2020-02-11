// --------------------------------------------------------------------------
// Handler for SGP mesh types.
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
#include "StaticMeshImport.h"
#include "SceneProtocolUserData.h"
#include "NodeHandler.h"
#include "Names.h"

#include "Editor.h"
#include "AssetRegistryModule.h"
#include "EngineUtils.h"


namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

// The handler is not publically exposed and is only accessed via the AssetFactory
// construction mechanism and the IAssetHandler interface.
namespace {

  class MeshHandler : public SPNB::Unreal::IAssetHandler
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

    static UStaticMesh* findMesh(const STP::Client::Response* response);
  };

  // Statically allocated handler.
  MeshHandler _theMeshHandler;


  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createMeshHandler()
  {
    return &_theMeshHandler;
  }

  /** Factory destruction function */
  void destroyMeshHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of MeshHandler against a type name, with its factory
  *  creation/update functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(MeshHandler, STP::Names::ResponseTypeMesh, createMeshHandler, destroyMeshHandler);

  bool MeshHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
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

    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);

    SPNB::Unreal::StaticMeshImporter importer(upackage, FName(*displayName), options);

    std::vector<UObject*> objects;

    bool addedAny = false;
    const auto materials = SPNB::Unreal::PackageMapper::instance().getMaterials(response);
    addedAny = importer.appendMesh(response, materials);

    if (addedAny)
    {
      uniqueName = package->name();

      UStaticMesh* ueMesh = importer.finalizeAndReturn().release();
      USceneProtocolUserData* assetUserData = NewObject< USceneProtocolUserData >(ueMesh, NAME_None, RF_Public);
      assetUserData->itemToken = *nodeName;
      ueMesh->AddAssetUserData(assetUserData);

      // Static mesh object gets added first
      objects.push_back(ueMesh);

      SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objects);

      return true;
    }
    // else
    return false;
  }

  bool MeshHandler::update(
      SPNB::BridgeProtocol::IPackage* package,
      const STP::Client::Response* response,
      std::string& uniqueName)
  {
    UStaticMesh* mesh = findMesh(response);
    if (mesh) {
      FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
      FString displayName = SPNB::Unreal::stringDataToFString(response->getDisplayName());
      if (displayName.IsEmpty())
      {
        displayName = FPackageName::GetShortName(nodeName);
      }

      if (!displayName.IsEmpty() && mesh->GetName() != displayName)
      {
        if (StaticFindObject(UStaticMesh::StaticClass(), ANY_PACKAGE, *displayName) != nullptr)
        {
          displayName = MakeUniqueObjectName(ANY_PACKAGE, UStaticMesh::StaticClass(), *displayName).ToString();
        }

        mesh->Rename(*displayName);
      }

      const auto materials = SPNB::Unreal::PackageMapper::instance().getMaterials(response);
      SPNB::Unreal::StaticMeshImporter importer(mesh);
      
      bool addedAny = importer.appendMesh(response, materials);
      
      if (addedAny)
      {
        importer.finalize();
      }
    }

    return false;
  }

  UStaticMesh* MeshHandler::findMesh(const STP::Client::Response* response)
  {
    std::string token = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
    UObject* Asset = SPNB::Unreal::PackageMapper::instance().findObject(token);

    if (Asset && Asset->IsA(UStaticMesh::StaticClass())) 
    {
      return Cast<UStaticMesh>(Asset);
    }

    return nullptr;
  }
} // anonymous namespace
