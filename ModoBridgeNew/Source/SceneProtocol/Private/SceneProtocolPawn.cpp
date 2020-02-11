// --------------------------------------------------------------------------
// Implementation of the SceneProtocol pawn.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#include "SceneProtocolPawn.h"
#include "SceneProtocol.h"
#include "GameFramework/InputSettings.h"

ASceneProtocolPawn::ASceneProtocolPawn()
{
  // Create a CameraComponent
  FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
  FirstPersonCameraComponent->SetupAttachment(GetMeshComponent());
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 24
  FirstPersonCameraComponent->SetRelativeLocation(FVector (0.f, 0.f, 0.f)); // Position the camera
#else
  FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera
#endif
  FirstPersonCameraComponent->bUsePawnControlRotation = true;
  // If this actor has been reclaimed from the GC after a re-fetch, then its
  // component will likely also be reused. The component will still be marked
  // as pending kill, so we need to clear that flag otherwise the component
  // won't be registered.
  if (FirstPersonCameraComponent->IsPendingKill()) {
    FirstPersonCameraComponent->ClearPendingKill();
  }
  RootComponent = FirstPersonCameraComponent;
}
