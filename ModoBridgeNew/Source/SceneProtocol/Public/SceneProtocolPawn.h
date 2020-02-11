// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine pawn.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "GameFramework/DefaultPawn.h"
#include "Camera/CameraComponent.h"
#include "SceneProtocolPawn.generated.h"

class UInputComponent;

UCLASS(config = Game)
class SCENEPROTOCOL_API ASceneProtocolPawn : public ADefaultPawn
{
  GENERATED_BODY()

    /** First person camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FirstPersonCameraComponent;

public:
  ASceneProtocolPawn();

public:
  /** Returns FirstPersonCameraComponent subobject **/
  FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};
