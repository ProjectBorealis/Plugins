// --------------------------------------------------------------------------
// Implementation of the SGP mesh importer.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "StaticMeshImport.h"
#include "Package.h"

#include "SceneProtocol.h"
#include "SceneProtocolOptions.h"

#include "NodeHandler.h"
#include "MetadataUtils.h"
#include "Handlers/HandlerUtils.h"

#include "CoreReaders/MeshReader.h"
#include "ArrayData.h"

#include "BSPOps.h"
#include "PhysicsEngine/BodySetup.h"
#include "EditorSupportDelegates.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"

#include <cassert>
#include <array>

namespace SPNB = SceneProtocol::NetworkBridge;
namespace STP = SceneTransmissionProtocol;

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {

      StaticMeshImporter::StaticMeshImporter(Package* package, const FName& meshName, const USceneProtocolOptions* options) :
        _mesh(nullptr),
        _totalVertexIndex(0),
        _options(options)
      {
        initialize(package, meshName, options);
      }

      StaticMeshImporter::StaticMeshImporter(UStaticMesh* mesh) :
        _mesh(mesh),
        _totalVertexIndex(0)
      {
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
        FStaticMeshSourceModel& srcModel = _mesh->GetSourceModel(0);
#else
        FStaticMeshSourceModel& srcModel = _mesh->SourceModels[0];
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 22
        // Unreal engine is slowly deprecating raw mesh and replacing it with mesh description for mesh data
        srcModel.LoadRawMesh(_rawMesh);
#else
        srcModel.RawMeshBulkData->LoadRawMesh(_rawMesh);
#endif
        _rawMesh.Empty();

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION > 13
        // Store materials so we don't trample over existing materials during mesh update
        _staticMatStorage = _mesh->StaticMaterials;
#endif
        _mesh->StaticMaterials.Empty();
      }

      StaticMeshImporter::~StaticMeshImporter()
      {
        delete _mesh;
        _mesh = nullptr;
      }

      void StaticMeshImporter::initialize(Package* package, const FName& meshName, const USceneProtocolOptions* options)
      {
        FName displayName = meshName;
        if (StaticFindObject(UStaticMesh::StaticClass(), ANY_PACKAGE, *meshName.ToString()) != nullptr)
        {
          displayName = MakeUniqueObjectName(ANY_PACKAGE, UStaticMesh::StaticClass(), meshName);
        }

        _mesh = NewObject<UStaticMesh>(package->package(), displayName, RF_Standalone | RF_Public);
        int lodIndex = 0;

        // Add one LOD
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 22
        _mesh->AddSourceModel();
#else
        new(_mesh->SourceModels) FStaticMeshSourceModel();
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
        if (_mesh->GetSourceModels().Num() < lodIndex + 1)
        {
          lodIndex = _mesh->GetSourceModels().Num() - 1;
        }

		FStaticMeshSourceModel& srcModel = _mesh->GetSourceModel(lodIndex);
#else
        if (_mesh->SourceModels.Num () < lodIndex + 1)
        {
          lodIndex = _mesh->SourceModels.Num () - 1;
        }

		FStaticMeshSourceModel& srcModel = _mesh->SourceModels[lodIndex];
#endif

        
        // Copy build settings.
        // Enabled automatic adjacency buffer generation for meshes due to Megascan materials not working without it
        srcModel.BuildSettings.bBuildAdjacencyBuffer = true;
        srcModel.BuildSettings.bBuildReversedIndexBuffer = false;
        srcModel.BuildSettings.bGenerateDistanceFieldAsIfTwoSided = false;
        srcModel.BuildSettings.bGenerateLightmapUVs = true;
        srcModel.BuildSettings.bRecomputeNormals = false;
        srcModel.BuildSettings.bRecomputeTangents = false;
        srcModel.BuildSettings.bRemoveDegenerates = false;
        //srcModel.BuildSettings.BuildScale3D = false;
        srcModel.BuildSettings.bUseFullPrecisionUVs = false;
        srcModel.BuildSettings.bUseMikkTSpace = true;
        //srcModel.BuildSettings.DistanceFieldReplacementMesh = buildSettings.DistanceFieldReplacementMesh;
        //srcModel.BuildSettings.DistanceFieldResolutionScale = buildSettings.DistanceFieldResolutionScale;
        //srcModel.BuildSettings.DstLightmapIndex = buildSettings.DstLightmapIndex;
        //srcModel.BuildSettings.MinLightmapResolution = buildSettings.MinLightmapResolution;
        //srcModel.BuildSettings.SrcLightmapIndex = buildSettings.SrcLightmapIndex;

        // make sure it has a new lighting guid
        _mesh->LightingGuid = FGuid::NewGuid();

        // Set it to use textured lightmaps. Note that Build Lighting will do the error-checking (texcoordindex exists for all LODs, etc).
        _mesh->LightMapResolution = 64;
        _mesh->LightMapCoordinateIndex = 1;

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 22
		// Unreal engine is slowly deprecating raw mesh and replacing it with mesh description for mesh data
        srcModel.LoadRawMesh(_rawMesh);
#else
		srcModel.RawMeshBulkData->LoadRawMesh (_rawMesh);
#endif
      }

      bool isInputDataMalformed(const STP::Array::Handle& indices,
        const std::array<STP::Array::Handle, MAX_MESH_TEXTURE_COORDS>& uvBuffer,
        const STP::Array::Handle& vertexBuffer,
        const STP::Array::Handle& RGBABuffer)
      {
        const auto indexCount = STP::Array::getArraySize(indices);
        if ((indexCount == 0) || ((indexCount % 3) != 0)) {
          return true; //malformed
        }

        for (int i = 0; i < uvBuffer.size(); i++)
        {
          auto uvCount = STP::Array::getArraySize(uvBuffer[i]); // assume 2D UVs
          if (uvCount > 0) { // further tests
            if (uvCount != indexCount * 2) {
              return true; //malformed
            }
            if ((uvCount % 2) != 0) {
              return true; //malformed
            }
          }
        }

        const auto vertexCount = STP::Array::getArraySize(vertexBuffer);
        if ((vertexCount == 0) || ((vertexCount % 3) != 0)) {
          return true; // malformed
        }

        const auto RGBACount = STP::Array::getArraySize(RGBABuffer);
        if (RGBACount != 0)
          if ((RGBACount % 4) != 0)
            return true; //malformed

        return false;
      }

      bool StaticMeshImporter::appendMesh(const SceneTransmissionProtocol::Client::Response *response, const std::vector<UMaterialInterface*> &materials)
      {
        STP::Readers::MeshReader meshReader(response->getContentReader());

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20
		// UE 4.20 implements a new type of mesh descriptions that need to be cleared before changing
		// and static mesh rebuilding
		// UE 4.22 doesn't require mesh description clear to update the mesh render
		// Unreal engine is slowly deprecating raw mesh and replacing it with mesh description for mesh data
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 22
		for (int i = 0; i < _mesh->SourceModels.Num(); i++)
            _mesh->ClearOriginalMeshDescription(i);
#endif

#endif

        // Get vertex buffer
        STP::Array::Handle vertexBuffer = meshReader.getVertices();

        // Get normals
        STP::Array::Handle normalBuffer = STP::Array::kInvalidHandle;
        if (meshReader.hasVertexNormals()) {
          normalBuffer = meshReader.getVertexNormals();
        }

        // Get UVs and limit to the maximum of mesh texture coords in UE
        std::array<STP::Array::Handle, MAX_MESH_TEXTURE_COORDS> uvBuffer;
        for (auto& a : uvBuffer) {
          a.ptr = 0;
          a.size = 0;
        }
        std::array<STP::Array::Handle,MAX_MESH_TEXTURE_COORDS> uvIndices;
        for (auto& a : uvIndices) {
          a.ptr = 0;
          a.size = 0;
        }

        int uvSets = meshReader.getUVSetCount();
        auto uvCount = 0;

        uvSets = uvSets > MAX_MESH_TEXTURE_COORDS ? MAX_MESH_TEXTURE_COORDS : uvSets; // Limit to MAX_MESH_TEXTURE_COORDS UV sets
        for (int i = 0; i < uvSets && i < MAX_MESH_TEXTURE_COORDS; i++)
        {
          uvBuffer[i] = meshReader.getUVSetUVs(i);
          uvIndices[i] = meshReader.getUVSetIndices(i);
          
          size_t uvBufSize = STP::Array::getArraySize(uvBuffer[i]);
          if (uvBufSize / 2 > uvCount)
            uvCount = uvBufSize / 2; // assume 2D UVs
        }

        // Get vertex indices
        STP::Array::Handle indices = meshReader.getIndices();
        STP::Array::Handle tangents = STP::Array::kInvalidHandle;
        STP::Array::Handle bitangents = STP::Array::kInvalidHandle;

        // Get color sets
        int colorSets = meshReader.getRGBASetCount();
        STP::Array::Handle vertexRGBA = STP::Array::kInvalidHandle;
        STP::Array::Handle RGBAIndices = STP::Array::kInvalidHandle;

        if (colorSets > 0) {
          vertexRGBA = meshReader.getRGBASetColors(0);
          RGBAIndices = meshReader.getRGBASetIndices(0);
        }
            // removing this temporarily until it can be properly fixed - returned false positives.
//         if (isInputDataMalformed(indices, uvBuffer, vertexBuffer, vertexRGBA))
//         {
//           return false;
//         }

        const auto indexCount = STP::Array::getArraySize(indices);
        const auto faceCount = indexCount / 3; // since we can safely assume triangles
        const auto vertexCount = STP::Array::getArraySize(vertexBuffer) / 3;
        const auto normalCount = STP::Array::getArraySize(normalBuffer) / 3;

        //UE_LOG(LogTemp, Warning, TEXT("vertexCount: %d indexCount : %d numFaces : %d normalCount : %d uvCount : %d"),
        //    vertexCount, indexCount, faceCount, normalCount, uvCount);

        // specify FaceMaterialIndices and FaceSmoothingMasks as per-face
        // Initialize
        for (auto i = 0; i < faceCount; ++i) {
          _rawMesh.FaceMaterialIndices.Add(_numFacesets);
          _rawMesh.FaceSmoothingMasks.Add(0);
        }

        const auto numFacesets = meshReader.getFacesetCount();
        if (0 != numFacesets) {
          // assign a different material index to each faceset
          // this creates mesh Sections, each of which has a different material
          // see further below for material assignments
          auto totalFaceCount = 0;
          for (auto fs = 0; fs < numFacesets; ++fs) {
            STP::Array::Handle faceIndices = meshReader.getFacesetFaces(fs);

            const auto materialIndex = _numFacesets + fs;
            const auto facesetFaceCount = STP::Array::getArraySize(faceIndices);
            for (auto i = 0; i < facesetFaceCount; ++i) {
              int32_t faceIndex = STP::Array::getIntegerArrayItem(faceIndices, i, nullptr);

              assert(faceIndex < faceCount);

              _rawMesh.FaceMaterialIndices[faceIndex] = materialIndex;
            }
            totalFaceCount += facesetFaceCount;
          }
          assert(totalFaceCount == faceCount);
        }

        // specify positions as per-vertex
        for (int32_t i = 0; i < vertexCount; i++) {
          const float x = STP::Array::getFloatArrayItem(vertexBuffer, i * 3, nullptr);
          const float y = STP::Array::getFloatArrayItem(vertexBuffer, i * 3 + 1, nullptr);
          const float z = STP::Array::getFloatArrayItem(vertexBuffer, i * 3 + 2, nullptr);
          _rawMesh.VertexPositions.Add(FVector(x, y, z));
        }

        auto attributesPerVertex = true;
        if ((STP::Array::getArraySize(normalBuffer) > 0 && normalCount > vertexCount) ||
          uvCount > vertexCount) {
          // vertex attributes per index!
          attributesPerVertex = false;
        }

        // A wedge is a 'property for each corner of each face'
        // 'wedge' indices map a wedge to the vertex (position) it corresponds to
        if (attributesPerVertex) {
          for (int32_t i = 0; i < indexCount; i++) {
            const int32_t vertIdx = STP::Array::getIntegerArrayItem(indices, i, nullptr);
            _rawMesh.WedgeIndices.Add(vertIdx + _totalVertexIndex);

            if (STP::Array::getArraySize(tangents) && STP::Array::getArraySize(bitangents)) {
              float tx = STP::Array::getFloatArrayItem(tangents, vertIdx * 3, nullptr);
              float ty = STP::Array::getFloatArrayItem(tangents, vertIdx * 3 + 1, nullptr);
              float tz = STP::Array::getFloatArrayItem(tangents, vertIdx * 3 + 2, nullptr);
              _rawMesh.WedgeTangentX.Add(FVector(tx, ty, tz));

              float bx = STP::Array::getFloatArrayItem(bitangents, vertIdx * 3, nullptr);
              float by = STP::Array::getFloatArrayItem(bitangents, vertIdx * 3 + 1, nullptr);
              float bz = STP::Array::getFloatArrayItem(bitangents, vertIdx * 3 + 2, nullptr);
              _rawMesh.WedgeTangentY.Add(FVector(bx, by, bz));
            }

            if (normalCount > 0) {
              float nx = STP::Array::getFloatArrayItem(normalBuffer, vertIdx * 3, nullptr);
              float ny = STP::Array::getFloatArrayItem(normalBuffer, vertIdx * 3 + 1, nullptr);
              float nz = STP::Array::getFloatArrayItem(normalBuffer, vertIdx * 3 + 2, nullptr);
              _rawMesh.WedgeTangentZ.Add(FVector(nx, ny, nz));
            }

            if (STP::Array::getArraySize(vertexRGBA) > 0) {
              float r = STP::Array::getFloatArrayItem(vertexRGBA, vertIdx * 4, nullptr);
              float g = STP::Array::getFloatArrayItem(vertexRGBA, vertIdx * 4 + 1, nullptr);
              float b = STP::Array::getFloatArrayItem(vertexRGBA, vertIdx * 4 + 2, nullptr);
              float a = STP::Array::getFloatArrayItem(vertexRGBA, vertIdx * 4 + 3, nullptr);
              _rawMesh.WedgeColors.Add(FColor(r, g, b, a));
            }
            else {
              _rawMesh.WedgeColors.AddZeroed(1);
            }

            // Add maximally MAX_MESH_TEXTURE_COORDS UV sets
            // minimum requirement for a mesh is to have one set of UVs, one per wedge index
            if (uvSets > 0)
            {
              for (int UVSet = 0; UVSet < uvSets; UVSet++)
              {
                if (STP::Array::getArraySize(uvBuffer[UVSet]) / 2 >= STP::Array::getArraySize(vertexBuffer) / 3) {
                  // only add uv's if theres one per vertex, otherwise this mesh will be invalid.
                  auto index = STP::Array::getIntegerArrayItem(uvIndices[UVSet], vertIdx, nullptr);
                  float u = STP::Array::getFloatArrayItem(uvBuffer[UVSet], index * 2, nullptr);
                  float v = STP::Array::getFloatArrayItem(uvBuffer[UVSet], index * 2 + 1, nullptr);
                  _rawMesh.WedgeTexCoords[UVSet].Add(FVector2D(u, v));
                }
                else {
                  _rawMesh.WedgeTexCoords[UVSet].AddZeroed(1);
                }
              }
            }
            else
            {
              _rawMesh.WedgeTexCoords[0].AddZeroed(1);
            }
          }
        }
        else {
          // other vertex attributes must have the same length
          if (normalCount > 0)
          {
            assert(normalCount == indexCount);
          }
          if (uvCount > 0)
          {
            assert(uvCount == indexCount);
          }
          if (STP::Array::getArraySize(tangents) > 0 && STP::Array::getArraySize(bitangents) > 0) {
            assert(STP::Array::getArraySize(tangents) / 3 == indexCount);
            assert(STP::Array::getArraySize(bitangents) / 3 == indexCount);
          }
          for (auto i = 0; i < indexCount; ++i)
          {
            // which vertex does this wedge correspond to?
            _rawMesh.WedgeIndices.Add(STP::Array::getIntegerArrayItem(indices, i, nullptr));

            // TODO: tangents are not yet supported via the server
            // they are calculated by UE when missing anyway

            // minimum requirement for a mesh is to have one set of UVs, one per wedge index
            if (uvSets > 0)
            {
              for (int UVSet = 0; UVSet < uvSets; UVSet++)
              {
                if (uvCount > 0) {
                  auto index = STP::Array::getIntegerArrayItem(uvIndices[UVSet], i, nullptr);
                  float u = STP::Array::getFloatArrayItem(uvBuffer[UVSet], index * 2, nullptr);
                  float v = STP::Array::getFloatArrayItem(uvBuffer[UVSet], index * 2 + 1, nullptr);
                  _rawMesh.WedgeTexCoords[UVSet].Add(FVector2D(u, v));
                }
                else {
                  _rawMesh.WedgeTexCoords[UVSet].AddZeroed(1);
                }
              }
            }
            else
            {
              _rawMesh.WedgeTexCoords[0].AddZeroed(1);
            }

            // supply per-index normals
            if (normalCount > 0) {
              float nx = STP::Array::getFloatArrayItem(normalBuffer, i * 3, nullptr);
              float ny = STP::Array::getFloatArrayItem(normalBuffer, i * 3 + 1, nullptr);
              float nz = STP::Array::getFloatArrayItem(normalBuffer, i * 3 + 2, nullptr);
              _rawMesh.WedgeTangentZ.Add(FVector(nx, ny, nz));
            }

            // supply per-index vertex colors
            if (STP::Array::getArraySize(vertexRGBA)) {
              float r = STP::Array::getFloatArrayItem(vertexRGBA, i * 4, nullptr);
              float g = STP::Array::getFloatArrayItem(vertexRGBA, i * 4 + 1, nullptr);
              float b = STP::Array::getFloatArrayItem(vertexRGBA, i * 4 + 2, nullptr);
              float a = STP::Array::getFloatArrayItem(vertexRGBA, i * 4 + 3, nullptr);
              _rawMesh.WedgeColors.Add(FColor(r, g, b, a));
            }
            else {
              _rawMesh.WedgeColors.AddZeroed(1);
            }
          }
        }
        _totalVertexIndex += indexCount;

        // Check mesh validity
        bool valid = _rawMesh.IsValid();
        bool fixable = _rawMesh.IsValidOrFixable();
        UE_LOG(LogTemp, Warning, TEXT("Mesh valid : %d valid or fixable : %d"), valid, fixable);
        //UE_LOG(LogTemp, Warning, TEXT("HasValidRenderData (2) : %d"), StaticMesh->HasValidRenderData() );

        if (valid)
        {
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
          FStaticMeshSourceModel& srcModel = _mesh->GetSourceModel(0);
#else
          FStaticMeshSourceModel& srcModel = _mesh->SourceModels[0];
#endif

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 22
		  // Unreal engine is slowly deprecating raw mesh and replacing it with mesh description for mesh data
          srcModel.SaveRawMesh(_rawMesh);
#else
          srcModel.RawMeshBulkData->SaveRawMesh(_rawMesh);
#endif

          // Build a new map of sections with the unique material set
          FMeshSectionInfoMap newMap;
          auto numMaterials = numFacesets;
          if (0 == numMaterials) {
            numMaterials++; // because there is always at least one
          }
          assert(numMaterials == materials.size());
          for (auto fs = 0; fs < numMaterials; ++fs) {
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
            FMeshSectionInfo info = _mesh->GetSectionInfoMap().Get(0, fs);
#else
            FMeshSectionInfo info = _mesh->SectionInfoMap.Get (0, fs);
#endif
            auto material = materials[fs];
            if (nullptr == material) 
            {
              UE_LOG(LogTemp, Warning, TEXT("Could not obtain material for mesh %s, faceset %d.  Using default."), *SPNB::Unreal::stringDataToFString(response->getEntityToken()), fs);

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION > 13
              if (fs < _staticMatStorage.Num())
              {
                material = _staticMatStorage[fs].MaterialInterface;
              }
              else
              {
                material = UMaterial::GetDefaultMaterial(MD_Surface);
              }
#else
              material = UMaterial::GetDefaultMaterial(MD_Surface);
#endif
            }

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION > 13
            int32 index = _mesh->StaticMaterials.Add(material);
#else
            int32 index = _mesh->Materials.Add(material);
#endif
            info.MaterialIndex = index;
            newMap.Set(0, fs, info);
          }

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
          _mesh->GetSectionInfoMap().Clear();
          _mesh->GetSectionInfoMap().CopyFrom(newMap);
#else
          _mesh->SectionInfoMap.Clear ();
          _mesh->SectionInfoMap.CopyFrom (newMap);
#endif

          _numFacesets += numMaterials;
        }
        else if (fixable)
        {
          UE_LOG(LogTemp, Warning, TEXT("NOT Saving mesh. It is fixable though..."));
          return false;
        }
        else
        {
          UE_LOG(LogTemp, Error, TEXT("NOT Saving mesh. Borked mesh."));
          return false;
        }
        return true;
      }

      std::unique_ptr<UStaticMesh> StaticMeshImporter::finalizeAndReturn()
      {
        TArray<FText> buildErrors;
        _mesh->Build(true, &buildErrors);
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
        check(_mesh->GetSourceModels().IsValidIndex(0));
#else
        check(_mesh->SourceModels.IsValidIndex(0));
#endif
        FAssetRegistryModule::AssetCreated(_mesh);
        _mesh->MarkPackageDirty();

        std::unique_ptr<UStaticMesh> upMesh(_mesh);
        _mesh = nullptr;
        return upMesh;
      }

      void StaticMeshImporter::finalize()
      {
        TArray<FText> buildErrors;
        _mesh->Build(true, &buildErrors);
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
        check (_mesh->GetSourceModels().IsValidIndex(0));
#else
        check (_mesh->SourceModels.IsValidIndex (0));
#endif
        FAssetRegistryModule::AssetCreated(_mesh);
        _mesh->MarkPackageDirty();
        _mesh = nullptr;
      }
    } // namespace Unreal
  } // namespace NetworkBridge
} // namespace SceneProtocol