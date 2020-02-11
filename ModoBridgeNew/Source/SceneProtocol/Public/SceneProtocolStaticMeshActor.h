// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine StaticMeshActor
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "MetadataAsset.h"

#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"

#include "SceneProtocolStaticMeshActor.generated.h"

// Sadly the UnrealHeaderTool doesn't support namespaces

UCLASS()
class SCENEPROTOCOL_API ASceneProtocolStaticMeshActor : public AStaticMeshActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ASceneProtocolStaticMeshActor(const FObjectInitializer& ObjectInitializer);

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
  UMetadataAsset* Metadata;
};