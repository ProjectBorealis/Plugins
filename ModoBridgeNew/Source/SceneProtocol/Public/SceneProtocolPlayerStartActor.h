// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine PlayerStartActor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "MetadataAsset.h"
#include "GameFramework/Actor.h"

#include "Editor/EditorEngine.h"
#include "GameFramework/PlayerStart.h"

#include "SceneProtocolPlayerStartActor.generated.h"

UCLASS()
class SCENEPROTOCOL_API ASceneProtocolPlayerStartActor : public APlayerStart
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ASceneProtocolPlayerStartActor(const FObjectInitializer& ObjectInitializer);

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
  UMetadataAsset* Metadata;
};


