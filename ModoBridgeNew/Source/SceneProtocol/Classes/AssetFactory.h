// --------------------------------------------------------------------------
// Declaration of the UnrealEngine implementation of the IAssetFactory API.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#ifndef ASSETFACTORY__H
#define ASSETFACTORY__H

#include "IAssetFactory.h"
#include "Package.h"
#include "IAssetHandler.h"
#include "MetadataAsset.h"
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include "AssetToolsModule.h"
#include "SceneProtocolUserData.h"

#define MESH_PACKAGE_ROOT "/Game/SceneProtocol/Mesh"
#define MESH_INST_PACKAGE_ROOT "/Game/SceneProtocol/Instance"
#define LIGHTS_PACKAGE_ROOT "/Game/SceneProtocol/Lights"
#define TEXTURES_PACKAGE_ROOT "/Game/SceneProtocol/Textures"
#define MATERIAL_DEF_PACKAGE_ROOT "/Game/SceneProtocol/MaterialDefinitions"
#define MATERIAL_INST_PACKAGE_ROOT "/Game/SceneProtocol/MaterialInstances"
#define CAMERA_PACKAGE_ROOT "/Game/SceneProtocol/Camera"

class FString;
class FName;
class UObject;
class UMaterialInterface;

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace BridgeProtocol {
      class IPackage;

    }

    namespace Unreal {

      // Prevent Unreal leaking passed the public AssetFactory Boundary.  Will make switching engines
      // later a lot easier.
      //
      // Note that the package to object relationship may look odd for SceneProtocol, but thats because you 
      // don't currently support compound objects (required for animation), so whilst currently the
      // vector of UObjects per package seems redundant, it won't be once you support compound objects.
      class AssetFactory : public SceneProtocol::NetworkBridge::BridgeProtocol::IAssetFactory{
      public:
        static AssetFactory&  instance();

        // For package create pass in the name of the package needed, the package will then either be
        // opened if it already exists, or created if it doesn't, if this all goes well, return a pointer, 
        // to the created package, otherwise return nullptr.
        SceneProtocol::NetworkBridge::BridgeProtocol::IPackage*  createPackage(const std::string& name) override;

        // For all asset creates, pass in name of package to be created in, and the Node pointer containing the
        // data, item will be created if data is good, and it's unique name returned, unique name will only
        // be different in the case of a name clash.  If created return true, otherwise false.
        bool    createMaterialDefinition(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;
        bool    createMaterial(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;
        bool    createTexture(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;

        bool    createLight(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;
        bool    createStaticMesh(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;
        bool    createMeshInstance(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;
        bool    createCamera(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;
        bool    createCustom(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) override;

        // It is possible to register new handlers for types not covered above.
        // See the documentation in IAssetHandler for how to create these handlers.

        // registerHandler is used to register a new handler whose type is given by
        // "type". The "factory" parameter is used to create and destroy instances
        // of the handler during createCustom().
        void    registerHandler(const std::string& type, AssetHandlerFactory& factory);

        void FillMetadataAsset(UMetadataAsset* Metadata, const SceneTransmissionProtocol::Client::Response *response);

      private:
        AssetFactory();

        // Prevent incorrect compiler generated defaults.
        AssetFactory(const AssetFactory&);
        AssetFactory& operator=(const AssetFactory&);

        static AssetFactory*  _instance;

        bool createAndRegisterItemPackage(
            const std::string& packageName,
            const SceneTransmissionProtocol::Client::Response *response,
            std::string& uniqueName,
            const char* packageRoot,
            AssetHandlerFactory handlerFactory);

        FAssetToolsModule&                                                                  _assetToolsModule;

        // Custom handlers are mapped to names. Handlers for builtin types
        // cannot be overridden, so these are stored separately.
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
        std::map<FName, AssetHandlerFactory, cmpFNames> _customHandlers;
#else
		std::map<FName, AssetHandlerFactory> _customHandlers;
#endif
        AssetHandlerFactory _meshHandler;
        AssetHandlerFactory _meshInstanceHandler;
        AssetHandlerFactory _materialHandler;
        AssetHandlerFactory _materialDefinitionHandler;
        AssetHandlerFactory _lightHandler;
        AssetHandlerFactory _cameraHandler;
        AssetHandlerFactory _textureHandler;
      };
    }
  }
}

#endif
