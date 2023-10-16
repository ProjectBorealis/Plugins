#pragma once

#include "CoreMinimal.h"
#include "Engine/SkeletalMesh.h"
#include "BlastAsset.h"
#include "BlastAssetImportData.h"
#include "BlastMaterial.h"
#include "BlastMesh.generated.h"

class USkeletalMesh;
class UBlastAsset;
class UPhysicsAsset;
class USkeleton;
struct FRawMesh;



USTRUCT()
struct BLAST_API FBlastStressProperties
{
	GENERATED_USTRUCT_BODY()

	// Is Stress solver enabled? If set to 'true' every frame stress will be calculated and overstressed bonds would be broken.
	UPROPERTY(EditAnywhere, Category = "Blast")
	bool							bCalculateStress = false;

	// Below this compression pressure no damage is done to bonds.  Also used as the default for shear and tension if they are set to < 0.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							CompressionElasticLimit = 1.0f;

	// Above this compression pressure the bond is immediately broken.  Also used as the default for shear and tension if they are set to < 0.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							CompressionFatalLimit = 2.0f;

	// Below this tension pressure no damage is done to bonds.  Use a negative value to fall back on compression limit.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							TensionElasticLimit = -1.0f;

	// Above this tension pressure the bond is immediately broken.  Use a negative value to fall back on compression limit.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							TensionFatalLimit = -1.0f;

	// Below this shear pressure no damage is done to bonds.  Use a negative value to fall back on compression limit.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							ShearElasticLimit = -1.0f;

	// Above this shear pressure the bond is immediately broken.  Use a negative value to fall back on compression limit.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							ShearFatalLimit = -1.0f;

	// This value is linearly connected with amount of time spent in stress solver every frame. The more iterations the better quality of stress propagation.
	// It is recommended to tune this value first to setup how much frame time can be spent on stress solving and then tune quality with GraphReductionLevel.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress", UIMin = 1, ClampMin = 1, UIMax = 50, ClampMax = 50))
	uint32							MaxSolverIterationsPerFrame = 25;

	// Determines how much smaller is stress graph in compare with support graph. The resulting graph will be roughly 2 ^ GraphReductionLevel times smaller than the original.
	// The smaller stress graph is, the better stress propagation quality will be achieved for the same amount of BondIterationsPerFrame. But the resulting damage would be less detailed, more rough (chunks bigger).
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress", UIMin = 0, ClampMin = 0))
	uint32							GraphReductionLevel = 3;

	// Impulse to apply after splitting as the result of bonds broken by stress solver. Velocity based.
	// Can be used in case the result of stress solver damage is not visibly noticeable because all chunks stay still and the whole construction remains as if it wasn't broken.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	float							SplitImpulseStrength = 0.f;

	// Apply/Pass impact impulses to stress graph. It is an alternative to impact damage, so it is not recommended to use them both turned on. Use stress-based impact damage if you can afford it in terms of computational cost.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bCalculateStress"))
	bool							bApplyImpactImpulses = false;

	// Impulse multiplier if it's passed into stress solver.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bApplyImpactImpulses"))
	float							ImpactImpulseToStressImpulseFactor = 0.01f;
};

USTRUCT()
struct BLAST_API FBlastActorDesc
{
	GENERATED_BODY()

	/**
	 * The default bond health if BondHealth is empty.
	 *
	 * Bond health is multiplied by the material health.
	 * Linear damage subtracts from health, destroying the bond when health reaches 0.
	 * Stress damage does not apply damage until the health threshold is exceeded.
	 */
	float DefaultBondHealth = 1.0f;

	/** Array of health for each bond. */
	TArray<float> BondHealths;

	/** The default support chunk health. */
	float DefaultSupportChunkHealth = 1.0f;

	/**
	 * Health for each support chunk.
	 *
	 * Every descendent of a support chunk will have its health initialized to its ancestor support chunk's health.
	 */
	TArray<float> SupportChunkHealths;
};

USTRUCT()
struct FBlastDebrisFilter
{
	GENERATED_USTRUCT_BODY()

