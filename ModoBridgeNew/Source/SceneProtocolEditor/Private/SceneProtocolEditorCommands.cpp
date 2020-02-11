// --------------------------------------------------------------------------
// Implements SceneProtocol plugin editor commands registration.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolEditorCommands.h"

namespace SPNBU = SceneProtocol::NetworkBridge::Unreal;

#define LOCTEXT_NAMESPACE "SceneProtocolEditor"

void SPNBU::FSceneProtocolEditorCommands::RegisterCommands()
{
  UI_COMMAND(
    FetchSceneAction,
    "Import from SceneProtocol",
    "Fetches the scene from SceneProtocol based on the Project Settings.",
    EUserInterfaceActionType::Button, FInputChord(EKeys::I, false, true, true, false)
  );

  UI_COMMAND(
    CancelFetchAction,
    "Cancel Fetch",
    "Stops the current fetch in progress.",
    EUserInterfaceActionType::Button, FInputChord(EKeys::C, false, true, true, false)
  );

  UI_COMMAND(
    ClientStartAction,
    "Client Start",
    "Start the client.",
    EUserInterfaceActionType::Button, FInputChord(EKeys::S, false, true, true, false)
  );

  UI_COMMAND(
    ClientStopAction,
    "Client Stop",
    "Start the client.",
    EUserInterfaceActionType::Button, FInputChord(EKeys::Q, false, true, true, false)
  );

  UI_COMMAND(
    PushAllItems,
    "Push all",
    "Send all actors in the current scene graph to the server.",
    EUserInterfaceActionType::Button, FInputChord(EKeys::A, false, true, true, false)
  );

  UI_COMMAND(
    PushSelectedItems,
    "Push Selected",
    "Send only selected actors in the current scene graph to the server.",
    EUserInterfaceActionType::Button, FInputChord(EKeys::D, false, true, true, false)
  );
}

#undef LOCTEXT_NAMESPACE

