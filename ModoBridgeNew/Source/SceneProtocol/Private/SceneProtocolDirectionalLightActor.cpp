// --------------------------------------------------------------------------
// Implementation of the SceneProtocol-specific directional light actor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolDirectionalLightActor.h"
#include "SceneProtocol.h"

ASceneProtocolDirectionalLightActor::ASceneProtocolDirectionalLightActor(const FObjectInitializer& ObjectInitializer) {
  directionalComponent = ObjectInitializer.CreateDefaultSubobject<UDirectionalLightComponent>(this, TEXT("Directional light"));
  // If this actor has been reclaimed from the GC after a re-fetch, then its
  // component will likely also be reused. The component will still be marked
  // as pending kill, so we need to clear that flag otherwise the component
  // won't be registered.
  if (directionalComponent->IsPendingKill()) {
    directionalComponent->ClearPendingKill();
  }
  RootComponent = directionalComponent;
}

ULightComponent* ASceneProtocolDirectionalLightActor::GetLightComponent()
{
  return Cast<ULightComponent>(directionalComponent);
}