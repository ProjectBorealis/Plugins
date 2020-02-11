// --------------------------------------------------------------------------
// Implementation of plugin commandlets module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolCommandlets.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * Implements the CommandletPlugin module.
 */
class FSceneProtocolCommandletsModule : public IModuleInterface
{
public:

  //~ IModuleInterface interface

  virtual void StartupModule() override { }
  virtual void ShutdownModule() override { }

  virtual bool SupportsDynamicReloading() override
  {
    return true;
  }
};


IMPLEMENT_MODULE(FSceneProtocolCommandletsModule, SceneProtocolCommandlets);

