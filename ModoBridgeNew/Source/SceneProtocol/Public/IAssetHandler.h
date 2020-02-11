// ----------------------------------------------------------------------------
// An interface for creating asset handlers.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.
//-----------------------------------------------------------------------------
#pragma once

#include "Response.h"

#include <string>
#include <functional>

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace BridgeProtocol {
      class IPackage;
    } // namespace BridgeProtocol
    namespace Unreal {
      class IAssetHandler
      {
      public:
        virtual ~IAssetHandler() {}
        virtual bool create(
            SceneProtocol::NetworkBridge::BridgeProtocol::IPackage* package,
            const SceneTransmissionProtocol::Client::Response *response,
            std::string& uniqueName) = 0;

        virtual bool update(
            SceneProtocol::NetworkBridge::BridgeProtocol::IPackage* package,
            const SceneTransmissionProtocol::Client::Response *response,
            std::string& uniqueName) = 0;
      };

      struct AssetHandlerFactory
      {
        std::function<IAssetHandler*()> create;
        std::function<void(IAssetHandler*)> destroy;

        bool isValid() {
          return create && destroy;
        }
      };
    } // namespace BridgeProtocol
  } // namespace NetworkBridge
} // namespace SceneProtocol

#define NETBRIDGE_ADD_ASSET_HANDLER(classname, responsetype, create_func, destroy_func) \
  struct classname##Initializer\
  { \
    classname##Initializer() {\
      SceneProtocol::NetworkBridge::Unreal::AssetHandlerFactory factory{create_func, destroy_func};\
      SceneProtocol::NetworkBridge::Unreal::AssetFactory::instance().registerHandler(responsetype, factory); \
    }\
  }; \
  classname##Initializer classname##Init;
