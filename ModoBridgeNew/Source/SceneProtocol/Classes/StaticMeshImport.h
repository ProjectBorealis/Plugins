// --------------------------------------------------------------------------
// An importer for SGP mesh types into UnrealEngine.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "RawMesh.h"
#include "StaticMeshResources.h"

#include "Runtime/Launch/Resources/Version.h"

#include <string>
#include <memory>
#include <vector>

class UMaterialInterface;
class USceneProtocolOptions;

namespace SceneTransmissionProtocol {
namespace Client {
  class Response;
} // namespace Client
} // namespace SceneTransmissionProtocol

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace BridgeProtocol {
      class IPackage;
    }
    namespace Unreal {
      class Package;

      class StaticMeshImporter {

      public:
        StaticMeshImporter(Package* package, const FName& meshName, const USceneProtocolOptions* options);
        StaticMeshImporter(UStaticMesh* mesh);
        ~StaticMeshImporter();

        void                          initialize(Package* package, const FName& meshName, const USceneProtocolOptions* options);
        bool                          appendMesh(const SceneTransmissionProtocol::Client::Response *response, const std::vector<UMaterialInterface*> &materials);

        std::unique_ptr<UStaticMesh>  finalizeAndReturn();
        void finalize();

      protected:
        static const std::string  _root;

        UStaticMesh*  _mesh;
        FRawMesh      _rawMesh;
        int           _numFacesets = 0;
        int           _totalVertexIndex;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION > 13
        // Used to store materials so we don't trample over existing materials during mesh update
        TArray<FStaticMaterial> _staticMatStorage; 
#endif

        const USceneProtocolOptions* _options;
      };
    }
  }
}
