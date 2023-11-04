#pragma once

#include "CoreMinimal.h"

#include "NvBlastExtDamageShaders.h"

#include "BlastMeshComponent.h"
#include "BlastBaseDamageProgram.h"
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
	BlastRadialDamageProgram(float damage, float minRadius, float maxRadius, float impulseStrength = 0.0f,
	                         bool bVelChange = false, bool bRandomizeImpulse = false,
	                         float ImpulseRandomizationDivider = 2.f):
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

	virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
	                     UBlastMeshComponent& owner) const override
	{
		const float normalizedDamage = input.material->GetNormalizedDamage(Damage);
		if (normalizedDamage == 0.f)
		{
			return false;
		}

		NvBlastExtRadialDamageDesc damage[] = {
			{
				normalizedDamage, {input.localOrigin.X, input.localOrigin.Y, input.localOrigin.Z}, MinRadius, MaxRadius
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

	virtual FCollisionShape GetCollisionShape() const override
	{
		return FCollisionShape::MakeSphere(MaxRadius);
	}


	virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
	                                     UBlastMeshComponent& owner) const override
	{
		if (ImpulseStrength > 0.f && actorBody->IsInstanceSimulatingPhysics())
		{
			// Do not increase impulse a lot during randomization
			const float FinalImpulseStrength = bRandomizeImpulse
				                                   ? FMath::RandRange(
					                                   ImpulseStrength - ImpulseStrength / ImpulseRandomizationDivider,
					                                   ImpulseStrength + ImpulseStrength / (ImpulseRandomizationDivider
						                                   * 2.f))
				                                   : ImpulseStrength;
			actorBody->AddRadialImpulseToBody(FVector(input.worldOrigin), MaxRadius, FinalImpulseStrength, 0,
			                                  bImpulseVelChange);
		}
	}
};


///////////////////////////////////////////////////////////////////////////////
//  Capsule Damage
///////////////////////////////////////////////////////////////////////////////

FVector3f ClosestPointOnLine(const FVector3f& LineStart, const FVector3f& LineEnd, const FVector3f& Point)
{
	// Solve to find alpha along line that is closest point
	// Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Switchram Web Resource. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html 
	const float A = (LineStart - Point) | (LineEnd - LineStart);
	const float B = (LineEnd - LineStart).SizeSquared();
	// This should be robust to B == 0 (resulting in NaN) because clamp should return 1.
	const float T = FMath::Clamp<float>(-A / B, 0.f, 1.f);

	return LineStart + (T * (LineEnd - LineStart));
}

/**
Capsule Falloff Damage Program

Can be used for laser/cutting-like narrow capsules (kind of a swords) for example.
*/
struct BlastCapsuleDamageProgram final : public FBlastBaseDamageProgram
{
	BlastCapsuleDamageProgram(float damage, float halfHeight, float minRadius, float maxRadius,
	                          float impulseStrength = 0.0f, bool bVelChange = false) :
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


	virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
	                     UBlastMeshComponent& owner) const override
	{
		FVector3f CapsuleDir = input.localRot.RotateVector(FVector3f::UpVector);

		FVector3f pointA = input.localOrigin + CapsuleDir * HalfHeight;
		FVector3f pointB = input.localOrigin - CapsuleDir * HalfHeight;

		const float normalizedDamage = input.material->GetNormalizedDamage(Damage);
		if (normalizedDamage == 0.f)
		{
			return false;
		}

		NvBlastExtCapsuleRadialDamageDesc damage[] = {
			{
				normalizedDamage,
				{pointA.X, pointA.Y, pointA.Z},
				{pointB.X, pointB.Y, pointB.Z},
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

	virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
	                                     UBlastMeshComponent& owner) const override
	{
		if (ImpulseStrength > 0.f && actorBody->IsInstanceSimulatingPhysics())
		{
			FVector3f CapsuleDir = input.worldRot.RotateVector(FVector3f::UpVector);

			FVector3f pointA = input.worldOrigin + CapsuleDir * HalfHeight;
			FVector3f pointB = input.worldOrigin - CapsuleDir * HalfHeight;

			FVector3f ActorCom = FVector3f(actorBody->GetCOMPosition());
			FVector3f CapsulePoint = ClosestPointOnLine(pointA, pointB, ActorCom);

			actorBody->AddRadialImpulseToBody(FVector(CapsulePoint), (ActorCom - CapsulePoint).SizeSquared(),
			                                  ImpulseStrength, 0, bImpulseVelChange);
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
	BlastShearDamageProgram(float damage, float minRadius, float maxRadius, float impulseStrength = 0.0f,
	                        bool bVelChange = false) :
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


	virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
	                     UBlastMeshComponent& owner) const override
	{
		const float normalizedDamage = input.material->GetNormalizedDamage(Damage);
		if (normalizedDamage == 0.f)
		{
			return false;
		}

		FVector3f LocalNormal = input.localRot.GetForwardVector();

		NvBlastExtShearDamageDesc damage[] = {
			{
				normalizedDamage,
				{LocalNormal.X, LocalNormal.Y, LocalNormal.Z},
				{input.localOrigin.X, input.localOrigin.Y, input.localOrigin.Z},
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

	virtual FCollisionShape GetCollisionShape() const override
	{
		return FCollisionShape::MakeSphere(MaxRadius);
	}

	virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
	                                     UBlastMeshComponent& owner) const override
	{
		if (ImpulseStrength > 0.f && actorBody->IsInstanceSimulatingPhysics())
		{
			actorBody->AddRadialImpulseToBody(FVector(input.worldOrigin), MaxRadius, ImpulseStrength, 0,
			                                  bImpulseVelChange);
		}
	}
};
