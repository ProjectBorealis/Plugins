// --------------------------------------------------------------------------
// Implementation of the node processing manager.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "NodeHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "Handlers/HandlerUtils.h"

#include "Names.h"
#include "SceneProtocolModule.h"
#include "Client.h"
#include "ContentReader.h"
#include "CoreReaders/CameraReader.h"
#include "CoreReaders/MeshReader.h"
#include "CoreReaders/MeshInstanceReader.h"
#include "CoreReaders/MaterialReader.h"
#include "CoreReaders/MaterialDefinitionReader.h"
#include "CoreReaders/LightReader.h"
#include "CoreReaders/UpdateReader.h"

#include "Materials/Material.h"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <chrono>

namespace SPNB = SceneProtocol::NetworkBridge;
namespace STP = SceneTransmissionProtocol;

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {
      NodeHandlerManager* NodeHandlerManager::_theInstance = nullptr;

      NodeHandlerManager* NodeHandlerManager::get()
      {
        if (!_theInstance) {
          _theInstance = new NodeHandlerManager;
        }
        return _theInstance;
      }

// TODO: Servers with local matrix implementation should upgrade parent transformation 
// storage to update with user scene graph changes in Unreal. Not doing that breaks children
// transfers if user does any changes to parent transformations.
#ifdef STP_LOCAL_MATRIX_PAYLOAD
      void NodeHandlerManager::setParent(const std::string &name, const std::string &parent)
      {
        _locationParent[name] = parent;
      }

      void NodeHandlerManager::setTransform(const std::string& name, const STP::Array::Handle &transform)
      {
        // incoming matrix is row-major, and so is FMatrix
        FMatrix localMatrix;
        localMatrix = constructUEMatrix(transform);
        _locationTransform[name] = localMatrix;
      }

      bool NodeHandlerManager::getParent(const std::string &name, std::string &outParent)
      {
        auto it = _locationParent.find(name);
        if (it != _locationParent.end())
        {
          outParent = it->second;
          return true;
        }
        return false;
      }

      bool NodeHandlerManager::getTransform(const std::string &name, FMatrix &outTransform)
      {
        auto it = _locationTransform.find(name);
        if (it != _locationTransform.end())
        {
          outTransform = it->second;
          return true;
        }
        UE_LOG(LogSceneProtocol, Error, TEXT("Unable to find transform for %s"), *FString(name.c_str()));
        return false;
      }
#endif

      void NodeHandlerManager::addMesh(STP::Client::Response* response)
      {
        _pendingMeshes.push_back(response);
      }

      void NodeHandlerManager::addMeshInstance(STP::Client::Response* response)
      {
        _pendingMeshInstances.push_back(response);
      }

      void NodeHandlerManager::addMaterialDefinition(STP::Client::Response* response)
      {
        _pendingMaterialDefs.push_back(response);
      }

      void NodeHandlerManager::addMaterial(STP::Client::Response* response)
      {
        _pendingMaterials.push_back(response);
      }

      void NodeHandlerManager::addLight(STP::Client::Response* response)
      {
        _pendingLights.push_back(response);
      }

      void NodeHandlerManager::addCamera(STP::Client::Response* response)
      {
        _pendingCameras.push_back(response);
      }

      void NodeHandlerManager::addTexture(SceneTransmissionProtocol::Client::Response* response)
      {
        _pendingTextures.push_back(response);
      }

      void NodeHandlerManager::addUpdate(SceneTransmissionProtocol::Client::Response* response)
      {
        _pendingUpdates.push_back(std::move(response));
      }

      void NodeHandlerManager::addCustom(STP::Client::Response* response)
      {
        _pendingCustom.push_back(response);
      }

#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
      bool isParentPresent(const SceneTransmissionProtocol::Client::Response* response)
      {
          const std::string parentName = SPNB::Unreal::stringDataToStdString(response->getParentToken());
          if (parentName.empty())
          {
              return true;
          }
          if (nullptr == PackageMapper::instance().findObject(parentName))
          {
              return false;
          }
          return true;
      }
#endif

      void NodeHandlerManager::poll()
      {
        auto &assetFactory = SPNB::Unreal::AssetFactory::instance();

        if (_pendingCameras.empty() && _pendingLights.empty() &&
            _pendingMaterialDefs.empty() && _pendingMaterials.empty() &&
            _pendingTextures.empty() &&
            _pendingMeshes.empty() && _pendingMeshInstances.empty() &&
            _pendingCustom.empty() && _pendingUpdates.empty())
        {
          return;
        }

        // Request updates back from the server
        auto pendingEnd = _pendingUpdates.end();
        for (auto it = _pendingUpdates.begin(); it != pendingEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif
          STP::Readers::UpdateReader updateReader(response->getContentReader());

          SPNB::Unreal::SceneProtocolModule& protocolModule = SPNB::Unreal::SceneProtocolModule::Get();

          STP::Array::Handle tokens = updateReader.getUpdateTokens();

          for (size_t i = 0; i < STP::Array::getArraySize(tokens); i++) {
            std::string token = SPNB::Unreal::stringDataToStdString(STP::Array::getStringArrayItem(tokens, i, nullptr));
            protocolModule._client->fetchToken(token.c_str());
          }

          // finished
          delete response;
          _pendingUpdates.erase(currentIt);
        }

        auto camerasEnd = _pendingCameras.end();
        // deal with objects with least dependencies first
        for (auto it = _pendingCameras.begin(); it != camerasEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif
          STP::Readers::CameraReader cameraReader(response->getContentReader());

          // all dependencies are met, instantiate
          std::string uniqueName;
          // TODO: is _nodeUObjectMap obsoleted by the AssetFactory?
          assetFactory.createCamera("", response, uniqueName);

          // finished
          delete response;
          _pendingCameras.erase(currentIt);
        }

        auto materialDefsEnd = _pendingMaterialDefs.end();
        for (auto it = _pendingMaterialDefs.begin(); it != materialDefsEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif
          if (!materialDefinitionDependenciesMet(response)) {
            continue;
          }

          std::string uniqueName;
          assetFactory.createMaterialDefinition("", response, uniqueName);

          // finished
          delete response;
          _pendingMaterialDefs.erase(currentIt);
        }


        auto materialsEnd = _pendingMaterials.end();
        for (auto it = _pendingMaterials.begin(); it != materialsEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif
          if (!materialDependenciesMet(response)) {
            continue;
          }

          // all dependencies are met, instantiate
          std::string uniqueName;
          // TODO: is _nodeUObjectMap obsoleted by the AssetFactory?
          assetFactory.createMaterial("", response, uniqueName);

          // finished
          delete response;
          _pendingMaterials.erase(currentIt);
        }

        auto lightsEnd = _pendingLights.end();
        for (auto it = _pendingLights.begin(); it != lightsEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif
          STP::Readers::LightReader lightReader(response->getContentReader());

          // TODO: check dependencies, e.g. materials

          // all dependencies are met, instantiate
          std::string uniqueName;
          assetFactory.createLight("", response, uniqueName);

          // finished
          delete response;
          _pendingLights.erase(currentIt);
        }

        auto textureEnd = _pendingTextures.end();
        for (auto it = _pendingTextures.begin(); it != textureEnd; )
        {
          auto currentIt = it++;
          auto response = *currentIt;
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif

          std::string uniqueName;
          assetFactory.createTexture("", response, uniqueName);

          delete response;
          _pendingTextures.erase(currentIt);

        }

        auto meshesEnd = _pendingMeshes.end();
        for (auto it = _pendingMeshes.begin(); it != meshesEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;

          if (!meshDependenciesMet(response)) {
              continue;
          }

          // all dependencies are met, instantiate
          std::string uniqueName;
          // TODO: is _nodeUObjectMap obsoleted by the AssetFactory?
          assetFactory.createStaticMesh("", response, uniqueName);

          // finished
          delete response;
          _pendingMeshes.erase(currentIt);

          // In order for the GUI to be responsive, only deal with one mesh per tick, otherwise
          // the game thread is too busy to perform updates
          // When running as a commandlet, there is no GUI, so you just want all meshes to be dealt with
          if (!IsRunningCommandlet())
          {
            break;
          }
        }

        auto meshesInstEnd = _pendingMeshInstances.end();
        for (auto it = _pendingMeshInstances.begin(); it != meshesInstEnd;)
        {
            auto currentIt = it++;
            auto response = *currentIt;

            if (!meshInstanceDependenciesMet(response)) {
                continue;
            }

            // all dependencies are met, instantiate
            std::string uniqueName;
            // TODO: is _nodeUObjectMap obsoleted by the AssetFactory?
            assetFactory.createMeshInstance("", response, uniqueName);

            // finished
            delete response;
            _pendingMeshInstances.erase(currentIt);

            // In order for the GUI to be responsive, only deal with one mesh per tick, otherwise
            // the game thread is too busy to perform updates
            // When running as a commandlet, there is no GUI, so you just want all meshes to be dealt with
            if (!IsRunningCommandlet())
            {
                break;
            }
        }

        auto customEnd = _pendingCustom.end();
        for (auto it = _pendingCustom.begin(); it != customEnd;)
        {
          auto currentIt = it++;
          auto response = *currentIt;

#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
          if (!isParentPresent(response))
          {
              continue;
          }
#endif

          std::string uniqueName;
          assetFactory.createCustom("", response, uniqueName);

          // finished
          delete response;
          _pendingCustom.erase(currentIt);

          // We have no way of knowing how long a custom handler will take, so we just assume that
          // it will take a while and, as with meshes, break if we're not running in a commandlet
          // so as to ensure the GUI stays responsive.
          if (!IsRunningCommandlet())
          {
            break;
          }
        }
      }

      void NodeHandlerManager::setOptions(const USceneProtocolOptions* options)
      {
        _options = options;
      }

      const USceneProtocolOptions* NodeHandlerManager::getOptions() const
      {
        return _options;
      }

      bool NodeHandlerManager::materialDependenciesMet(const SceneTransmissionProtocol::Client::Response* response)
      {
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
        if (!isParentPresent(response))
        {
          return false;
        }
#endif
        STP::Readers::MaterialReader materialReader(response->getContentReader());
        bool dependenciesMet = true;

        std::string target = SPNB::Unreal::stringDataToStdString(materialReader.getTarget());
        if (!target.empty()) {
          auto uMaterial = PackageMapper::instance().findObject(target);
          if (!uMaterial) {
            FString path = "/Game/" / FString(target.c_str());
            // It might also be that the material is prebuilt, so we need to look for that.
            UMaterial* pBase = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), 0, *path));
            if (!pBase) {
              dependenciesMet = false;
            }
          }
        }
        return dependenciesMet;
      }

      bool NodeHandlerManager::materialDefinitionDependenciesMet(const SceneTransmissionProtocol::Client::Response* response)
      {
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
        if (!isParentPresent(response))
        {
          return false;
        }
#endif
        STP::Readers::MaterialDefinitionReader materialDefReader(response->getContentReader());

        int32 paramCount = materialDefReader.getParameterCount();
        bool dependenciesMet = true;
        for (int32 i = 0; i < paramCount; ++i) {
          bool isOK = false;
          if (materialDefReader.getParameterType(i) == STP::Readers::MaterialDefinitionReader::Texture) {
            STP::String::RAII paramName = STP::Names::buildMaterialDefinitionParameterValueName(i);
            STP::String::Data texToken = response->getContentReader().getString(STP::Readers::ContentReader::Type, *paramName, &isOK);
            if (isOK) {
              auto uMaterial = PackageMapper::instance().findObject(SPNB::Unreal::stringDataToStdString(texToken));
              if (!uMaterial) {
                dependenciesMet = false;
                break;
              }
            }
          }
        }
        return dependenciesMet;
      }

      bool NodeHandlerManager::meshDependenciesMet(const SceneTransmissionProtocol::Client::Response* response)
      {
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
        if (!isParentPresent(response))
        {
          return false;
        }
#endif
        STP::Readers::MeshReader meshReader(response->getContentReader());

        bool dependenciesMet = true;
        // check if dependencies are ready - if not, continue
        const auto numFacesets = meshReader.getFacesetCount();
        if (numFacesets > 0) {
          auto anyFaceSetMaterialsMissing = false;

          // first check whether all faceset materials are present - if not, wait until the next poll
          for (auto fs = 0; fs < numFacesets; ++fs)
          {
            const std::string materialName = SPNB::Unreal::stringDataToStdString(meshReader.getFacesetMaterial(fs));
            if (!materialName.empty())
            {
              auto uMaterial = PackageMapper::instance().findObject(materialName);
              if (!uMaterial) {
                anyFaceSetMaterialsMissing = true;
                break;
              }
            }
          }

          if (anyFaceSetMaterialsMissing) {
            // not all faceset materials are present, try again later
            dependenciesMet = false;
          }
        }
        else {
          const std::string materialName = SPNB::Unreal::stringDataToStdString(meshReader.getMaterial());

          // meshes with no material assigned just have to pass through and get the default shader
          if (!materialName.empty()) {
            auto uMaterial = PackageMapper::instance().findObject(materialName);
            if (!uMaterial) {
              dependenciesMet = false;
            }
          }
        }
        return dependenciesMet;
      }

      bool NodeHandlerManager::meshInstanceDependenciesMet(const SceneTransmissionProtocol::Client::Response* response)
      {
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
        if (!isParentPresent(response))
        {
          return false;
        }
#endif
        STP::Readers::MeshInstanceReader meshInstReader(response->getContentReader());

        // check if dependencies are ready - if not, continue
        const auto token = SPNB::Unreal::stringDataToStdString(meshInstReader.getMeshToken());

        // If mesh token is empty create an empty actor in the scenegraph
        if (token.empty()) {
          return true;
        }

        auto mesh = PackageMapper::instance().findObject(token);

        if (!mesh) {
          return false;
        }

        return true;
      }


      NodeHandlerManager::NodeHandlerManager() :
        _options(nullptr)
      {
        _options = GetMutableDefault<USceneProtocolOptions>();
      }

      NodeHandlerManager::~NodeHandlerManager()
      {
        // Empty
      }

      FTransform getWorldTransform(const STP::Client::Response *response)
      {
// TODO: Create a flag for responses if they have payloads of local or world matrices and enable both code paths
// Case for STP servers implementations which send local matrices ( Katana )
#ifdef STP_LOCAL_MATRIX_PAYLOAD
        const auto nodeHandler = NodeHandlerManager::get();

        const auto entityToken = SPNB::Unreal::stringDataToStdString(response->getEntityToken());

        FMatrix localMatrix;
        nodeHandler->getTransform(entityToken, localMatrix);

#if defined(STP_ENABLE_SCENEGRAPH_HIERARCHY) && false
        // I was expecting to do this
        // but it appears to move all actors to the origin
        return FTransform(localMatrix);
#else
        //needed to get calculate the matrix hierarchy by walking up the nodes
        FMatrix worldMatrix = localMatrix * getParentMatrix(entityToken);
        return FTransform(worldMatrix);
#endif

// Case for Modo server where STP sends world transforms
#else
        // Get world matrix
        STP::Array::Handle transform = response->getTransform();
        return FTransform(constructUEMatrix(transform));
#endif
      }

      static std::vector<float> convertDoubleVectorToFloat(const std::vector<double> &doubleVec)
      {
        std::vector<float> floatVec;
        floatVec.reserve(doubleVec.size());
        std::transform(doubleVec.begin(), doubleVec.end(),
          std::back_inserter(floatVec),
          [](double value)
        {
          return static_cast<float>(value);
        });
        return floatVec;
      }

      FMatrix constructUEMatrix(const STP::Array::Handle& buffer) {
        if (STP::Array::getArraySize(buffer) != 16) {
          UE_LOG(LogSceneProtocol, Warning, TEXT("Transform array is incorrectly sized"));
        }

        std::vector<float> floatBuffer{
          1.f, 0.f, 0.f, 0.f,
          0.f, 1.f, 0.f, 0.f,
          0.f, 0.f, 1.f, 0.f,
          0.f, 0.f, 0.f, 1.f
        };

        if (STP::Array::getArraySize(buffer) == 16) {
          for (size_t i = 0; i < 16; ++i) {
            floatBuffer[i] = static_cast<float>(STP::Array::getDoubleArrayItem(buffer, i, nullptr));
          }
        }
        
        FMatrix uMatrix;
        memcpy(uMatrix.M, floatBuffer.data(), sizeof(float) * 16);
        return uMatrix;
      }

#ifdef STP_LOCAL_MATRIX_PAYLOAD
      FMatrix getParentMatrix(const std::string &name)
      {
        const auto nodeHandler = NodeHandlerManager::get();

        std::string parent;
        if (nodeHandler->getParent(name, parent)) {
          FMatrix transform;
          nodeHandler->getTransform(parent, transform);
          return transform * getParentMatrix(parent);
        }
        else {
          FMatrix id;
          id.SetIdentity();
          return id;
        }
      }
#endif
      FString getShortName(FString longName)
      {
        FString shortName = longName;
        int lastSlash;
        if (shortName.FindLastChar(L'/', lastSlash))
        {
          shortName = shortName.RightChop(lastSlash + 1);
          int lastDot;
          if (shortName.FindLastChar(L'.', lastDot))
          {
            shortName = shortName.Left(lastDot);
          }
        }
        return shortName;
      }
    } // namespace Unreal
  } // namespace NetworkBridge
} // namespace SceneProtocol
