// Copyright 2019 Tefel. All Rights Reserved

#include "GravityCharacter.h"
#include "GravityMovementComponent.h"

UGravityMovementComponent* AGravityCharacter::GetGravityMovementComponent()
{
	return Cast<UGravityMovementComponent>(GetMovementComponent());
}