// --------------------------------------------------------------------------
// Implementation of the UnrealEngine AssetFactory.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "AssetFactory.h"
#include "MetadataUtils.h"
#include "SceneProtocol.h"
#include "NodeHandler.h"
#include "Handlers/HandlerUtils.h"

#include "Response.h"
#include "ContentReader.h"
#include "CoreReaders/MeshReader.h"
#include "CoreReaders/MaterialReader.h"
#include "Names.h"

#include "Editor.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"

#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "PackageTools.h"
#include "Developer/AssetTools/Public/IAssetTools.h"

#include "PackageMapper.h"

#include <cassert>

// Currently, a Package is created for each of the types. This is not strictly
// necessary for Unreal for instances of types that don't also create an asset, 
// i.e. cameras and lights that don't have metadata.
//
// However, Packages are used for book-keeping while fetching, so they shouldn't
// be removed without first devising a new way to do this.
//

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
  bool isBuiltinType(const std::string& type)
  {
    namespace STPN = SceneTransmissionProtocol::Names;

    return STPN::ResponseTypeMesh == type ||
      STPN::ResponseTypeMaterial == type ||
      STPN::ResponseTypeMaterialDefinition == type ||
      STPN::ResponseTypeTexture == type ||
      STPN::ResponseTypeCamera == type ||
      STPN::ResponseTypeLight == type;
  }

  // Helper RAII for the AssetHandlerFactory, but this code will expand
  // in the future so may as well get it in now.
  struct FactoryRAII
  {
    FactoryRAII(SPNB::Unreal::AssetHandlerFactory& f)
      : _factory(f), _handler(_factory.create()) {}

    ~FactoryRAII() {
      _factory.destroy(_handler);
    }

    SPNB::Unreal::IAssetHandler* handler() {
      return _handler;
    }

  private:
    SPNB::Unreal::AssetHandlerFactory& _factory;
    SPNB::Unreal::IAssetHandler* _handler;
  };
} // anonymous namespace


namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {

      AssetFactory* AssetFactory::_instance = nullptr;

      AssetFactory& AssetFactory::instance()
      {
        if (!_instance)
        {
          _instance = new AssetFactory();
        }
        return *_instance;
      }

      SPNB::BridgeProtocol::IPackage* AssetFactory::createPackage(const std::string& name)
      {
        FName packageFName(ANSI_TO_TCHAR(name.c_str()));
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();

        Package* newPackage = new Package(name);
        return newPackage;
      }

      bool AssetFactory::createAndRegisterItemPackage(
          const std::string& packageName,
          const STP::Client::Response *response, 
          std::string& uniqueName,
          const char* packageRoot,
          AssetHandlerFactory handlerFactory)
      {
        FString finalPackageName;
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();
        std::string token = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
        if (packageName.empty()) {
          finalPackageName = packageMapper.generatePackageName(FString(packageRoot), token);
        }
        else {
          finalPackageName = FString(packageName.c_str());
        }

        bool success = false;

        if (handlerFactory.isValid()) {
          FactoryRAII factory(handlerFactory);

          if (packageMapper.hasPackage(FName(*finalPackageName))) {
            auto package = packageMapper.getPackageFromPackageName(finalPackageName);
            success = factory.handler()->update(package, response, uniqueName);
          }
          else {
            SPNB::BridgeProtocol::IPackage* package = createPackage(TCHAR_TO_UTF8(*finalPackageName));

            if (package)
            {
              uniqueName = package->name();
              success = factory.handler()->create(package, response, uniqueName);

              if (success)
              {
                packageMapper.registerOwnedPackage(token, package);
              }
              else
              {
                // Cleanup
                delete package;
              }
            }
          }
        }

        return success;
      }

      bool AssetFactory::createMaterialDefinition(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, MATERIAL_DEF_PACKAGE_ROOT, _materialDefinitionHandler);
      }

      bool AssetFactory::createMaterial(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, MATERIAL_INST_PACKAGE_ROOT, _materialHandler);
      }

      bool AssetFactory::createTexture(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, TEXTURES_PACKAGE_ROOT, _textureHandler);
      }

      bool AssetFactory::createLight(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, LIGHTS_PACKAGE_ROOT, _lightHandler);
      }

      bool AssetFactory::createStaticMesh(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, MESH_PACKAGE_ROOT, _meshHandler);
      }

      bool AssetFactory::createMeshInstance(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, MESH_INST_PACKAGE_ROOT, _meshInstanceHandler);
      }

      bool AssetFactory::createCamera(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        return createAndRegisterItemPackage(packageName, response, uniqueName, CAMERA_PACKAGE_ROOT, _cameraHandler);
      }

      bool AssetFactory::createCustom(const std::string& packageName, const STP::Client::Response *response, std::string& uniqueName)
      {
        const USceneProtocolOptions* options = NodeHandlerManager::get()->getOptions();
        if (!options)
        {
          return false;
        }

        const std::string token = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
        if (token.empty())
        {
          return false;
        }

        FString customPackageName = TEXT("/Game/SceneProtocol/Custom");
        FString tokenString = FString(token.c_str());
        customPackageName /= GetHashedInputPath(tokenString);

        bool success = false;
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();

        if (!packageMapper.hasPackage(FName(*customPackageName))) {
          const std::string& type = SPNB::Unreal::stringDataToStdString(response->getType());
          auto result = _customHandlers.find(ANSI_TO_TCHAR(type.c_str()));

          if (result != _customHandlers.end()) {
            SPNB::BridgeProtocol::IPackage* package = createPackage(TCHAR_TO_UTF8(*customPackageName));

            if (package)
            {
              FactoryRAII factory(result->second);
              success = factory.handler()->create(package, response, uniqueName);

              if (success)
              {
                packageMapper.registerOwnedPackage(token, package);
              }
              else
              {
                // Cleanup
                delete package;
              }
            }
          }
        }
        return success;
      }

      // Currently custom types are mapped to a single factory. This may be updated
      // to support chained handlers at some point.
      void AssetFactory::registerHandler(const std::string& type, AssetHandlerFactory& factory)
      {
        if (type == SceneTransmissionProtocol::Names::ResponseTypeMesh)
        {
          if (!_meshHandler.isValid())
          {
            _meshHandler = factory;
          }
        }
        else if (type == SceneTransmissionProtocol::Names::ResponseTypeMeshInstance)
        {
            if (!_meshInstanceHandler.isValid())
            {
                _meshInstanceHandler = factory;
            }
        }
        else if (type == SceneTransmissionProtocol::Names::ResponseTypeMaterialDefinition)
        {
            if (!_materialDefinitionHandler.isValid())
            {
                _materialDefinitionHandler = factory;
            }
        }
        else if (type == SceneTransmissionProtocol::Names::ResponseTypeMaterial)
        {
          if (!_materialHandler.isValid())
          {
            _materialHandler = factory;
          }
        }
        else if (type == SceneTransmissionProtocol::Names::ResponseTypeTexture)
        {
          if (!_textureHandler.isValid())
          {
            _textureHandler = factory;
          }
        }
        else if (type == SceneTransmissionProtocol::Names::ResponseTypeCamera)
        {
          if (!_cameraHandler.isValid())
          {
            _cameraHandler = factory;
          }
        }
        else if (type == SceneTransmissionProtocol::Names::ResponseTypeLight)
        {
          if (!_lightHandler.isValid())
          {
            _lightHandler = factory;
          }
        }
        else {
          // It's a custom handler.
          FName fname = ANSI_TO_TCHAR(type.c_str());
          _customHandlers[fname] = factory;
        }
      }

      AssetFactory::AssetFactory() :
        IAssetFactory(),
        _assetToolsModule(FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools"))
      {
      }

      void AssetFactory::FillMetadataAsset(UMetadataAsset* Metadata, const STP::Client::Response *response)
      {
        const auto inputCount = response->getContentReader().getKeyCount(STP::Readers::ContentReader::Dynamic);
        for (auto i = 0; i < inputCount; ++i)
        {
          const auto metaName = SPNB::Unreal::stringDataToStdString(response->getContentReader().getKey(STP::Readers::ContentReader::Dynamic, i));
          const auto type = response->getContentReader().getDataType(STP::Readers::ContentReader::Dynamic, i);
          const auto key = metaName.substr(metaName.rfind('.') + 1);

          switch (type)
          {
          case STP::Types::String:
            {
              bool isOK = false;
              std::string data = SPNB::Unreal::stringDataToStdString(response->getContentReader().getString(STP::Readers::ContentReader::Dynamic, key.c_str(), &isOK));
              if (!isOK) {
                UE_LOG(LogTemp, Warning, TEXT("\tFailed to get string data for %s"), UTF8_TO_TCHAR(metaName.c_str()));
              }
              FString fKey = FString(key.c_str());
              FString fData = FString(data.c_str());
              FStrings stringVals;
              stringVals.Items.Add(fData);
              Metadata->Strings.Add(fKey, stringVals);
            }
            break;

          case STP::Types::Integer:
            {
              bool isOK = false;
              int32_t data = response->getContentReader().getInteger(STP::Readers::ContentReader::Dynamic, key.c_str(), &isOK);
              if (!isOK) {
                UE_LOG(LogTemp, Warning, TEXT("\tFailed to get scalar integer data for %s"), UTF8_TO_TCHAR(metaName.c_str()));
              }
              FIntegers intVals;
              intVals.Items.Add(data);
              Metadata->Integers.Add(FString(key.c_str()), intVals);
            }
            break;

          case STP::Types::Float:
            {
              bool isOK = false;
              float data = response->getContentReader().getFloat(STP::Readers::ContentReader::Dynamic, key.c_str(), &isOK);
              if (!isOK) {
                UE_LOG(LogTemp, Warning, TEXT("\tFailed to get scalar float data for %s"), UTF8_TO_TCHAR(metaName.c_str()));
              }
              FFloats floatVals;
              floatVals.Items.Add(data);
              Metadata->Floats.Add(FString(key.c_str()), floatVals);
            }
            break;

          default:
            UE_LOG(LogSceneProtocol, Warning, TEXT("Failed to understand data %s of type %s"),
              *FString(metaName.c_str()),
              *FString(STP::Types::dataTypeToString(type)));
          }
        }
      }
    }
  }
}