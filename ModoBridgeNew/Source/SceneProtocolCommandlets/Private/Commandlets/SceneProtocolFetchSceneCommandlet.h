// --------------------------------------------------------------------------
// A commandlet for fetching a scene from a SceneProtocol server.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "Commandlets/Commandlet.h"
#include "SceneProtocolFetchSceneCommandlet.generated.h"

/**
 * The SceneProtocolFetchScene commandlet allows automated retrieval of assets from
 * the SceneProtocol plaftorm. It takes the following arguments:
 *
 * - SaveSettingsToProject : If present, then any options provided via the command
 *     line will be saved into the Project Settings
 *
 * - TargetMap : (optional) [LongPath] Which map within the project to import data to. If
 *     not specified, then either the last used, or project default map will be
 *     targeted for import. This is a Long Path, eg: '/Game/MyLevel'
 *
 * In Addition, and of the following Bridge options can be supplied:
 *
 *  - Mode <string> "Server" | "File"
 *  - Asset <string>
 *  - RenderNode <string>
 *  - Server <string>
 *  - Port <int>
 *
 */
UCLASS()
class USceneProtocolFetchSceneCommandlet : public UCommandlet
{
  GENERATED_BODY()

  public:

    USceneProtocolFetchSceneCommandlet();
    USceneProtocolFetchSceneCommandlet(const FObjectInitializer& ObjectInitializer);

    virtual int32 Main(const FString& params) override;

  private:

    bool _persistOptions;
    bool _useSourceControl;
    FString _targetMap;

    bool ParseParams(const FString& Params);

    UWorld* LoadTargetMap();

    void InitSourceControl();

    void ClearDirtyPackages();
    int32 AttemptSave();


};


