// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine directional light actor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "MetadataAsset.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"

#include "SceneProtocolDirectionalLightActor.generated.h"

UCLASS()
class SCENEPROTOCOL_API ASceneProtocolDirectionalLightActor : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ASceneProtocolDirectionalLightActor(const FObjectInitializer& ObjectInitializer);

  ULightComponent* GetLightComponent();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Directional Light")
  UDirectionalLightComponent * directionalComponent;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
  UMetadataAsset* Metadata;
};