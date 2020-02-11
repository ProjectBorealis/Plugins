// --------------------------------------------------------------------------
// Declaration of SceneProtocol log category.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Runtime/Launch/Resources/Version.h"

// For this option to properly be disabled UPROPERTY line in scene protocol options
// must be commented out manually due to Unreal header tool working before prerocessor definitions
//#define STP_CUSTOM_ACTOR_CLASSES

#ifdef STP_CUSTOM_ACTOR_CLASSES
// For this option to properly be disabled UPROPERTY line in scene protocol options
// must be commented out manually due to Unreal header tool working before prerocessor definitions
#define STP_METADATA_SUPPORT
#endif

//#define STP_LOCAL_MATRIX_PAYLOAD

// define this to enable the scene graph hierarchy in Unreal
//#define STP_ENABLE_SCENEGRAPH_HIERARCHY

DECLARE_LOG_CATEGORY_EXTERN(LogSceneProtocol, Log, All);

