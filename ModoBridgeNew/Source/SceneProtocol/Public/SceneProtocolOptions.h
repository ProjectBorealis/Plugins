// --------------------------------------------------------------------------
// Declaration of SceneProtocol module options.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include <string>
#include <SceneProtocol.h>

#include "SceneProtocolOptions.generated.h"


UCLASS(Config=Editor)
class SCENEPROTOCOL_API USceneProtocolOptions : public UObject
{

  GENERATED_UCLASS_BODY()

public:

  // Note: When adding properties that you wish to be saved in the Config
  // the 'Config' flag is essential, otherwise, if no properties have this
  // the editor will exit on an assert.

  UPROPERTY(Config, EditAnywhere, Category = "Server")
  FString Server;

  UPROPERTY(Config, EditAnywhere, Category = "Server")
  uint16 Port;

  UPROPERTY(Config, EditAnywhere, Category = "Server")
  uint32 Timeout;

//#ifdef STP_METADATA_SUPPORT
  // For this option to properly be disabled UPROPERTY line must be commented out
  // due to Unreal header tool working before prerocessor definitions
  UPROPERTY(Config, EditAnywhere, Category = "Options")
  bool ImportMetadata;
//#endif

//#ifdef STP_CUSTOM_ACTOR_CLASSES
  // For this option to properly be disabled UPROPERTY line must be commented out
  // due to Unreal header tool working before prerocessor definitions
  UPROPERTY(Config, EditAnywhere, Category = "Options")
  bool UseCustomActorClasses;
//#endif


  bool ProcessEventLoop;


  /**
   * Populates the options object from any parameters defined
   * in the supplied params string, such as may come from a commandlet, etc...
   * Params should be the TitleCase version of the parameter name.
   *
   * @note 'Mode' should be supplied as the enum names. If mode is ommited and Server
   * or Asset are supplied, then mode will be inferred. If both are supplied,
   * Server takes precedence.
   * Booleans must be specified as '1' otherwise (if present), false will be
   * assumed for all other values.
   *
   * @reuturns true if Parameter values were found in the input string.
   */
  bool ReadFromParamString(const FString& Params);
  void UpdateFrom(const USceneProtocolOptions* otherOptions, const bool configOnly=false);

  void PrintToLog();

};


