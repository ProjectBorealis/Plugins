// Copyright Epic Games, Inc. All Rights Reserved.

#include "RigUnit_PowerIK.h"
#include "Units/RigUnitContext.h"

FRigUnit_PowerIK_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()
	FRigBoneHierarchy* Hierarchy = ExecuteContext.GetBones();
	if (Hierarchy == nullptr)
	{
		return;
	}

	if (Context.State == EControlRigState::Init)
	{
		// load skeleton into solver
		Core.LoadBonesFromControlRig(Hierarchy);

		FPowerIKEffector* Effector = const_cast<FPowerIKEffector*>(Effectors.GetData());
		TArrayView<FPowerIKEffector> EffectorsArray(Effector, Effectors.Num());

		FPowerIKExcludedBone* ExcludedBone = const_cast<FPowerIKExcludedBone*>(ExcludedBones.GetData());
		TArrayView<FPowerIKExcludedBone> ExcludedBonesArray(ExcludedBone, ExcludedBones.Num());

		FPowerIKBoneBendDirection* BendDirection = const_cast<FPowerIKBoneBendDirection*>(BendDirections.GetData());
		TArrayView<FPowerIKBoneBendDirection> BendDirectionsArray(BendDirection, BendDirections.Num());

		FPowerIKBoneLimit* JointLimit = const_cast<FPowerIKBoneLimit*>(JointLimits.GetData());
		TArrayView<FPowerIKBoneLimit> JointLimitsArray(JointLimit, JointLimits.Num());

		Core.InitializeSolver(
			CharacterRoot,
			EffectorsArray,
			ExcludedBonesArray,
			BendDirectionsArray,
			JointLimitsArray);

	}else if (Context.State == EControlRigState::Update && Core.IsInitialized)
	{
		
		FPowerIKEffector* Effector = const_cast<FPowerIKEffector*>(Effectors.GetData());
		TArrayView<FPowerIKEffector> EffectorsArray(Effector, Effectors.Num());

		// update effector transforms
		Core.UpdateEffectorBoneTransformsFromControlRig(EffectorsArray, Hierarchy);
		Core.UpdateEffectorTransforms(EffectorsArray, FTransform::Identity);
		
		// copy input pose and settings to solver
		Core.SetSolverBoneTransformsFromControlRig(Hierarchy);
		Core.SetSolverInputs(
			RootRotationMultiplier,
			MaxSquashIterations,
			MaxStretchIterations,
			MaxFinalIterations,
			AllowBoneTranslation,
			1.0f, //SmoothingMaxSpeedMultiplier
			1.0f, //SmoothingMaxDistanceMultiplier
			CenterOfGravityConstraint,
			Inertia,
			EffectorsArray);

		// run the solver
		Core.Solver->Solve(Context.DeltaTime, SolverAlpha);

		// copy the solver results
		Core.CopySolverOutputToControlRig(Hierarchy);
	}
}


