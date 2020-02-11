// --------------------------------------------------------------------------
// Implementation of the metadata helpers.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "MetadataUtils.h"
#include "Response.h"

#include "Containers/UnrealString.h"
#include "Misc/SecureHash.h"

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {

      FString GetHashedInputPath(const FString& input)
      {
        // We'll hash the input path (which may be very long)
        return FMD5::HashAnsiString(*input);
      }
    } // namespace Unreal
  } // namespace NetworkBridge
} // namespace SceneProtocol
