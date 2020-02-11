// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine point light actor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "MetadataAsset.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"

#include "SceneProtocolPointLightActor.generated.h"

UCLASS()
class SCENEPROTOCOL_API ASceneProtocolPointLightActor : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ASceneProtocolPointLightActor(const FObjectInitializer& ObjectInitializer);

  ULightComponent* GetLightComponent();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Point Light")
  UPointLightComponent * pointComponent;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
  UMetadataAsset* Metadata;
};