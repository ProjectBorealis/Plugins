// --------------------------------------------------------------------------
// Declaration of SceneProtocol plugin editor module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "Modules/ModuleManager.h"

#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"

class FToolBarBuilder;
class FMenuBuilder;
class SWidget;

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {

/**
 * The SceneProtocolEditor module provides UE4Editor UI integration for
 * the bridge to SceneProtocol server. This module should only ever have
 * light-weight UI bindings to functionality implemented in the
 * SceneProtocol module itself. This is to simplify changing our strategy
 * for editor integration and reusing the bridge code in non UI contexts.
 */
class FSceneProtocolEditorModule : public IModuleInterface
{
public:

    void StartupModule();
    void ShutdownModule();

    void DoFetchScene();
    void DoCancelFetch();

    void DoStartClient();
    void DoStopClient();

    void DoPushAll();
    void DoPushSelected();

    bool CanDoFetchScene();
    bool CanDoCancelFetch();
    
    bool CanDoStartClient();
    bool CanDoStopClient();
    
    bool CanDoPushAll();
    bool CanDoPushSelected();
private:

    void AddToolbarExtension(FToolBarBuilder& Builder);
    void AddMenuExtension(FMenuBuilder& Builder);

    TSharedRef<SWidget> CreateSceneProtocolMenu();
    void AddSceneProtocolMenuItems(FMenuBuilder& Builder);

    // Function that will loop through the current level and replace any SceneProtocl Static Mesh Actors with standard Static Mesh Actors.
    // Essentially automating Right Click - Replace Actor With
    static void SwapOutCustomMeshClasses();
    IConsoleObject* cmdSwapOutCustomMeshClasses;

private:
    TSharedPtr<class FUICommandList> PluginCommands;
};

}
}
}

