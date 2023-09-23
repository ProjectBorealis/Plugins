#pragma once

#include "CoreMinimal.h"
#include "BlastBaseDamageProgram.h"

#include "NvBlastExtDamageShaders.h"
#include "BlastMaterial.h"


///////////////////////////////////////////////////////////////////////////////
// This file contains default/basic damage programs for BlastPlugin. Users 
// are welcome to implement their own by looking at these examples.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  Radial Damage
///////////////////////////////////////////////////////////////////////////////

/**
Radial Damage Program with falloff
*/
struct BlastRadialDamageProgram final : public FBlastBaseDamageProgram
{
	BlastRadialDamageProgram(float damage, float minRadius, float maxRadius, float impulseStrength = 0.0f, bool bVelChange = false, bool bRandomizeImpulse = false, float ImpulseRandomizationDivider = 2.f):
		Damage(damage),
		MinRadius(minRadius),
		MaxRadius(maxRadius),
		ImpulseStrength(impulseStrength),
		bImpulseVelChange(bVelChange),
		bRandomizeImpulse(bRandomizeImpulse),
		ImpulseRandomizationDivider(ImpulseRandomizationDivider)
	{
	}

	// Damage amount
	float Damage;

	// Inner radius of damage
	float MinRadius;

	// Outer radius of damage
	float MaxRadius;

	// Impulse to apply after splitting
	float ImpulseStrength;

	// If true, the impulse will ignore mass of objects and will always result in a fixed velocity change
	bool bImpulseVelChange;

	// If true, we'll randomize impulses a bit
	bool bRandomizeImpulse;

	// Use this divider to min/max impulse randomization
	float ImpulseRandomizationDivider;

	virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input, UBlastMeshComponent& owner) const override
	{
		const float normalizedDamage = input.material->GetNormalizedDamage(Damage);
		if (normalizedDamage == 0.f)
		{
			return false;
		}

		NvBlastExtRadialDamageDesc damage[] = {
			{
				normalizedDamage,{ input.localOrigin.X, input.localOrigin.Y, input.localOrigin.Z }, MinRadius, MaxRadius
			}
		};

		NvBlastExtProgramParams programParams(damage, input.material);

		programParams.accelerator = owner.GetAccelerator();

		NvBlastDamageProgram program = {
			NvBlastExtFalloffGraphShader,
			NvBlastExtFalloffSubgraphShader
		};

		return owner.ExecuteBlastDamageProgram(actorIndex, program, programParams, DamageType);
	}

	virtual FCollisionShape GetCollisionShape() const  override
	{
		return FCollisionShape::MakeSphere(MaxRadius);
	}


	virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input, UBlastMeshComponent& owner) const override
	{
		if (ImpulseStrength > 0.f && actorBody->IsInstanceSimulatingPhysics())
		{
			// Do not increase impulse a lot during randomization
			const float FinalImpulseStrength = bRandomizeImpulse ? FMath::RandRange(ImpulseStrength - ImpulseStrength / ImpulseRandomizationDivider,
				ImpulseStrength + ImpulseStrength / (ImpulseRandomizationDivider * 2.f)) : ImpulseStrength;
			actorBody->AddRadialImpulseToBody(input.worldOrigin, MaxRadius, FinalImpulseStrength, 0, bImpulseVelChange);
		}
	}
};


///////////////////////////////////////////////////////////////////////////////
//  Capsule Damage
///////////////////////////////////////////////////////////////////////////////

/**
Capsule Falloff Damage Program

Can be used for laser/cutting-like narrow capsules (kind of a swords) for example.
*/
struct BlastCapsuleDamageProgram final : public FBlastBaseDamageProgram
{
	BlastCapsuleDamageProgram(float damage, float halfHeight, float minRadius, float maxRadius, float impulseStrength = 0.0f, bool bVelChange = false) :
		Damage(damage),
		HalfHeight(halfHeight),
		MinRadius(minRadius),
		MaxRadius(maxRadius),
		ImpulseStrength(impulseStrength),
		bImpulseVelChange(bVelChange)
	{
	}

	// Damage amount
	float Damage;

	// Capsule Half Height
	float HalfHeight;

	// Inner radius of damage
	float MinRadius;

	// Outer radius of damage
	float MaxRadius;

	// Impulse to apply after splitting
	float ImpulseStrength;

	// If true, the impulse will ignore mass of objects and will always result in a fixed velocity change
	bool bImpulseVelChange;


	virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input, UBlastMeshComponent& owner) const override
	{
		FVector CapsuleDir = FVector(0, 0, 1);
		CapsuleDir = input.localRot.RotateVector(CapsuleDir);

		FVector pointA = input.localOrigin + CapsuleDir * HalfHeight;
		FVector pointB = input.localOrigin - CapsuleDir * HalfHeight;

		const float normalizedDamage = input.material->GetNormalizedDamage(Damage);
		if (normalizedDamage == 0.f)
		{
			return false;
		}

		NvBlastExtCapsuleRadialDamageDesc damage[] = {
			{
				normalizedDamage,
				{ pointA.X, pointA.Y, pointA.Z },
				{ pointB.X, pointB.Y, pointB.Z },
				MinRadius, 
				MaxRadius
			}
		};

		NvBlastExtProgramParams programParams(damage, input.material);

		programParams.accelerator = owner.GetAccelerator();
		
		NvBlastDamageProgram program = {
			NvBlastExtCapsuleFalloffGraphShader,
			NvBlastExtCapsuleFalloffSubgraphShader
		};

		return owner.ExecuteBlastDamageProgram(actorIndex, program, programParams, DamageType);
	}

	virtual FCollisionShape GetCollisionShape() const override
	{
		return FCollisionShape::MakeCapsule(MaxRadius, HalfHeight);
	}

	virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input, UBlastMeshComponent& owner) const override
	{
		if (ImpulseStrength > 0.f && actorBody->IsInstanceSimulatingPhysics())
		{
			FVector CapsuleDir = FVector(0, 0, 1);
			CapsuleDir = input.worldRot.RotateVector(CapsuleDir);

			FVector pointA = input.worldOrigin + CapsuleDir * HalfHeight;
			FVector pointB = input.worldOrigin - CapsuleDir * HalfHeight;

			FVector ActorCom = actorBody->GetCOMPosition();
			FVector CapsulePoint = FMath::ClosestPointOnLine(pointA, pointB, ActorCom);

			actorBody->AddRadialImpulseToBody(CapsulePoint, (ActorCom - CapsulePoint).SizeSquared(), ImpulseStrength, 0, bImpulseVelChange);
		}
	}
};


///////////////////////////////////////////////////////////////////////////////
//  Shear Damage
///////////////////////////////////////////////////////////////////////////////

/**
Shear Damage Program
*/
struct BlastShearDamageProgram final : public FBlastBaseDamageProgram
{
	BlastShearDamageProgram(float damage, float minRadius, float maxRadius, float impulseStrength = 0.0f, bool bVelChange = false) :
		Damage(damage),
		MinRadius(minRadius),
		MaxRadius(maxRadius),
		ImpulseStrength(impulseStrength),
		bImpulseVelChange(bVelChange)
	{
	}

	// Damage amount
	float Damage;

	// Inner radius of damage
	float MinRadius;

	// Outer radius of damage
	float MaxRadius;

	// Impulse to apply after splitting
	float ImpulseStrength;

	// If true, the impulse will ignore mass of objects and will always result in a fixed velocity change
	bool bImpulseVelChange;


	virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input, UBlastMeshComponent& owner) const override
	{
		const float normalizedDamage = input.material->GetNormalizedDamage(Damage);
		if (normalizedDamage == 0.f)
		{
			return false;
		}

		FVector LocalNormal = input.localRot.GetForwardVector();

		NvBlastExtShearDamageDesc damage[] = {
			{
				normalizedDamage,
				{ LocalNormal.X, LocalNormal.Y, LocalNormal.Z },
				{ input.localOrigin.X, input.localOrigin.Y, input.localOrigin.Z },
				MinRadius,
				MaxRadius
			}
		};

		NvBlastExtProgramParams programParams(damage, input.material);

		programParams.accelerator = owner.GetAccelerator();

		NvBlastDamageProgram program = {
			NvBlastExtShearGraphShader,
			NvBlastExtShearSubgraphShader
		};

		return owner.ExecuteBlastDamageProgram(actorIndex, program, programParams, DamageType);
	}

	virtual FCollisionShape GetCollisionShape() const  override
	{
		return FCollisionShape::MakeSphere(MaxRadius);
	}

	virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input, UBlastMeshComponent& owner) const override
	{
		if (ImpulseStrength > 0.f && actorBody->IsInstanceSimulatingPhysics())
		{
			actorBody->AddRadialImpulseToBody(input.worldOrigin, MaxRadius, ImpulseStrength, 0, bImpulseVelChange);
		}
	}
};
