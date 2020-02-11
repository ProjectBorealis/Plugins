// --------------------------------------------------------------------------
// Implementation of the SceneProtocol-specific point light actor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolPointLightActor.h"
#include "SceneProtocol.h"

ASceneProtocolPointLightActor::ASceneProtocolPointLightActor(const FObjectInitializer& ObjectInitializer) {
  pointComponent = ObjectInitializer.CreateDefaultSubobject<UPointLightComponent>(this, TEXT("Point light"));
  // If this actor has been reclaimed from the GC after a re-fetch, then its
  // component will likely also be reused. The component will still be marked
  // as pending kill, so we need to clear that flag otherwise the component
  // won't be registered.
  if (pointComponent->IsPendingKill()) {
    pointComponent->ClearPendingKill();
  }
  RootComponent = pointComponent;
}

ULightComponent* ASceneProtocolPointLightActor::GetLightComponent()
{
  return Cast<ULightComponent>(pointComponent);
}