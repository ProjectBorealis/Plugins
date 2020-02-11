// --------------------------------------------------------------------------
// Declares SceneProtocol plugin editor commands.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "SceneProtocolEditorStyle.h"

#include "Framework/Commands/Commands.h"

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {


class FSceneProtocolEditorCommands : public TCommands<FSceneProtocolEditorCommands>
{
public:

  FSceneProtocolEditorCommands()
    : TCommands<FSceneProtocolEditorCommands>(
        TEXT("SceneProtocolEditor"),
        NSLOCTEXT("Contexts", "SceneProtocolEditor", "SceneProtocolEditor Plugin"),
        NAME_None, FSceneProtocolEditorStyle::GetStyleSetName()
    )
  {
  }

  // TCommands<> interface
  virtual void RegisterCommands() override;

public:

  TSharedPtr< FUICommandInfo > FetchSceneAction;
  TSharedPtr< FUICommandInfo > CancelFetchAction;

  TSharedPtr< FUICommandInfo > ClientStartAction;
  TSharedPtr< FUICommandInfo > ClientStopAction;

  TSharedPtr< FUICommandInfo > PushAllItems;
  TSharedPtr< FUICommandInfo > PushSelectedItems;

};

}
}
}

