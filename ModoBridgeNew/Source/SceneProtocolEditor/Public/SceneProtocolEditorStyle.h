// --------------------------------------------------------------------------
// Declaration of SceneProtocol plugin editor style.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {


class FSceneProtocolEditorStyle
{
public:

  static void Initialize();

  static void Shutdown();

  /** reloads textures used by slate renderer */
  static void ReloadTextures();

  /** @return The Slate style set for the Shooter game */
  static const ISlateStyle& Get();

  static FName GetStyleSetName();

private:

  static TSharedRef< class FSlateStyleSet > Create();

private:

  static TSharedPtr< class FSlateStyleSet > StyleInstance;
};

}
}
}