	/**
	Use DebrisDepth as debris condition
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (InlineEditConditionToggle))
	uint32 bUseDebrisDepth : 1;

	/**
	The hierarchy depth at which chunks are considered to be "debris".
	Root chunk has depth 0.
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (EditCondition = "bUseDebrisDepth"))
	uint32 DebrisDepth;

	/**
	Use DebrisMaxSeparation as debris condition
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (InlineEditConditionToggle))
	uint32 bUseDebrisMaxSeparation : 1;

	/**
	Chunks are considered to be "debris" if they are separated from their origin by a distance
	greater than maxSeparation.
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (EditCondition = "bUseDebrisMaxSeparation", ClampMin = "0", UIMin = "0"))
	float DebrisMaxSeparation;

	/**
	Use DebrisMaxSize as debris condition
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (InlineEditConditionToggle))
	uint32 bUseDebrisMaxSize : 1;

	/**
	Chunks are considered to be "debris" if its bounding box max size is smaller then DebrisMaxSize
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (EditCondition = "bUseDebrisMaxSize", ClampMin = "0", UIMin = "0"))
	float DebrisMaxSize;

	/**
	Use ValidBounds as debris condition
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (InlineEditConditionToggle))
	uint32 bUseValidBounds : 1;

	/**
	Chunks are considered to be "debris" if they leave this box.
	The box translates with the blast actor's initial position, but does not
	rotate or scale.
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (EditCondition = "bUseValidBounds"))
	FBox ValidBounds;

	/**
	"Debris chunks" will be destroyed after a time (in seconds)
	separated from non-debris chunks.  The actual lifetime is randomly chosen between these
	two DebrisLifetime.  To disable lifetime, reset both values to default (0)
	If DebrisLifetimeMax < DebrisLifetimeMin, the mean of the two is used.
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (ClampMin = "0", UIMin = "0"))
	float DebrisLifetimeMin;

	UPROPERTY(EditAnywhere, Category = BlastDebrisFilter, meta = (ClampMin = "0", UIMin = "0"))
	float DebrisLifetimeMax;

	FBlastDebrisFilter()
		: bUseDebrisDepth(false)
		, DebrisDepth(1)
		, bUseDebrisMaxSeparation(false)
		, DebrisMaxSeparation(1000.0f)
		, bUseDebrisMaxSize(false)
		, DebrisMaxSize(1.f)
		, bUseValidBounds(false)
		, ValidBounds(FVector(-500000.0f), FVector(500000.0f))
		, DebrisLifetimeMin(0.f)
		, DebrisLifetimeMax(0.f)
	{
	}
};

/** Properties that pertain to chunk debris - level settings. */
USTRUCT()
struct FBlastDebrisProperties
{
	GENERATED_USTRUCT_BODY()

	/**
	Each DebrisFilter in array will be applied to chunks.
	If some chunks matches all conditions of a filter it will be marked as a "debris"
	and destroyed after specified lifetime.
	To disable debris processing clear DebrisFilters array
	*/
	UPROPERTY(EditAnywhere, Category = BlastDebrisProperties)
	TArray <FBlastDebrisFilter> DebrisFilters;
};

USTRUCT()
struct BLAST_API FBlastImpactDamageAdvancedProperties
{
	GENERATED_USTRUCT_BODY()

	FBlastImpactDamageAdvancedProperties() :
		bUseShearDamage(false),
		bVelocityBased(true),
		bSelfCollision(false),
		MinDamageThreshold(0.1f),
		MaxDamageThreshold(1.0f),
		DamageFalloffRadiusFactor(2.f),
		KinematicsMaxContactImpulse(-1)
	{}

	// Use shear damage program (otherwise simple radial damage is used).
	UPROPERTY(EditAnywhere, Category = "Blast")
	bool							bUseShearDamage;

	// If 'true' masses will be ignored and impact impulse would be velocity based. 
	// If 'false' impulse will be mass * velocity, so masses of the objects have great influence on the impulses.
	UPROPERTY(EditAnywhere, Category = "Blast")
	bool							bVelocityBased;

	// If set 'true' own chunks could damage each other. Otherwise self-collision events are filtered out.
	UPROPERTY(EditAnywhere, Category = "Blast")
	bool							bSelfCollision;

