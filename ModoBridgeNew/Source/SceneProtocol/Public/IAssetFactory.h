// --------------------------------------------------------------------------
// An interface for creating engine-specific assets.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#ifndef IASSETFACTORY__H
#define IASSETFACTORY__H

#include "Response.h"

#include <string>

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace BridgeProtocol {
      class IPackage;

      // This is the AssetFactory interface, defines is an engine agnostic way elements of 3D graphics,
      // and provides an unique name reference, to reference assets in future systems.
      class IAssetFactory {
      public:
        // For package create pass in the name of the package needed, the package will then either be
        // opened if it already exists, or created if it doesn't, if this all goes well, return a pointer, 
        // to the package, or nullptr if not.
        virtual IPackage*  createPackage(const std::string& name) = 0;

        // For all asset creates, pass in name of package to be created in, and the Node pointer containing the
        // data, item will be created if data is good, and it's unique name returned, unique name will only
        // be different in the case of a name clash.  If created return true, otherwise false.
        virtual bool    createMaterialDefinition(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createMaterial(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createTexture(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createLight(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createStaticMesh(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createMeshInstance(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createCamera(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;
        virtual bool    createCustom(const std::string& packageName, const SceneTransmissionProtocol::Client::Response *response, std::string& uniqueName) = 0;

      protected:
        IAssetFactory() {};
        virtual ~IAssetFactory() {};
      };
    }
  }
}

#endif