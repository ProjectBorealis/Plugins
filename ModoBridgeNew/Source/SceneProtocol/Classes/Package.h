// --------------------------------------------------------------------------
// Declaration of the UnrealEngine implementation of the IPackage API.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#ifndef PACKAGE__H
#define PACKAGE__H

#include "IPackage.h"
#include "Runtime/Launch/Resources/Version.h"

#include <string>

class UPackage;

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {
      // Package that we created and own
      class Package : public SceneProtocol::NetworkBridge::BridgeProtocol::IPackage {
      public:
        Package();
        Package(const std::string& name);
        Package(const Package& package);
        ~Package();

        bool      exists() const override;
        bool      isOpen() const override;
        UPackage* package() const;

        void      name(const std::string& name) override;

        bool      open() override;
        bool      close() override;

        Package&  operator=(const Package& package);

      private:
        void      save() const;

        UPackage*   _package;
      };

      // Package that we don't own
      class UnownedPackage : public SceneProtocol::NetworkBridge::BridgeProtocol::IPackage {
      public:
        UnownedPackage();
        UnownedPackage(UPackage* _package);
        ~UnownedPackage();

        bool      exists() const override;
        bool      isOpen() const override;
        UPackage* package() const;

        void      name(const std::string& name) override;

        bool      open() override;
        bool      close() override;

      private:
        void      save() const;

        UPackage*   _package;
      };

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
      struct cmpFNames {
         inline bool operator()(const FName& a, const FName& b) const 
         {
            return a.LexicalLess (b);
         }
      };
#endif
    }
  }
}

#endif

