// --------------------------------------------------------------------------
// Implementation of the SGP mesh importer.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "StaticMeshExport.h"
#include "Package.h"

#include "SceneProtocol.h"
#include "SceneProtocolStaticMeshActor.h"

#include "NodeHandler.h"
#include "MetadataUtils.h"
#include "Handlers/HandlerUtils.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "SceneProtocolUserData.h"

#include "CoreWriters/MeshWriter.h"
#include "CoreWriters/MeshInstanceWriter.h"
#include "CoreWriters/MaterialDefinitionWriter.h"
#include "ArrayData.h"
#include "Names.h"

#include "BSPOps.h"
#include "PhysicsEngine/BodySetup.h"
#include "EditorSupportDelegates.h"
#include "AssetRegistryModule.h"

#include "Editor.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "EngineUtils.h"
#include "Engine/Selection.h"
#include "UObject/UObjectGlobals.h"

#include <string>
#include <array>

namespace SPNB = SceneProtocol::NetworkBridge;
namespace STP = SceneTransmissionProtocol;

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {

      enum
      {
        DataStrideIndex = 3,
        DataStrideVertex = 3,
        DataStrideNormal = 3,
        DataStrideUV = 2,
        DataStrideColor = 4,
        DataStrideMatIndices = 1,
        DataStrideMatrix = 16,
        // Number of vertices per polygen is always 3
        nPolyVerts = 3
      };

      void appendUniqueSuffix(std::string& token)
      {
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();
        std::string tokenStore;
        tokenStore = token;
        int suffixNum = 1;
        while (packageMapper.tokenExists(tokenStore))
        {
          tokenStore = token;
          tokenStore.append("_");
          tokenStore.append(std::to_string(suffixNum));
          suffixNum++;
        }

        token = tokenStore;
      }

      void StaticMeshExporter::getSelectedMeshActors(std::vector<AStaticMeshActor*>& selectedMeshActors)
      {
        USelection* selection = GEditor->GetSelectedActors();
        for (int32 i = 0; i < selection->Num(); i++)
        {
          UObject* obj = selection->GetSelectedObject(i);

          if (obj == nullptr)
          {
            continue;
          }

          if (obj->GetClass() == AStaticMeshActor::StaticClass() || obj->GetClass() == ASceneProtocolStaticMeshActor::StaticClass())
          {
            AStaticMeshActor *mesh = dynamic_cast<AStaticMeshActor*>(obj);
            if (mesh != nullptr)
            {
              selectedMeshActors.push_back(mesh);
            }
          }
        }
      }

      void StaticMeshExporter::getAllMeshActors(std::vector<AStaticMeshActor*>& meshActors)
      {
        ULevel* desiredLevel = GWorld->GetCurrentLevel();

        for (TActorIterator<AStaticMeshActor> ActorItr(desiredLevel->OwningWorld); ActorItr; ++ActorItr)
        {
          AStaticMeshActor *Mesh = *ActorItr;
          meshActors.push_back(Mesh);
        }
      }

      void StaticMeshExporter::createMeshItemToken(AStaticMeshActor* meshActor, std::string& meshToken)
      {
        UStaticMesh* staticMeshPtr = meshActor->GetStaticMeshComponent ()->GetStaticMesh ();
        if (!staticMeshPtr)
            return;

        USceneProtocolUserData* userData = Cast<USceneProtocolUserData>(staticMeshPtr->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));

        // If we find protocol user data we return the stored token
        if (userData && !userData->itemToken.IsNone())
        {
          meshToken = TCHAR_TO_UTF8(*userData->itemToken.ToString());
          return;
        }

        // If we don't find stored item token generate a new one and store it
        ULevel* desiredLevel = GWorld->GetCurrentLevel();

        meshToken = "/";
        meshToken += TCHAR_TO_UTF8(*desiredLevel->GetName());
        meshToken += "/meshes/";

        
        if (staticMeshPtr)
        {
          meshToken += TCHAR_TO_UTF8(*staticMeshPtr->GetName());
        }
        else
        {
          meshToken += TCHAR_TO_UTF8(*meshActor->GetName());
          meshToken += "_mesh";
        }

        appendUniqueSuffix(meshToken);
      }

      void StaticMeshExporter::createMeshInstanceItemToken(AStaticMeshActor* meshActor, std::string& meshInstToken)
      {
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();
        UStaticMeshComponent* meshComponent = meshActor->GetStaticMeshComponent();
        USceneProtocolUserData* userData = Cast<USceneProtocolUserData>(meshComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));

        // If we find protocol user data we return the stored token
        if (userData && !userData->itemToken.IsNone())
        {
          meshInstToken = TCHAR_TO_UTF8(*userData->itemToken.ToString());
          return;
        }

        // If we don't find stored item token generate a new one and store it
        ULevel* desiredLevel = GWorld->GetCurrentLevel();

        meshInstToken = "/";
        meshInstToken += TCHAR_TO_UTF8(*desiredLevel->GetName());
        meshInstToken += "/instances/";
        meshInstToken += TCHAR_TO_UTF8(*meshActor->GetName());
        meshInstToken += "_inst";

        appendUniqueSuffix(meshInstToken);

        if (!userData)
        {
          userData = NewObject< USceneProtocolUserData >(meshComponent, NAME_None, RF_Public);
          meshComponent->AddAssetUserData(userData);
        }

        userData->itemToken = FName(*FString(meshInstToken.c_str()));

        FString packageName = packageMapper.generatePackageName(FString(MESH_INST_PACKAGE_ROOT), meshInstToken);

        // Add the object and the static mesh object
        std::vector<UObject*> objects;
        objects.push_back(meshActor);

        packageMapper.registerUnownedPackage(meshInstToken, FName(*packageName), meshActor->GetOutermost(), objects);
      }

      void StaticMeshExporter::createMaterialItemToken(UMaterial* material, std::string& materialToken)
      {
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();
        USceneProtocolUserData* userData = Cast<USceneProtocolUserData>(material->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));

        // If we find protocol user data we return the stored token
        if (userData && !userData->itemToken.IsNone())
        {
          materialToken = TCHAR_TO_UTF8(*userData->itemToken.ToString());
          return;
        }

        // If we don't find stored item token generate a new one and store it
        ULevel* desiredLevel = GWorld->GetCurrentLevel();

        materialToken = "/";
        materialToken += TCHAR_TO_UTF8(*desiredLevel->GetName());
        materialToken += "/materials/";
        materialToken += TCHAR_TO_UTF8(*material->GetName());

        appendUniqueSuffix(materialToken);

        if (!userData)
        {
          userData = NewObject< USceneProtocolUserData >(material, NAME_None, RF_Public);
          material->AddAssetUserData(userData);
        }

        userData->itemToken = FName(*FString(materialToken.c_str()));

        FString packageName = packageMapper.generatePackageName(FString(MATERIAL_DEF_PACKAGE_ROOT), materialToken);

        // Add the object and the static mesh object
        std::vector<UObject*> objects;
        objects.push_back(material);

        packageMapper.registerUnownedPackage(materialToken, FName(*packageName), material->GetOutermost(), objects);
      }

      void StaticMeshExporter::createMaterialInstItemToken(UMaterialInstanceConstant* materialInst, std::string& materialInstToken)
      {
        PackageMapper& packageMapper = SPNB::Unreal::PackageMapper::instance();
        USceneProtocolUserData* userData = Cast<USceneProtocolUserData>(materialInst->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));

        // If we find protocol user data we return the stored token
        if (userData && !userData->itemToken.IsNone())
        {
          materialInstToken = TCHAR_TO_UTF8(*userData->itemToken.ToString());
          return;
        }

        // If we don't find stored item token generate a new one and store it
        ULevel* desiredLevel = GWorld->GetCurrentLevel();

        materialInstToken = "/";
        materialInstToken += TCHAR_TO_UTF8(*desiredLevel->GetName());
        materialInstToken += "/materialsInst/";
        materialInstToken += TCHAR_TO_UTF8(*materialInst->GetName());

        appendUniqueSuffix(materialInstToken);

        if (!userData)
        {
          userData = NewObject< USceneProtocolUserData >(materialInst, NAME_None, RF_Public);
          materialInst->AddAssetUserData(userData);
        }

        userData->itemToken = FName(*FString(materialInstToken.c_str()));

        FString packageName = packageMapper.generatePackageName(FString(MATERIAL_INST_PACKAGE_ROOT), materialInstToken);

        // Add the object and the static mesh object
        std::vector<UObject*> objects;
        objects.push_back(materialInst);

        packageMapper.registerUnownedPackage(materialInstToken, FName(*packageName), materialInst->GetOutermost(), objects);
      }

      STP::Client::Request* StaticMeshExporter::generateMeshRequest(AStaticMeshActor* meshActor, TArray<UMaterial*>& materialArray)
      {
        UStaticMesh* staticMeshPtr = meshActor->GetStaticMeshComponent()->GetStaticMesh();
        if (staticMeshPtr == nullptr)
          return nullptr;

        const FStaticMeshLODResources& mesh = staticMeshPtr->GetLODForExport(0);
        // Verify the integrity of the static mesh.
        if (mesh.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() == 0 || mesh.Sections.Num() == 0)
          return nullptr;

        // Remaps an Unreal vert to final reduced vertex list
        TArray<int32> vertRemap;
        TArray<int32> uniqueVerts;

        // TODO: control Weld and UnWeld
        // Do not weld verts
        vertRemap.SetNum(mesh.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
        for (int32 i = 0; i < vertRemap.Num(); i++)
        {
          vertRemap[i] = i;
        }
        uniqueVerts = vertRemap;

        const uint32 vertexCount = vertRemap.Num();
        const uint32 sectionsCount = mesh.Sections.Num();

        std::string meshToken;
        createMeshItemToken(meshActor, meshToken);

        // Create a mesh CreateOP request
        STP::Client::Request* request = new STP::Client::Request(STP::Operation::CreateOp);

        request->startHeader();
        request->setEntityToken(meshToken.c_str());
        request->setIdentifier("");
        request->endHeader();

        request->beginContent();
        request->setDisplayName(TCHAR_TO_UTF8(*meshActor->GetName()));

        // Children, parent, transformation
        packageStandardData(meshActor, request);

        request->setType(STP::Names::ResponseTypeMesh);

        STP::Writers::MeshWriter::MeshConfig config;

        // Sections are facesets
        config.facesetCount = sectionsCount;

        std::vector<int32> indices;
        std::vector<std::vector<int32>> materialIndices;
        indices.reserve(DataStrideIndex * sectionsCount);
        materialIndices.reserve(config.facesetCount);
        for (uint32 sectionIndex = 0; sectionIndex < sectionsCount; ++sectionIndex)
        {
          materialIndices.push_back(std::vector<int32>());
          FIndexArrayView RawIndices = mesh.IndexBuffer.GetArrayView();
          const FStaticMeshSection& Polygons = mesh.Sections[sectionIndex];
          const uint32 TriangleCount = Polygons.NumTriangles;
          uint32 trianglePointInd = 0;
          for (uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
          {
            for (uint32 PointIndex = 0; PointIndex < 3; PointIndex++)
            {
              uint32 UnrealVertIndex = RawIndices[Polygons.FirstIndex + (trianglePointInd + PointIndex)];
              indices.push_back(UnrealVertIndex);
            }
            materialIndices[sectionIndex].push_back(Polygons.MaterialIndex);
            trianglePointInd += 3;
          }
        }

        std::vector<float> vertices;
        vertices.reserve(DataStrideVertex * vertexCount);
        for (uint32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
        {
          FVector pos = mesh.VertexBuffers.PositionVertexBuffer.VertexPosition(vertexIndex);

          vertices.push_back(pos.X);
          vertices.push_back(pos.Y);
          vertices.push_back(pos.Z);
        }

        std::vector<float> normals;
        normals.reserve (DataStrideNormal * vertexCount);
        config.hasNormals = true;
        for (uint32 normalIndex = 0; normalIndex < vertexCount; normalIndex++)
        {
          FVector Normal = mesh.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(normalIndex);

          normals.push_back(Normal.X);
          normals.push_back(Normal.Y);
          normals.push_back(Normal.Z);
        }

        std::vector<std::vector<float>> uvSets;
        config.uvsetCount = mesh.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
        uvSets.reserve(config.uvsetCount);
        for (int32 UVSetIndex = 0; UVSetIndex < config.uvsetCount; UVSetIndex++)
        {
          uvSets.push_back(std::vector<float>());
          uvSets[UVSetIndex].reserve(DataStrideUV * vertexCount);
          for (uint32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
          {
            FVector2D UV = mesh.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(vertexIndex, UVSetIndex);

            uvSets[UVSetIndex].push_back(UV.X);
            uvSets[UVSetIndex].push_back(UV.Y);
          }
        }

        std::vector<float> colors;
        colors.reserve (DataStrideColor * vertexCount);
        auto colorVertexCount = mesh.VertexBuffers.ColorVertexBuffer.GetNumVertices();
        if (colorVertexCount > 0)
        {
          config.RGBAsetCount = 1;
        }

        for (uint32 vertexIndex = 0; vertexIndex < colorVertexCount; vertexIndex++)
        {
          const FColor& Color = mesh.VertexBuffers.ColorVertexBuffer.VertexColor(vertexIndex);
          colors.push_back(Color.R / 255.0);
          colors.push_back(Color.G / 255.0);
          colors.push_back(Color.B / 255.0);
          colors.push_back(Color.A / 255.0);
        }

        // TODO: tangents and binormals
        /*
        //std::vector<float> tangents;
        //tangents.reserve(DataStrideTangent * vertexCount);
        for (uint32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
        {
          FVector Tangent = mesh.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);

          tangents.push_back(Tangent.X);
          tangents.push_back(Tangent.Y);
          tangents.push_back(Tangent.Z);

          FVector BiNormal = mesh.VertexBuffers.StaticMeshVertexBuffer.VertexTangentY(VertexIndex);

          tangents.push_back(BiNormal.X);
          tangents.push_back(BiNormal.Y);
          tangents.push_back(BiNormal.Z);
        }
        */

        // Build the request data payload with mesh writer
        STP::Writers::MeshWriter meshWriter(request->getContentWriter(), config);

        request->beginTypeData(meshWriter.calculateFieldCount());

        meshWriter.begin();
        meshWriter.setVertices(vertices.data(), vertices.size());
        meshWriter.setIndices(indices.data(), indices.size());
        meshWriter.setVertexNormals(normals.data(), normals.size());

        for (int i = 0; i < config.uvsetCount; i++)
        {
          meshWriter.addUVs(uvSets[i].data(), uvSets[i].size(), indices.data(), indices.size());
        }

        for (int i = 0; i < config.facesetCount; i++)
        {
          UMaterialInterface* matInterface = staticMeshPtr->GetMaterial(i);
          if (matInterface == nullptr)
          {
            continue;
          }

          UMaterial* material = matInterface->GetMaterial();

          if (!material)
          {
            continue;
          }

          materialArray.AddUnique(material);
          
          std::string materialToken;

          if (matInterface->IsA(UMaterialInstanceConstant::StaticClass()))
          {
            createMaterialInstItemToken(Cast<UMaterialInstanceConstant>(matInterface), materialToken);
            meshWriter.addFaceset(materialIndices[i].data(), materialIndices[i].size(), materialToken.c_str());
          }
          else
          {
            createMaterialItemToken(material, materialToken);
            meshWriter.addFaceset(materialIndices[i].data(), materialIndices[i].size(), materialToken.c_str());
          }
        }

        for (int i = 0; i < config.RGBAsetCount; i++)
        {
          meshWriter.addVertRGBA(colors.data(), colors.size(), indices.data(), indices.size());
        }

        request->endTypeData();

        request->beginDynamicData(0);
        request->endDynamicData();

        request->endContent();

        return request;
      }

      STP::Client::Request* StaticMeshExporter::generateMeshInstanceRequest(AStaticMeshActor* meshActor)
      {
        UStaticMesh* staticMeshPtr = meshActor->GetStaticMeshComponent()->GetStaticMesh();
        if (staticMeshPtr == nullptr)
          return nullptr;

        std::string meshInstToken, srcMeshToken;
        createMeshInstanceItemToken(meshActor, meshInstToken);
        createMeshItemToken(meshActor, srcMeshToken);

        // Create a mesh CreateOP request
        STP::Client::Request* request = new STP::Client::Request(STP::Operation::CreateOp);

        request->startHeader();
        request->setEntityToken(meshInstToken.c_str());
        request->setIdentifier("");
        request->endHeader();

        request->beginContent();
        request->setDisplayName(TCHAR_TO_UTF8(*meshActor->GetName()));

        // Children, parent, transformation
        packageStandardData(meshActor, request);

        request->setType (STP::Names::ResponseTypeMeshInstance);

        // Build the request data payload with mesh writer
        STP::Writers::MeshInstanceWriter meshInstWriter(request->getContentWriter(), srcMeshToken.c_str());

        request->beginTypeData (meshInstWriter.calculateFieldCount ());

        meshInstWriter.begin ();

        request->endTypeData ();

        request->beginDynamicData (0);
        request->endDynamicData ();

        request->endContent ();

        return request;
      }

      STP::Client::Request* StaticMeshExporter::generateMaterialRequest(UMaterial* material)
      {
        // Create a mesh CreateOP request
        STP::Client::Request* request = new STP::Client::Request(STP::Operation::CreateOp);

        std::string matToken, parentToken;
        createMaterialItemToken(material, matToken);
        const char* displayName = matToken.c_str();

        request->startHeader();
        request->setEntityToken(displayName);
        request->setIdentifier("");
        request->endHeader();

        request->beginContent();
        request->setDisplayName(TCHAR_TO_UTF8(*material->GetName()));

        STP::Types::Transform transform;
        request->setStandardContentFields(parentToken.c_str(), nullptr, 0, transform);
        request->setType(STP::Names::ResponseTypeMaterialDefinition);

        // Build the request data payload with mesh writer
        STP::Writers::MaterialDefinitionWriter matWriter(request->getContentWriter(), 1);

        request->beginTypeData(matWriter.calculateFieldCount());

        matWriter.begin();
        matWriter.setModel(STP::Writers::MaterialDefinitionWriter::StandardPBR);
        matWriter.setBlendMode(SceneTransmissionProtocol::Writers::MaterialDefinitionWriter::Opaque);

        FColorMaterialInput* input = &material->BaseColor;
        UMaterialExpression* expression = input->Expression;

        std::vector<float> data = {0.0f, 0.0f, 0.0f, 0.0f};
        UMaterialExpressionVectorParameter* vectParam = Cast<UMaterialExpressionVectorParameter>(expression);
        if (vectParam)
        {
          data[0] = vectParam->DefaultValue.R;
          data[1] = vectParam->DefaultValue.B;
          data[2] = vectParam->DefaultValue.G;
          data[3] = vectParam->DefaultValue.A;
        }

        matWriter.addParameter("baseColor", STP::Names::MaterialDefinitionTargetColor, data.data(), data.size());

        request->endTypeData();

        request->beginDynamicData(0);
        request->endDynamicData();

        request->endContent();

        return request;
      }

      void StaticMeshExporter::packageStandardData(AStaticMeshActor* meshActor, STP::Client::Request* request)
      {
          // Iterate children
        TArray<AActor* > attachedActors;
        std::vector<const char*> children;
        meshActor->GetAttachedActors (attachedActors);

        for (int i = 0; i < attachedActors.Num (); i++)
        {
            if (attachedActors[i]->GetClass () != AStaticMeshActor::StaticClass ())
            {
                continue;
            }

            AStaticMeshActor* childMeshActor = Cast<AStaticMeshActor> (attachedActors[i]);

            if (childMeshActor)
            {
                std::string childToken;
                createMeshItemToken (childMeshActor, childToken);
                children.push_back (childToken.c_str ());
            }
        }

        // Find parent
        AActor* parent = meshActor->GetParentActor ();
        std::string parentToken;
        if (parent)
        {
            if (parent->GetClass () != AStaticMeshActor::StaticClass ())
            {
                AStaticMeshActor* parentMeshActor = Cast<AStaticMeshActor> (parent);

                if (parentMeshActor)
                {
                    createMeshItemToken (parentMeshActor, parentToken);
                }
            }
        }

        // Get transformation data
        FTransform rootXfrm = meshActor->GetActorTransform ();
        FMatrix mat;
        STP::Types::Transform transform;

        mat = rootXfrm.ToMatrixWithScale ();

        for (int row = 0; row < 4; row++)
        {
            for (int col = 0; col < 4; col++)
            {
                transform.elements[row * 4 + col] = mat.M[row][col];
            }
        }

        request->setStandardContentFields(parentToken.c_str(), children.data(), children.size(), transform);
      }
    } // namespace Unreal
  } // namespace NetworkBridge
} // namespace SceneProtocol