	// Minimum damage fraction threshold to be applied. Range [0, 1]. For example 0.1 filters all damage below 10% of health. It is recommended to leave 
	// some small value to filter out small impact damage events.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (UIMin = 0, ClampMin = 0, ClampMax = 1, UIMax = 1))
	float							MinDamageThreshold;

	// Maximum damage fraction threshold to be applied. Range [0, 1]. For example 0.8 won't allow more then 80% of health damage to be applied.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (UIMin = 0, ClampMin = 0, ClampMax = 1, UIMax = 1))
	float							MaxDamageThreshold;

	// Damage attenuation radius factor.   See the MaxDamageRadius description.  Given a radius R calculated for full damage, 
	// damage may be applied outside this radius up to R * DamageFalloffRadiusFactor.  The damage applied in this "falloff zone" is linearly 
	// attenuated down to zero at the outer radius.  The default value should always look fine, tweak in case you know what you want.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (UIMin = 1, ClampMin = 1, ClampMax = 32, UIMax = 32))
	float							DamageFalloffRadiusFactor;

	// Max contact impulse on kinematic (say bound to world) actors. The idea if for example you have a glass which is hit by 
	// something heavy and it is shattered with impact damage setup above. You do not want heavy object to repulse back, but to 
	// keep it flying through instead. To do that you can limit max contact impulse. The problem though if you set it too low the object can get 
	// through even if it wasn't fractured. If you use non-velocity based (bVelocityBased is 'false') set it somewhere around Hardness * Health, because that
	// is the approximate amount of impulse required to break destructible. Also take a consideration of a mass of objects you hit with versus mass of dynamic chunks 
	// that would be spawned on that place, it would make more sense for chunks to have much lower mass.
	// For velocity based impact damage it's trickier and vastly different masses can still lead to artifacts, so use carefully. Works only on 
	// kinematic actors, so use it only if 'bSupportedByWorld' set to 'true'.
	// Default is '-1' where maxContactImpulse is not overwritten and PhysX by default sets it to infinity (max float value).
	UPROPERTY(EditAnywhere, Category = "Blast")
	float							KinematicsMaxContactImpulse;
};


USTRUCT()
struct BLAST_API FBlastImpactDamageProperties
{
	GENERATED_USTRUCT_BODY()

	FBlastImpactDamageProperties() :
		bEnabled(true),
		Hardness(10.0f),
		MaxDamageRadius(200.f),
		PhysicalImpulseFactor(0.05f)
	{}

	// Whether or not impact damage is enabled. If 'true' then all collisions from OnComponentHit will be processed and impact impulse will be applied as damage. 
	// Note: You must also set 'Simulation Generates Hit Events' to 'true' in order to capture physics collision events. 
	UPROPERTY(EditAnywhere, Category = "Blast")
	bool								bEnabled;

	// Hardness of material for impact damage scenario. Damage = Impulse / Hardness.  This damage is capped by the material's health.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bEnabled"))
	float								Hardness;

	// The maximum radius to which full damage is applied.  The damage radius is given by MaxDamageRadius multiplied by the ratio of damage to material health.
	// Note: Damage can extend beyond this radius by an amount determined by AdvancedSettings.DamageFalloffRadiusFactor.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bEnabled"))
	float								MaxDamageRadius;

	// When damage leads to splitting and new chunks being created, impact impulse can be applied on a new actors.  It is not physically 
	// correct by any means and will look more like a small explosion, but can be a useful effect.  ApplyRadialImpulse will be 
	// called with damage radius (<= MaxDamageRadius). Set it to >0 to apply impact impulse (1.0 will apply the same impulse).
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bEnabled"))
	float								PhysicalImpulseFactor;

	// In most cases you don't need to tweak advanced settings, start with basic first.
	UPROPERTY(EditAnywhere, Category = "Blast", meta = (EditCondition = "bEnabled"))
	FBlastImpactDamageAdvancedProperties AdvancedSettings;
};


USTRUCT()
struct BLAST_API FBlastCookedChunkData
{
	GENERATED_BODY();

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FGuid	SourceBodySetupGUID;
#endif

	UPROPERTY(Instanced)
	TObjectPtr<class UBodySetup> CookedBodySetup;

