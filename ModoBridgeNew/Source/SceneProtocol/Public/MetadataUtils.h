// --------------------------------------------------------------------------
// Helper for creating metadata. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "Containers/UnrealString.h"

class UDataTable;
class UPackage;

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {

  FString GetHashedInputPath(const FString& input);

}
}
}
