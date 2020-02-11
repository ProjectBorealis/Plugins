// --------------------------------------------------------------------------
// An importer for SGP mesh types into UnrealEngine.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "RawMesh.h"
#include "StaticMeshResources.h"
#include "SceneProtocolOptions.h"

#include "Request.h"

#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"

#include <vector>

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {
      class StaticMeshExporter {

      public:
        void getSelectedMeshActors(std::vector<AStaticMeshActor*>& selectedMeshActors);
        void getAllMeshActors(std::vector<AStaticMeshActor*>& meshActors);

        /** Create mesh item token
        *
        * Method for creation of mesh response tokens.
        * We are not saving generated mesh item tokens to prevent edit of non STP items.
        *
        * @param meshActor AStaticMeshActor that we are trying to export a mesh from.
        * @param meshToken Resulting mesh token.
        */
        void createMeshItemToken(AStaticMeshActor* meshActor, std::string& meshToken);

        /** Create mesh instance item token
        *
        * Method for creation of mesh instance response tokens.
        * This method saves the instance mesh token to the custom user data for level saving on the actor
        * and adds the token to the unique name mapping so we can access it and update it.
        *
        * @param meshActor AStaticMeshActor that we are trying to export the mesh instance from.
        * @param meshToken Resulting mesh instance token.
        */
        void createMeshInstanceItemToken(AStaticMeshActor* meshActor, std::string& meshInstToken);

        /** Create material item token
        *
        * Method for creation of material response tokens.
        *
        * @param material UMaterial that we are trying to export a material item from.
        * @param materialToken Resulting material token.
        */
        void createMaterialItemToken(UMaterial* material, std::string& materialToken);

        /** Create material instance item token
        *
        * Method for creation of material instance response tokens.
        *
        * @param material UMaterialInstanceConstant that we are trying to export a material item from.
        * @param materialToken Resulting material token.
        */
        void createMaterialInstItemToken(UMaterialInstanceConstant* materialInst, std::string& materialToken);

        SceneTransmissionProtocol::Client::Request* generateMeshRequest(AStaticMeshActor* meshActor, TArray<UMaterial*>& materialArray);
        SceneTransmissionProtocol::Client::Request* generateMeshInstanceRequest(AStaticMeshActor* meshActor);
        SceneTransmissionProtocol::Client::Request* generateMaterialRequest(UMaterial* material);

      private:
        void packageStandardData (AStaticMeshActor* meshActor, SceneTransmissionProtocol::Client::Request* request);
      };
    }
  }
}

