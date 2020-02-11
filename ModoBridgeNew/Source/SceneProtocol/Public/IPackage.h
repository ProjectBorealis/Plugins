// --------------------------------------------------------------------------
// An interface for creating engine-specific packages.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#ifndef IPACKAGE__H
#define IPACKAGE__H

#include <string>

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace BridgeProtocol {

      // An agnostic representation of a package, which represents the serialized
      // representation of an asset.
      //
      class IPackage {
      public:
        IPackage() : _name() { /* Empty */ };
        IPackage(const std::string& name) : _name(name) { /* Empty */ };
        IPackage(const IPackage& package) : _name(package._name) { /* Empty */ };
        virtual ~IPackage() { /* Empty */ };

        virtual bool                exists() const = 0;
        virtual bool                isOpen() const = 0;
        virtual const std::string&  name() const { return _name; }

        virtual void                name(const std::string& name) { _name = name; }

        virtual bool                open() = 0;
        virtual bool                close() = 0;

      protected:
        std::string   _name;
      };
    }
  }
}

#endif

