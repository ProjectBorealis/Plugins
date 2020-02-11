// --------------------------------------------------------------------------
// Implementation of the SceneProtocol-specific spot light actor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolSpotLightActor.h"
#include "SceneProtocol.h"

ASceneProtocolSpotLightActor::ASceneProtocolSpotLightActor(const FObjectInitializer& ObjectInitializer) {
  spotComponent = ObjectInitializer.CreateDefaultSubobject<USpotLightComponent>(this, TEXT("Spot light"));
  // If this actor has been reclaimed from the GC after a re-fetch, then its
  // component will likely also be reused. The component will still be marked
  // as pending kill, so we need to clear that flag otherwise the component
  // won't be registered.
  if (spotComponent->IsPendingKill()) {
    spotComponent->ClearPendingKill();
  }
  RootComponent = spotComponent;
}

ULightComponent* ASceneProtocolSpotLightActor::GetLightComponent()
{
  return Cast<ULightComponent>(spotComponent);
}