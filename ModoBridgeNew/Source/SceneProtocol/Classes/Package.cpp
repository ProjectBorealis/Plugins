// --------------------------------------------------------------------------
// Implementation of the UnrealEngine package.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "Package.h"

#include <cassert>

#include "AssetRegistryModule.h"

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {

      Package::Package() :
        IPackage(),
        _package(nullptr)
      {
        /* Empty */
      }

      Package::Package(const std::string& name) :
        IPackage(name),
        _package(nullptr)
      {
        open();
      }

      Package::Package(const Package& package) :
        IPackage(package),
        _package(nullptr)
      {
        package.save();
        open();
      }

      Package::~Package()
      {
        close();
      }

      bool Package::exists() const
      {
        return (_package != nullptr);
      }

      bool Package::isOpen() const
      {
        if (_name.empty()) {
          return false;
        }
        return exists();
      }

      UPackage* Package::package() const
      {
        return _package;
      }

      void Package::name(const std::string& name)
      {
        _name = name;
        if (_package) {
          close();
          open();
        }
      }

      bool Package::open()
      {
        if (_name.empty()) {
          return false;
        }

        _package = CreatePackage(nullptr, ANSI_TO_TCHAR(_name.c_str()));
        assert(_package);

        if (_package) {
          _package->FullyLoad();
          return true;
        }

        return false;
      }

      bool Package::close()
      {
        save();

        if (_package && _package->IsValidLowLevel())
        {
          _package->ConditionalBeginDestroy();
          _package = nullptr;
        }

        return true;
      }

      Package& Package::operator=(const Package& package)
      {
        if (this != &package) {
          close();
          _name = package._name;
          package.save();
          open();
        }
        return *this;
      }

      void Package::save() const
      {
        if (_package && _package->IsDirty()) {
          _package->PostEditChange();
        }
      }

      /** Unowned package impl */

      UnownedPackage::UnownedPackage() :
        IPackage(),
        _package(nullptr)
      {
        /* Empty */
      }

      UnownedPackage::UnownedPackage(UPackage* _package) :
        IPackage(),
        _package(_package)
      {
        open();
      }

      UnownedPackage::~UnownedPackage()
      {
        close();
      }

      bool UnownedPackage::exists() const
      {
        return (_package != nullptr);
      }

      bool UnownedPackage::isOpen() const
      {
        if (_name.empty()) {
          return false;
        }
        return exists();
      }

      UPackage* UnownedPackage::package() const
      {
        return _package;
      }

      void UnownedPackage::name(const std::string& name)
      {
        _name = name;
        if (_package) {
          close();
          open();
        }
      }

      bool UnownedPackage::open()
      {
        return true;
      }

      bool UnownedPackage::close()
      {
        save();
        _package = nullptr;

        return true;
      }

      void UnownedPackage::save() const
      {
        if (_package && _package->IsDirty()) {
          _package->PostEditChange();
        }
      }
    } // namespace Unreal
  } // namespace NetworkBridge
} // namespace SceneProtocol
