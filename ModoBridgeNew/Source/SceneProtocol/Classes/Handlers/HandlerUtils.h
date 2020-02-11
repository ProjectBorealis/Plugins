// --------------------------------------------------------------------------
// Handler utils
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once
#include "StringData.h"
#include "Containers/UnrealString.h"
#include "SceneProtocol.h" // just for STP_ENABLE_SCENEGRAPH_HIERARCHY
#include <string>

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {
  
  FString stringDataToFString(const SceneTransmissionProtocol::String::Data& data);
  std::string stringDataToStdString(const SceneTransmissionProtocol::String::Data& data);

#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
  void attachParentActor(const SceneTransmissionProtocol::Client::Response *response, AActor *actor);
#endif

} // namespace SceneProtocol
} // namespace NetworkBridge
} // namespace Unreal
