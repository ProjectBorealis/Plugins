// --------------------------------------------------------------------------
// Manager for processing nodes of the scene graph, i.e. SGP responses.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "Response.h"
#include "ArrayData.h"

#include "SceneProtocolOptions.h"

#include <memory>
#include <list>
#include <vector>
#include <string>

class UWorld;

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {

//NodeHandler the abstract class from which to base handlers for any node type
class SCENEPROTOCOL_API NodeHandler {
public:
  NodeHandler() {}
};


class SCENEPROTOCOL_API NodeHandlerManager {

public:

  //The NodeHandlerManager singleton keeps the list of handlers
  static NodeHandlerManager* get();

  // TODO: Servers with local matrix implementation should upgrade parent transformation 
  // storage to update with user scene graph changes in Unreal. Not doing that breaks children
  // transfers if user does any changes to parent transformations.
#ifdef STP_LOCAL_MATRIX_PAYLOAD
  void setParent(const std::string &name, const std::string &parent);
  void setTransform(const std::string &name, const SceneTransmissionProtocol::Array::Handle &transform);
  bool getParent(const std::string &name, std::string &outParent);
  bool getTransform(const std::string &name, FMatrix &outTransform);
#endif

  void addMesh(SceneTransmissionProtocol::Client::Response* response);
  void addMeshInstance(SceneTransmissionProtocol::Client::Response* response);
  void addMaterialDefinition(SceneTransmissionProtocol::Client::Response* response);
  void addMaterial(SceneTransmissionProtocol::Client::Response* response);
  void addLight(SceneTransmissionProtocol::Client::Response* response);
  void addCamera(SceneTransmissionProtocol::Client::Response* response);
  void addTexture(SceneTransmissionProtocol::Client::Response* response);
  void addUpdate(SceneTransmissionProtocol::Client::Response* response);
  void addCustom(SceneTransmissionProtocol::Client::Response* response);
  void poll();

  void setOptions(const USceneProtocolOptions* options);
  const USceneProtocolOptions* getOptions() const;

  //initialized to false, becomes true if the user cancels an import
  bool userCancel = false;

protected:
  NodeHandlerManager();
  ~NodeHandlerManager();

  // due to the use of unique_ptr in member variables
  NodeHandlerManager(const NodeHandlerManager &) = delete;
  NodeHandlerManager(NodeHandlerManager &&) = delete;
  NodeHandlerManager &operator=(const NodeHandlerManager &) = delete;
  NodeHandlerManager &&operator=(NodeHandlerManager &&) = delete;

  bool materialDependenciesMet(const SceneTransmissionProtocol::Client::Response* response);
  bool materialDefinitionDependenciesMet(const SceneTransmissionProtocol::Client::Response* response);
  bool meshDependenciesMet(const SceneTransmissionProtocol::Client::Response* response);
  bool meshInstanceDependenciesMet(const SceneTransmissionProtocol::Client::Response* response);

  static NodeHandlerManager* _theInstance;

#ifdef STP_LOCAL_MATRIX_PAYLOAD
  std::unordered_map<std::string, std::string> _locationParent;
  std::unordered_map<std::string, FMatrix> _locationTransform;
#endif

  // these take ownership of the responses until they are dealt with (and then released)
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingMeshes;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingMeshInstances;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingMaterials;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingMaterialDefs;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingLights;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingCameras;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingTextures;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingUpdates;
  std::list<SceneTransmissionProtocol::Client::Response*> _pendingCustom;

  const USceneProtocolOptions* _options;
};

//SceneProtocol -> UE specific conversion utils:

//given a specific node which has the "kParentNode" property, the transform hierarchy
// is traversed to get the world transform
FTransform getWorldTransform(const SceneTransmissionProtocol::Client::Response *response);

//create a UE matrix from a float buffer consisting of 16 variables
FMatrix constructUEMatrix(const SceneTransmissionProtocol::Array::Handle& buffer);

// TODO: Create a flag for responses if they have payloads of local or world matrices and enable both code paths
#ifdef STP_LOCAL_MATRIX_PAYLOAD
//get the parent transform by walking up the node hierarchy recursively
FMatrix getParentMatrix(const std::string &name);
#endif

FString getShortName(const FString& longName);

} } } //SceneProtocol::NetworkBridge::Unreal
