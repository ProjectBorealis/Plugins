// --------------------------------------------------------------------------
// A commandlet for creating an Unreal project.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Commandlets/Commandlet.h"
#include "SceneProtocolCreateProjectCommandlet.generated.h"

/**
 * The SceneProtocolCreateProject commandlet allows automated retrieval of assets from
 * the SceneProtocol plaftorm. It takes the following arguments:
 *
 * - TemplatePath (optional) [Path] A path to a template .uproject. If this is
 *      omitted, the default SceneProtocol project template will be used.
 */
UCLASS()
class USceneProtocolCreateProjectCommandlet : public UCommandlet
{
  GENERATED_BODY()

  public:

    USceneProtocolCreateProjectCommandlet();
    USceneProtocolCreateProjectCommandlet(const FObjectInitializer& ObjectInitializer);

    virtual int32 Main(const FString& params) override;

  private:

    FString _projectPath;
    FString _templatePath;
    bool _sourceBuild;

    bool ParseParams(const FString& Params);

    FString GetDefaultTempatePath() const;

};


