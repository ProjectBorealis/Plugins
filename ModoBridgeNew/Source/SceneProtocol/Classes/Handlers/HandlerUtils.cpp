// --------------------------------------------------------------------------
// Handler utils
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#include "HandlerUtils.h"
#include "PackageMapper.h"
#include "NodeHandler.h"

#include "Editor/GroupActor.h"

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {

  FString stringDataToFString(const SceneTransmissionProtocol::String::Data& data)
  {
    FString val((int32)data.size, ANSI_TO_TCHAR(data.str));
    return val;
  }


  std::string stringDataToStdString(const SceneTransmissionProtocol::String::Data& data)
  {
    std::string val;
    val.append(data.str, data.size);
    return val;
  }


#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
  void attachParentActor(const SceneTransmissionProtocol::Client::Response *response, AActor *actor)
  {
      auto &packageMapper = SPNB::Unreal::PackageMapper::instance();
      auto nodeHandler = SPNB::Unreal::NodeHandlerManager::get();
      auto currentToken = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
      std::string parentToken;
      for (;;)
      {
          if (!nodeHandler->getParent(currentToken, parentToken))
          {
              break;
          }

          if (parentToken.empty())
          {
              break;
          }

          auto parentObject = packageMapper.findObject(parentToken);
          assert(nullptr != parentObject); // since we've serialized the scene graph, this shouldn't fail

          if (parentObject->IsA(AGroupActor::StaticClass()))
          {
              break;
          }
          currentToken = parentToken;
      }

      auto parentObject = packageMapper.findObject(parentToken);
      auto parentActor = Cast<AActor>(parentObject);
      actor->AttachToActor(parentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
  }
#endif


} // namespace SceneProtocol
} // namespace NetworkBridge
} // namespace Unreal