	FBlastCookedChunkData() : CookedBodySetup(nullptr) {}

	void PopulateBodySetup(class UBodySetup* NewBodySetup) const;
	void AppendToBodySetup(class UBodySetup* NewBodySetup) const;
private:

	//Store these separately since the FKConvexElem class clears them on assignment so array resizes can clear them
#if BLAST_USE_PHYSX
	typedef TArray<physx::PxConvexMesh*, TInlineAllocator<32>> ConvexMeshTempList;
#else
	typedef TArray<TSharedPtr<Chaos::FConvex>, TInlineAllocator<32>> ConvexMeshTempList;
#endif
	void UpdateAfterShapesAdded(class UBodySetup* NewBodySetup, ConvexMeshTempList& ConvexMeshes) const;
};

/*
	This composite class represents everything required for the "Mesh" part of the Blast assets.

	Asset points back to the paired BlastAsset and must match the provided skeletal mesh and physics asset.
*/
UCLASS()
class BLAST_API UBlastMesh : public UBlastAsset
{
	GENERATED_UCLASS_BODY()
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(Category = ImportSettings, VisibleAnywhere, Instanced)
	TObjectPtr<class UBlastAssetImportData> AssetImportData;

	UPROPERTY()
	FBlastFractureToolData FractureToolData;
#endif

	UPROPERTY(Instanced)
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(Instanced)
	TObjectPtr<USkeleton> Skeleton;

	/* The physics asset to use for this blast mesh */
	UPROPERTY(Instanced)
	TObjectPtr<UPhysicsAsset> PhysicsAsset;

	// Blast material
	UPROPERTY(EditAnywhere, Category = "Blast")
	FBlastMaterial			BlastMaterial;

	// Impact damage properties
	UPROPERTY(EditAnywhere, Category = "Blast")
	FBlastImpactDamageProperties ImpactDamageProperties;

	// Stress properties
	UPROPERTY(EditAnywhere, Category = "Blast")
	FBlastStressProperties	StressProperties;

	// Debris properties
	UPROPERTY(EditAnywhere, Category = "Blast")
	FBlastDebrisProperties  DebrisProperties;

	//Store this in the the asset so that if we change the logic in GetDefaultChunkBoneNameFromIndex, existing assets still work
	UPROPERTY()
	TArray<FName>		ChunkIndexToBoneName;

	UPROPERTY(Transient)
	TArray<uint32>		ChunkIndexToBoneIndex;

	/* Can this BlastMesh be used with it's current data? That means it needs a valid Asset, a Mesh that matches and a physics asset. */
	bool				IsValidBlastMesh();

	/* If not empty, the chunks have been saved out-of-order and should be unscrambled with this map before processing. */
	TArray<int32>		ChunkIndexMap;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;

	const FTransform& GetComponentSpaceInitialBoneTransform(int32 BoneIndex) const
	{
		return ComponentSpaceInitialBoneTransforms[BoneIndex];
	}

	virtual void PostLoad() override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	void RebuildIndexToBoneNameMap();

#if WITH_EDITOR
	void RebuildCookedBodySetupsIfRequired(bool bForceRebuild = false);
	void GetRenderMesh(int32 LODIndex, TArray<FRawMesh>& RawMeshes);
	virtual void CopyFromLoadedAsset(const NvBlastAsset* AssetToCopy, const FGuid& NewAssetGUID = FGuid::NewGuid()) override;
#endif

	const TArray<FName>& GetChunkIndexToBoneName();

	const TArray<FBlastCookedChunkData>& GetCookedChunkData();
	const TArray<FBlastCookedChunkData>& GetCookedChunkData_AssumeUpToDate() const;

	static const FString ChunkPrefix;
	static FName GetDefaultChunkBoneNameFromIndex(int32 ChunkIndex);
protected:

	//The cooking the bakes the transform into the data and we need all our bodies to be relative to the component root since that's where our instances are oriented
	UPROPERTY(DuplicateTransient)
	TArray<FBlastCookedChunkData>		CookedChunkData;

	//Cache this since GetComposedRefPoseMatrix is not available in non-editor builds
	UPROPERTY()
	TArray<FTransform>	ComponentSpaceInitialBoneTransforms;
};
