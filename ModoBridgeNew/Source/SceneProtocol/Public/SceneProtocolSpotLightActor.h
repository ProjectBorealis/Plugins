// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine spot light actor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "MetadataAsset.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"

#include "SceneProtocolSpotLightActor.generated.h"

UCLASS()
class SCENEPROTOCOL_API ASceneProtocolSpotLightActor : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ASceneProtocolSpotLightActor(const FObjectInitializer& ObjectInitializer);

  ULightComponent* GetLightComponent();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spot Light")
  USpotLightComponent * spotComponent;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
  UMetadataAsset* Metadata;
};