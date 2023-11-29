#include "BlastMeshComponent.h"

#include "TimerManager.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "NvBlast.h"
#include "NvBlastTypes.h"
#include "NvBlastExtDamageShaders.h"
#include "NvBlastExtStressSolver.h"
#include "Math/UnrealMathUtility.h"
#include "BlastGlobals.h"
#include "NvBlastGlobals.h"
#include "SkeletalRenderPublic.h"
#include "BlastScratch.h"
#include "DrawDebugHelpers.h"
#include "Stats/Stats.h"
#include "ComponentReregisterContext.h"
#include "BlastModule.h"
#include "BlastDamagePrograms.h"
#include "BlastGlueVolume.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "EngineUtils.h"
#include "BlastExtendedSupport.h"
#include "Rendering/SkeletalMeshRenderData.h"


#if BLAST_USE_PHYSX
#else
#include "Chaos/ChaosScene.h"
#include "Physics/Experimental/ChaosScopedSceneLock.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#endif

#define LOCTEXT_NAMESPACE "Blast"

DECLARE_CYCLE_STAT(TEXT("Calc BlastMeshComponent Bounds"), STAT_BlastMeshComponent_CalcBounds, STATGROUP_Blast);
DECLARE_CYCLE_STAT(TEXT("Sync Chunks and Bodies"), STAT_BlastMeshComponent_SyncChunksAndBodies, STATGROUP_Blast);
DECLARE_CYCLE_STAT(TEXT("Sync Chunks and Bodies (Non-rendering children update)"),
                   STAT_BlastMeshComponent_SyncChunksAndBodiesChildren, STATGROUP_Blast);

UBlastMeshComponent::UBlastMeshComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	BlastMesh(nullptr),
	ModifiedAssetOwned(nullptr),
	ModifiedAsset(nullptr),
	OwningSupportStructure(nullptr),
	OwningSupportStructureIndex(INDEX_NONE),
	bIsInitiallyKinematic(false),
	bSupportedByWorld(false),
	bIgnoreDamage(false),
	SmallChunkRadius(20.f),
	bCrumbleInmostChunks(false),
	bShouldAllChildrenChunksBeSmallChunks(false),
	bBindOnHitDelegate(false),
	bOverride_BlastMaterial(false),
	bOverride_StressProperties(false),
	bOverride_DebrisProperties(false),
	BlastActorsBeginLive(0),
	BlastActorsEndLive(0),
	StressSolver(nullptr),
	DebrisCount(0),
	bAddedOrRemovedActorSinceLastRefresh(false),
	bChunkVisibilityChanged(false),
	bHasBeenFractured(false),
	BlastProxy(nullptr)
{
	// NOTE: Do we want this component to tick5?

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bTickInEditor = true;

	// We want to tick the pose since we need to update our bone positions
	VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// Save some performance by disabling that
	bPerBoneMotionBlur = false;

	// Use URO
	bEnableUpdateRateOptimizations = true;

	bWantsInitializeComponent = true;

	static FName CollisionProfileName(TEXT("Destructible"));
	BodyInstance.SetCollisionProfileName(CollisionProfileName);
	DynamicChunkBodyInstance.SetCollisionProfileName(CollisionProfileName);
	DynamicChunkBodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Ignore);

	SetActive(true);
	bMultiBodyOverlap = true;

	// Make sure the PrimitiveComponent BodyInstance shows as simulating physics
	BodyInstance.bSimulatePhysics = true;
	//Turn on by default to enable impact damage, etc.
	BodyInstance.bNotifyRigidBodyCollision = true;
	DynamicChunkBodyInstance.bNotifyRigidBodyCollision = true;
	bOverride_DebrisProperties = true;
	FBlastDebrisFilter DebrisFilter;
	DebrisFilter.bUseDebrisMaxSeparation = true;
	DebrisFilter.DebrisMaxSeparation = 200000.f;
	DebrisProperties.DebrisFilters.Add(DebrisFilter);
}

void UBlastMeshComponent::SetModifiedAsset(UBlastAsset* newModifiedAsset)
{
	if (ModifiedAsset != newModifiedAsset)
	{
		ModifiedAsset = newModifiedAsset;
		ModifiedAssetOwned = (newModifiedAsset && newModifiedAsset->GetOuter() == this) ? newModifiedAsset : nullptr;
		RecreatePhysicsState();
#if WITH_EDITOR
		if (IsWorldSupportDirty())
		{
			UBlastGlueWorldTag::SetDirty(GetWorld());
		}
		if (IsExtendedSupportDirty())
		{
			UBlastGlueWorldTag::SetExtendedSupportDirty(GetWorld());
		}
#endif
	}
	if (ModifiedAsset)
	{
		ConditionalUpdateComponentToWorld();
		ModifiedAssetComponentToWorldAtBake = GetComponentTransform();
	}
	else
	{
		ModifiedAssetComponentToWorldAtBake = FTransform();
	}
}

#if WITH_EDITOR

void UBlastMeshComponent::PreEditChange(FProperty* PropertyThatWillChange)
{
	if (PropertyThatWillChange &&
		(PropertyThatWillChange->GetFName() == GET_MEMBER_NAME_CHECKED(UBlastMeshComponent, StressDebugMode)
			|| PropertyThatWillChange->GetFName() == GET_MEMBER_NAME_CHECKED(UBlastMeshComponent, bDrawSupportGraph)
			|| PropertyThatWillChange->GetFName() == GET_MEMBER_NAME_CHECKED(UBlastMeshComponent, bDrawChunkCentroids)))
	{
		//Don't pass this on, otherwise the component gets re-registered which resets the destruction state	
	}
	else
	{
		Super::PreEditChange(PropertyThatWillChange);
	}
}


void UBlastMeshComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.MemberProperty != nullptr)
		                           ? PropertyChangedEvent.MemberProperty->GetFName()
		                           : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UBlastMeshComponent, BlastMesh))
	{
		UBlastMesh* NewMesh = BlastMesh;
		BlastMesh = nullptr;

		//This checks BlastMesh != NewMesh before doing anything and it's already set
		SetBlastMesh(NewMesh);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UBlastMeshComponent, bSupportedByWorld))
	{
		SetModifiedAsset(nullptr);
		SetOwningSuppportStructure(nullptr, INDEX_NONE);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UBlastMeshComponent, BodyInstance))
	{
		RefreshDynamicChunkBodyInstanceFromBodyInstance();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UBlastMeshComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	SetModifiedAsset(nullptr);
	MarkDirtyOwningSuppportStructure();
}

void UBlastMeshComponent::CheckForErrors()
{
	Super::CheckForErrors();

	if (BlastMesh == nullptr)
	{
		FMessageLog("MapCheck").Error()
		                       ->AddToken(FUObjectToken::Create(this))
		                       ->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_InvalidBlastMesh",
		                                                             "There is no Blast mesh assigned to this component")));
	}

	if (bSupportedByWorld)
	{
		TArray<ABlastGlueVolume*> GlueVolumes;
		UBlastGlueWorldTag* WorldTag = UBlastGlueWorldTag::GetForWorld(GetWorld());
		if (WorldTag)
		{
			for (ABlastGlueVolume* GV : WorldTag->GlueVolumes)
			{
				if (GV->bEnabled)
				{
					GlueVolumes.Add(GV);
				}
			}
		}

		for (TActorIterator<ABlastGlueVolume> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->bEnabled)
			{
				GlueVolumes.Add(*ActorItr);
			}
		}

		TArray<uint32> OverlappingChunks;
		TArray<FVector> GlueVectors;
		TSet<ABlastGlueVolume*> OverlappingVolumes;
		if (!GetSupportChunksInVolumes(GlueVolumes, OverlappingChunks, GlueVectors, OverlappingVolumes, false))
		{
			FMessageLog("MapCheck").Warning()
			                       ->AddToken(FUObjectToken::Create(this))
			                       ->AddToken(FTextToken::Create(LOCTEXT(
				                       "MapCheck_Message_NoGlueVolumes",
				                       "BlastMeshComponent is marked bSupportedByWorld but is not inside a ABlastGlueVolume")));
		}
	}
}

bool UBlastMeshComponent::CanEditChange(const FProperty* InProperty) const
{
	bool bResult = Super::CanEditChange(InProperty);
	if (bResult)
	{
		const FProperty* OwnerProp = InProperty->GetOwnerProperty();
		if (OwningSupportStructure != nullptr && OwnerProp->HasMetaData("CantUseWithExtendedSupport"))
		{
			bResult = false;
		}
	}
	return bResult;
}

#endif


void UBlastMeshComponent::InitBlastFamily()
{
	check(!BlastFamily.IsValid());

	UBlastAsset* BlastAsset = GetBlastAsset();
	if (BlastMesh == nullptr)
	{
		UE_LOG(LogBlast, Error, TEXT("Trying to init NvBlastFamily, but no BlastMesh specified."));
		return;
	}

	auto LLBlastAsset = BlastAsset->GetLoadedAsset();
	if (LLBlastAsset == nullptr)
	{
		UE_LOG(LogBlast, Error, TEXT("Trying to init NvBlastFamily, BlastMesh is invalid"));
		return;
	}

	UPhysicsAsset* PhysicsAsset = BlastMesh->PhysicsAsset;
	// hide all chunks at first
	uint32 ChunkCount = BlastAsset->GetChunkCount();
	for (uint32 ChunkIndex = 0; ChunkIndex < ChunkCount; ChunkIndex++)
	{
		SetChunkVisible(ChunkIndex, false);
	}
	DebrisCount = 0;

	void* FamilyMem = NVBLAST_ALLOC(NvBlastAssetGetFamilyMemorySize(LLBlastAsset, Nv::Blast::logLL));
	// Create a NvBlastFamily and wrap it in a shared ptr with a custom deleter so it gets released when we're done with it.
	BlastFamily = TSharedPtr<NvBlastFamily>(NvBlastAssetCreateFamily(FamilyMem, LLBlastAsset, Nv::Blast::logLL),
	                                        [FamilyMem](NvBlastFamily* family)
	                                        {
		                                        NVBLAST_FREE((void*)FamilyMem);
	                                        }
	);

	uint32 MaxActorCount = NvBlastFamilyGetMaxActorCount(BlastFamily.Get(), Nv::Blast::logLL);
	BlastActors.SetNum(MaxActorCount);
	ActorBodySetups.Reset();
	//In some cases due to the editor duplicating objects this can be not empty so make sure it's zeroed out
	ActorBodySetups.SetNumZeroed(MaxActorCount);
	BlastActorsBeginLive = 0;
	BlastActorsEndLive = 0;

	TArray<uint8> Scratch;
	Scratch.SetNumUninitialized(
		NvBlastFamilyGetRequiredScratchForCreateFirstActor(BlastFamily.Get(), Nv::Blast::logLL));

	NvBlastActorDesc ActorDesc;
	ActorDesc.uniformInitialBondHealth = 1.0f;
	ActorDesc.uniformInitialLowerSupportChunkHealth = 1.0f;
	ActorDesc.initialBondHealths = nullptr;
	ActorDesc.initialSupportChunkHealths = nullptr;

	DamageAccelerator = NvBlastExtDamageAcceleratorCreate(LLBlastAsset, 3);

#if WITH_EDITOR
	BlastMesh->RebuildCookedBodySetupsIfRequired();
#endif

	NvBlastActor* Actor = NvBlastFamilyCreateFirstActor(BlastFamily.Get(), &ActorDesc, Scratch.GetData(),
	                                                    Nv::Blast::logLL);

	// Create stress solver if enabled (right after actor created, but before 'StressSolver->notifyActorCreated()' call)
	if (GetUsedStressProperties().bCalculateStress)
	{
		StressSolver = Nv::Blast::ExtStressSolver::create(*BlastFamily.Get());
		const float density = 0.000001f; // 1e-6 kg / cm3
		// TODO: set each node according to its mass, volume and local transform with setNodeInfo
		StressSolver->setAllNodesInfoFromLL(density);
	}

	SetupNewBlastActor(Actor, FBlastActorCreateInfo(GetComponentTransform()), nullptr, nullptr, FName(), true);

	bAddedOrRemovedActorSinceLastRefresh = true;
	bHasValidBoneTransform = false;

	UpdateFractureBufferSize();
}

void UBlastMeshComponent::UninitBlastFamily()
{
	if (!BlastFamily.IsValid())
	{
		return;
	}

	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		FActorData& ActorData = BlastActors[ActorIndex];
		if (ActorData.BodyInstance)
		{
			ActorData.BodyInstance->TermBody();
			delete ActorData.BodyInstance;
		}
		if (ActorData.TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(ActorData.TimerHandle);
			ActorData.TimerHandle.Invalidate();
		}
		ActorData = FActorData();
	}
	DebrisCount = 0;

	if (StressSolver)
	{
		StressSolver->release();
		StressSolver = nullptr;
	}

	if (DamageAccelerator)
	{
		DamageAccelerator->release();
		DamageAccelerator = nullptr;
	}

	BlastActors.Reset();
	ActorBodySetups.Reset();
	BlastFamily.Reset();

	BlastActorsBeginLive = 0;
	BlastActorsEndLive = 0;

	ShowRootChunks();
}

void UBlastMeshComponent::ShowRootChunks()
{
	UBlastAsset* BlastAsset = GetBlastAsset();
	if (BlastAsset != nullptr)
	{
		// hide all chunks at first
		uint32 ChunkCount = BlastAsset->GetChunkCount();
		for (uint32 i = 0; i < ChunkCount; i++)
		{
			bool bIsRootChunk = BlastAsset->GetRootChunks().Contains(i);
			SetChunkVisible(i, bIsRootChunk);
		}
	}

	bAddedOrRemovedActorSinceLastRefresh = true;
	RefreshBoneTransforms(nullptr);
}

void UBlastMeshComponent::ShowActorsVisibleChunks(uint32 actorIndex)
{
	check(BlastActors.IsValidIndex(actorIndex));

	for (const auto& ChunkData : BlastActors[actorIndex].Chunks)
	{
		SetChunkVisible(ChunkData.ChunkIndex, true);
	}
}

void UBlastMeshComponent::HideActorsVisibleChunks(uint32 actorIndex)
{
	check(BlastActors.IsValidIndex(actorIndex));

	for (const auto& ChunkData : BlastActors[actorIndex].Chunks)
	{
		SetChunkVisible(ChunkData.ChunkIndex, false);
	}
}

void UBlastMeshComponent::SetChunkVisible(int32 ChunkIndex, bool bInVisible)
{
	if (!ChunkVisibility.IsValidIndex(ChunkIndex))
	{
		UE_LOG(LogBlast, Warning, TEXT("Trying to set chunk %d/%d visible."), ChunkIndex, ChunkVisibility.Num());
		return;
	}

	if (bInVisible != ChunkVisibility[ChunkIndex])
	{
		ChunkVisibility[ChunkIndex] = bInVisible;
		bChunkVisibilityChanged = true;
		MarkRenderDynamicDataDirty();
	}
}

bool UBlastMeshComponent::IsChunkVisible(int32 ChunkIndex) const
{
	//The number of chunks != the number of bones if there are non-weighted bones for pivots, etc.
	if (!ChunkVisibility.IsValidIndex(ChunkIndex))
	{
		UE_LOG(LogBlast, Warning, TEXT("Trying to get chunk %d/%d visibility."), ChunkIndex, ChunkVisibility.Num());
		return false;
	}
	return ChunkVisibility[ChunkIndex];
}

bool UBlastMeshComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep,
                                            FHitResult* Hit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{
	if (bHasBeenFractured)
	{
		return false;
	}
	return Super::MoveComponentImpl(Delta, NewRotation, bSweep, Hit, MoveFlags, Teleport);
}

void UBlastMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR
	if (IsWorldSupportDirty())
	{
		SetModifiedAsset(nullptr);
		UBlastGlueWorldTag::SetDirty(GetWorld());
	}
	if (IsExtendedSupportDirty())
	{
		SetOwningSuppportStructure(OwningSupportStructure, INDEX_NONE);
		UBlastGlueWorldTag::SetExtendedSupportDirty(GetWorld());
	}
#endif

	UWorld* World = GetWorld();
	if (World)
	{
		if (World->IsGameWorld())
		{
			if (StressSolver)
			{
				TickStressSolver();
			}

			UpdateDebris();
		}

#if WITH_EDITOR
		//Using the normal debug drawing interface causes the lines to queue up forever when we aren't being rendered, so instead we pass them to the SceneProxy
		bool bHadDebugLinesBefore = PendingDebugLines.Num() > 0 || PendingDebugPoints.Num() > 0;
		PendingDebugLines.Reset();
		PendingDebugPoints.Reset();

		if (World->GetNetMode() != NM_DedicatedServer)
		{
			if (bDrawSupportGraph)
			{
				DrawDebugSupportGraph();
			}

			if (bDrawChunkCentroids)
			{
				DrawDebugChunkCentroids();
			}

			DrawDebugStressGraph();
		}

		if (!PendingDebugLines.IsEmpty() || !PendingDebugPoints.IsEmpty() || bHadDebugLinesBefore)
		{
			//If we have none, now but there were ones before we need to send one final update to turn them off
			MarkRenderDynamicDataDirty();
		}
#endif
	}

	if (IsRegistered() && IsSimulatingPhysics() && RigidBodyIsAwake())
	{
		SyncComponentToRBPhysics();
	}
}

void UBlastMeshComponent::PostEditImport()
{
	Super::PostEditImport();

	//Clear the glue data when we are duplicated
	SetModifiedAsset(nullptr);
	SetOwningSuppportStructure(nullptr, INDEX_NONE);
}

UBodySetup* UBlastMeshComponent::GetBodySetup()
{
	// PB - (Omar) Original Code had this comment below, it will return nullptr in the case of OnCreatePhysicsState
	// but we need to return the body setup later for Recast's geometry export.
	// (Original Comment): Returning null here prevents UPrimitiveComponent::OnCreatePhysicsState from creating a default state
	if (GetSkinnedAsset())
	{
		if (BlastMesh && ActorBodySetups.Num())
		{
			return ActorBodySetups[0];
		}
	}
	return nullptr;
}


FBodyInstance* UBlastMeshComponent::GetActorBodyInstance(int32 ActorIndex) const
{
	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		return OwningSupportStructure->GetExtendedSupportMeshComponent()->GetActorBodyInstance(ActorIndex);
	}

	if (ActorIndex == INDEX_NONE)
	{
		ActorIndex = 0;
	}
	return (BlastActors.IsValidIndex(ActorIndex) && BlastActors[ActorIndex].BodyInstance)
		       ? BlastActors[ActorIndex].BodyInstance
		       : nullptr;
}

FTransform UBlastMeshComponent::GetActorWorldTransform(FName ActorName) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorName);
	if (ActorBodyInstance)
	{
		FTransform Ret = ActorBodyInstance->GetUnrealWorldTransform();
		Ret.SetScale3D(ActorBodyInstance->Scale3D);
		return Ret;
	}
	return GetComponentTransform();
}

void UBlastMeshComponent::RestoreActorBodyInstanceObjectType(FName ActorName) const
{
	const int32 ActorIndex = ActorNameToActorIndex(ActorName);
	if (BlastActors.IsValidIndex(ActorIndex))
	{
		if (BlastActors[ActorIndex].bIsSmallChunk)
		{
			BlastActors[ActorIndex].BodyInstance->SetObjectType(SmallChunkBodyInstance.GetObjectType());
		}
		else
		{
			BlastActors[ActorIndex].BodyInstance->SetObjectType(DynamicChunkBodyInstance.GetObjectType());
		}
	}
}

FTransform UBlastMeshComponent::GetActorWorldTransform(uint32 ActorIndex) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorIndex);
	if (ActorBodyInstance)
	{
		FTransform Ret = ActorBodyInstance->GetUnrealWorldTransform();
		Ret.SetScale3D(ActorBodyInstance->Scale3D);
		return Ret;
	}
	return GetComponentTransform();
}

FVector UBlastMeshComponent::GetActorCOMWorldPosition(FName ActorName) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorName);
	return ActorBodyInstance ? ActorBodyInstance->GetCOMPosition() : FVector::ZeroVector;
}

float UBlastMeshComponent::GetActorMass(FName ActorName) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorName);
	return ActorBodyInstance ? ActorBodyInstance->GetBodyMass() : 0.f;
}

FName UBlastMeshComponent::GetActorForChunk(int32 ChunkIndex) const
{
	return ActorIndexToActorName(GetActorIndexForChunk(ChunkIndex));
}


int32 UBlastMeshComponent::GetActorIndexForChunk(int32 ChunkIndex) const
{
	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		UBlastExtendedSupportMeshComponent* ExtSupport = OwningSupportStructure->GetExtendedSupportMeshComponent();
		return ExtSupport->GetActorIndexForChunk(
			ExtSupport->GetCombinedChunkIndex(OwningSupportStructureIndex, ChunkIndex));
	}

	NvBlastActor* BlastActor = NvBlastFamilyGetChunkActor(BlastFamily.Get(), ChunkIndex, Nv::Blast::logLL);
	if (BlastActor)
	{
		int32 ActorIndex = NvBlastActorGetIndex(BlastActor, Nv::Blast::logLL);
		return ActorIndex;
	}
	return INDEX_NONE;
}

FTransform UBlastMeshComponent::GetChunkWorldTransform(int32 ChunkIndex) const
{
	if (!BlastMesh || !BlastMesh->IsValidBlastMesh() || ChunkIndex < 0 || ChunkIndex >= (int32)BlastMesh->
		GetChunkCount())
	{
		return GetComponentTransform();
	}

	return GetBoneTransform(BlastMesh->ChunkIndexToBoneIndex[ChunkIndex]);
}

FTransform UBlastMeshComponent::GetChunkActorRelativeTransform(int32 ChunkIndex) const
{
	if (!BlastMesh || !BlastMesh->IsValidBlastMesh() || ChunkIndex < 0 || ChunkIndex >= (int32)BlastMesh->
		GetChunkCount())
	{
		return FTransform::Identity;
	}

	int32 BoneIndex = BlastMesh->ChunkIndexToBoneIndex[ChunkIndex];
	return BlastMesh->GetComponentSpaceInitialBoneTransform(BoneIndex);
}

FVector UBlastMeshComponent::GetChunkCenterWorldPosition(int32 ChunkIndex) const
{
	return GetChunkWorldBounds(ChunkIndex).Origin;
}

FBoxSphereBounds UBlastMeshComponent::GetChunkWorldBounds(int32 ChunkIndex) const
{
	int32 ActorIndex = GetActorIndexForChunk(ChunkIndex);
	if (ActorIndex == INDEX_NONE)
	{
		return FBox();
	}

	const auto& CookedData = BlastMesh->GetCookedChunkData_AssumeUpToDate()[ChunkIndex];
	//These AggGeom's are in componennt space, they are pre-transformed with chunk -> actor
	return CookedData.CookedBodySetup->AggGeom.CalcAABB(GetActorWorldTransform(ActorIndex));
}

FVector UBlastMeshComponent::GetChunkWorldAngularVelocityInRadians(int32 ChunkIndex) const
{
	if (ChunkToActorIndex.IsValidIndex(ChunkIndex))
	{
		FTransform ChunkToActor = GetChunkActorRelativeTransform(ChunkIndex);
		FBodyInstance* ActorBodyInstance = GetActorBodyInstance(GetActorIndexForChunk(ChunkIndex));
		FVector Velocity = ActorBodyInstance
			                   ? ActorBodyInstance->GetUnrealWorldAngularVelocityInRadians()
			                   : FVector::ZeroVector;
		return ChunkToActor.InverseTransformVector(Velocity);
	}
	return FVector::ZeroVector;
}

FVector UBlastMeshComponent::GetChunkWorldVelocity(int32 ChunkIndex) const
{
	if (ChunkToActorIndex.IsValidIndex(ChunkIndex))
	{
		FBodyInstance* ActorBodyInstance = GetActorBodyInstance(GetActorIndexForChunk(ChunkIndex));
		return ActorBodyInstance
			       ? ActorBodyInstance->GetUnrealWorldVelocityAtPoint(
				       GetChunkWorldTransform(ChunkIndex).GetTranslation())
			       : FVector::ZeroVector;
	}
	return FVector::ZeroVector;
}

void UBlastMeshComponent::SetDynamicChunkCollisionEnabled(ECollisionEnabled::Type NewType)
{
	if (DynamicChunkBodyInstance.GetCollisionEnabled() != NewType)
	{
		DynamicChunkBodyInstance.SetCollisionEnabled(NewType);

		EnsurePhysicsStateCreated();
		OnComponentCollisionSettingsChanged();
	}
}

void UBlastMeshComponent::SetDynamicChunkCollisionProfileName(FName InCollisionProfileName)
{
	ECollisionEnabled::Type OldCollisionEnabled = DynamicChunkBodyInstance.GetCollisionEnabled();
	DynamicChunkBodyInstance.SetCollisionProfileName(InCollisionProfileName);
	OnComponentCollisionSettingsChanged();

	ECollisionEnabled::Type NewCollisionEnabled = DynamicChunkBodyInstance.GetCollisionEnabled();

	if (OldCollisionEnabled != NewCollisionEnabled)
	{
		EnsurePhysicsStateCreated();
	}
}

FName UBlastMeshComponent::GetDynamicChunkCollisionProfileName() const
{
	return DynamicChunkBodyInstance.GetCollisionProfileName();
}

void UBlastMeshComponent::SetDynamicChunkCollisionObjectType(ECollisionChannel Channel)
{
	DynamicChunkBodyInstance.SetObjectType(Channel);
	//UPrimitiveComponent::SetCollisionObjectType does not call OnComponentCollisionSettingsChanged()
}

void UBlastMeshComponent::SetDynamicChunkCollisionResponseToChannel(ECollisionChannel Channel,
                                                                    ECollisionResponse NewResponse)
{
	DynamicChunkBodyInstance.SetResponseToChannel(Channel, NewResponse);
	OnComponentCollisionSettingsChanged();
}

void UBlastMeshComponent::SetDynamicChunkCollisionResponseToAllChannels(ECollisionResponse NewResponse)
{
	DynamicChunkBodyInstance.SetResponseToAllChannels(NewResponse);
	OnComponentCollisionSettingsChanged();
}

FBox UBlastMeshComponent::GetActorWorldBounds(FName ActorName) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorName);
	return ActorBodyInstance ? ActorBodyInstance->GetBodyBounds() : FBox(EForceInit::ForceInit);
}

FVector UBlastMeshComponent::GetActorWorldAngularVelocityInRadians(FName ActorName) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorName);
	return ActorBodyInstance ? ActorBodyInstance->GetUnrealWorldAngularVelocityInRadians() : FVector::ZeroVector;
}

FVector UBlastMeshComponent::GetActorWorldVelocity(FName ActorName) const
{
	FBodyInstance* ActorBodyInstance = GetActorBodyInstance(ActorName);
	return ActorBodyInstance ? ActorBodyInstance->GetUnrealWorldVelocity() : FVector::ZeroVector;
}


FBodyInstance* UBlastMeshComponent::GetBodyInstance(FName BoneName /*= NAME_None*/, bool bGetWelded /*= true*/,
                                                    int32 Index /*= INDEX_NONE*/) const
{
	return GetActorBodyInstance(BoneName);
}

FBoxSphereBounds UBlastMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const bool bInExtendedSupport = OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE;
	if (BlastFamily.IsValid() || bInExtendedSupport)
	{
		if (bCachedLocalBoundsUpToDate)
		{
			return CachedWorldOrLocalSpaceBounds.TransformBy(
				CachedWorldToLocalTransform * LocalToWorld.ToMatrixWithScale());
		}

		//Examine the existing bodies to see what we have
		FBox NewBox(ForceInit);

		if (bInExtendedSupport)
		{
			UBlastExtendedSupportMeshComponent* ExtSupport = OwningSupportStructure->GetExtendedSupportMeshComponent();
			NewBox = ExtSupport->GetWorldBoundsOfComponentChunks(OwningSupportStructureIndex);
		}
		else
		{
			FScopedSceneLock_Chaos Lock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Read);
			for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
			{
				UBodySetup* BodySetup = ActorBodySetups[ActorIndex];
				const FActorData& BlastActor = BlastActors[ActorIndex];
				if (BodySetup && BlastActor.BodyInstance)
				{
					FTransform BodyWorldTransform = BlastActor.BodyInstance->GetUnrealWorldTransform_AssumesLocked();
					BodyWorldTransform.SetScale3D(BlastActor.BodyInstance->Scale3D);
					FBox AABB = BodySetup->AggGeom.CalcAABB(BodyWorldTransform);
					NewBox += AABB;
				}
			}
		}

		FBoxSphereBounds NewBounds = NewBox;
		bCachedLocalBoundsUpToDate = true;
		CachedWorldOrLocalSpaceBounds = NewBounds;
		CachedWorldToLocalTransform = LocalToWorld.ToInverseMatrixWithScale();
		return NewBounds;
	}
	else
	{
		return USkinnedMeshComponent::CalcBounds(LocalToWorld);
	}
}

FTransform UBlastMeshComponent::GetSocketTransform(FName InSocketName,
                                                   ERelativeTransformSpace TransformSpace /*= RTS_World*/) const
{
	if (InSocketName != NAME_None)
	{
		FBodyInstance* ActorBodyInstance = GetActorBodyInstance(InSocketName);
		if (ActorBodyInstance)
		{
			FTransform WorldTransform = ActorBodyInstance->GetUnrealWorldTransform();
			switch (TransformSpace)
			{
			case RTS_World:
				return WorldTransform;
			case RTS_Actor:
				return GetOwner()
					       ? WorldTransform.GetRelativeTransform(GetOwner()->GetActorTransform())
					       : WorldTransform;
			case RTS_ParentBoneSpace:
			case RTS_Component:
				return WorldTransform.GetRelativeTransform(GetComponentTransform());
			default:
				checkNoEntry();
			}
		}
	}
	return Super::GetSocketTransform(InSocketName, TransformSpace);
}

bool UBlastMeshComponent::DoesSocketExist(FName InSocketName) const
{
	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		return OwningSupportStructure->GetExtendedSupportMeshComponent()->DoesSocketExist(InSocketName);
	}
	//This can cause harmless but spammy warnings like
	//"LogCharacter:Warning: GetMovementBaseTransform(): Invalid bone or socket 'Actor_4' for SkinnedMeshComponent"
	//if you fracture a chunk by walking on it since the actor is destroyed while processing the hit.
	//We could silence this by always returning true if ActorNameToActorIndex returns != INDEX_NONE even if the 
	//BlastActor pointer is null, but this violates the intent of this method. Seems like the movement code assumes sockets
	//can't be created or destroyed during gameplay. Not sure what to do. Maybe defer applying impact damage till later in the frame?
	const int32 ActorIndex = ActorNameToActorIndex(InSocketName);
	if (BlastActors.IsValidIndex(ActorIndex) && BlastActors[ActorIndex].BlastActor)
	{
		return true;
	}
	return Super::DoesSocketExist(InSocketName);
}

bool UBlastMeshComponent::HasAnySockets() const
{
	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		return OwningSupportStructure->GetExtendedSupportMeshComponent()->HasAnySockets();
	}

	return BlastActorsBeginLive != BlastActorsEndLive || Super::HasAnySockets();
}

void UBlastMeshComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		OwningSupportStructure->GetExtendedSupportMeshComponent()->QuerySupportedSockets(OutSockets);
		return;
	}

	Super::QuerySupportedSockets(OutSockets);

	//The actors have special socket names which are not in the skeletal mesh
	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		if (BlastActors[ActorIndex].BlastActor)
		{
			new(OutSockets)
				FComponentSocketDescription(ActorIndexToActorName(ActorIndex), EComponentSocketType::Socket);
		}
	}
}

void UBlastMeshComponent::OnCreatePhysicsState()
{
	RefreshDynamicChunkBodyInstanceFromBodyInstance();

	Super::OnCreatePhysicsState();

	if (BlastMesh == nullptr)
	{
		UE_LOG(LogBlast, Error, TEXT("Failed to initialize BlastMeshComponent - no asset."));
		return;
	}

	SetSkinnedAsset(BlastMesh->Mesh);

	InitBlastFamily();
}

void UBlastMeshComponent::OnDestroyPhysicsState()
{
	UninitBlastFamily();

	Super::OnDestroyPhysicsState();
}

bool UBlastMeshComponent::SyncChunksAndBodies()
{
	SCOPE_CYCLE_COUNTER(STAT_BlastMeshComponent_SyncChunksAndBodies);

	check(BlastMesh != nullptr);

	if (!GetWorld()->GetPhysicsScene())
	{
		//During cooking there is no physics scene, so nothing to sync
		return false;
	}
	bool bAnyBodiesChanged = false;

	TBitArray<> BonesTouched(false, GetEditableComponentSpaceTransforms().Num());

	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		UBlastExtendedSupportMeshComponent* ExtSupport = OwningSupportStructure->GetExtendedSupportMeshComponent();
		bAnyBodiesChanged = ExtSupport->PopulateComponentBoneTransforms(GetEditableComponentSpaceTransforms(),
		                                                                BonesTouched, OwningSupportStructureIndex);
	}
	else
	{
		FScopedSceneLock_Chaos Lock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Read);
		for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
		{
			FActorData& ActorData = BlastActors[ActorIndex];
			FBodyInstance* BodyInst = ActorData.BodyInstance;
			if (!BodyInst)
			{
				continue;
			}

			FTransform BodyWT = BodyInst->GetUnrealWorldTransform_AssumesLocked();
			BodyWT.SetScale3D(BodyInst->Scale3D);

			UpdateDebris(ActorIndex, BodyWT, &Lock);

			if (!BodyWT.Equals(ActorData.PreviousBodyWorldTransform))
			{
				bAnyBodiesChanged = true;
				ActorData.PreviousBodyWorldTransform = BodyWT;
				FTransform BodyCST = BodyWT.GetRelativeTransform(GetComponentTransform());

				for (const FActorChunkData& ChunkData : ActorData.Chunks)
				{
					// The indices in ActorChunkIndices are NEW blast indices, so must go through indirection.
					uint32 ChunkIndex = ChunkData.ChunkIndex;
					int32 BoneIndex = BlastMesh->ChunkIndexToBoneIndex[ChunkIndex];
					GetEditableComponentSpaceTransforms()[BoneIndex] = BlastMesh->
						GetComponentSpaceInitialBoneTransform(BoneIndex) * BodyCST;
					BonesTouched[BoneIndex] = true;
				}
			}
		}
	}

	//We need to move the bones under any of the body bones that moved, technically we don't need to update these until SetupNewBlastActor since they are invisible, but just for sanity I think we should
	//until it's proven to be a perf bottleneck since SkinnedMeshComponent::GetBone* are not virtual so we can't do them on demand when somebody queries them. This means GetBoneTransform would give wrong values for them.
	if (bAnyBodiesChanged)
	{
		SCOPE_CYCLE_COUNTER(STAT_BlastMeshComponent_SyncChunksAndBodiesChildren);

		//BoneSpaceTransforms are sorted so parents always go first
		const auto& BoneSpaceTransforms = GetSkinnedAsset()->GetRefSkeleton().GetRefBonePose();

		// Build in 3 passes.
		const FTransform* LocalTransformsData = BoneSpaceTransforms.GetData();
		FTransform* SpaceBasesData = GetEditableComponentSpaceTransforms().GetData();

		//Skip 0 since we know the root bone is fine
		for (int32 BoneIndex = 1; BoneIndex < BoneSpaceTransforms.Num(); BoneIndex++)
		{
			//Did we just update this
			if (!BonesTouched[BoneIndex])
			{
				// For all bones below the root, final component-space transform is relative transform * component-space transform of parent.
				const int32 ParentIndex = GetSkinnedAsset()->GetRefSkeleton().GetParentIndex(BoneIndex);

				if (BonesTouched[ParentIndex])
				{
					FTransform::Multiply(SpaceBasesData + BoneIndex, LocalTransformsData + BoneIndex,
					                     SpaceBasesData + ParentIndex);
					BonesTouched[BoneIndex] = true;

					checkSlow(GetEditableComponentSpaceTransforms()[BoneIndex].IsRotationNormalized());
					checkSlow(!GetEditableComponentSpaceTransforms()[BoneIndex].ContainsNaN());
				}
			}
		}
	}

	bNeedToFlipSpaceBaseBuffers |= bAnyBodiesChanged;

	return bAnyBodiesChanged;
}

void UBlastMeshComponent::RefreshBoneTransforms(FActorComponentTickFunction* TickFunction)
{
	if (!GetSkinnedAsset() || GetNumComponentSpaceTransforms() == 0)
	{
		return;
	}

	bool bBodiesMoved = SyncChunksAndBodies();

	if (bBodiesMoved || !bHasValidBoneTransform || bAddedOrRemovedActorSinceLastRefresh)
	{
		/*
		 * Rebuild chunk visibility only if it changed, otherwise we can push crap data into the bone visibility buffer (because RebuildChunkVisibility sets bBoneVisibilityDirty=true)
		 * When the buffers are eventually flipped (eg. in FinalizeBoneTransform()) the bone visibility is 'commited' with old non-updated data instead
		 * The bad values persist because as far as blast is concerned, nothing has changed, so new data never makes it into the bone visibility array, because that is preceded by this call
		 * TLDR: You MUST check this before rebuilding chunk visibility (with the exception maybe being the initialization step)
		*/
		if (bChunkVisibilityChanged && BlastMesh)
		{
			RebuildChunkVisibility();
			// Clean motion vector only if we're using per bone motion blur
			if (bPerBoneMotionBlur)
			{
				ClearMotionVector();
			}
		}

		// Flip bone buffer and send 'post anim' notification
		FinalizeBoneTransform();

		// Update Child Transform - The above function changes bone transform, so will need to update child transform
		UpdateChildTransforms();

		// animation often change overlap. 
		UpdateOverlaps();

		// Cached local bounds are now out of date
		InvalidateCachedBounds();

		// update bounds
		UpdateBounds();

		// Need to send new bounds to 
		MarkRenderTransformDirty();

		// New bone positions need to be sent to render thread
		MarkRenderDynamicDataDirty();

		bAddedOrRemovedActorSinceLastRefresh = false;
	}
}

class FBlastMeshComponentInstanceData : public FPrimitiveComponentInstanceData
{
public:
	FBlastMeshComponentInstanceData(const UBlastMeshComponent* SourceComponent) :
		FPrimitiveComponentInstanceData(SourceComponent),
		ModifiedAsset(SourceComponent->ModifiedAsset),
		ModifiedAssetOwned(SourceComponent->ModifiedAssetOwned),
		SupportStructure(SourceComponent->OwningSupportStructure),
		SupportStructureIndex(SourceComponent->OwningSupportStructureIndex),
		//Unfortunately by the time we get here the new transform has already been set on ComponentToWorld
		PrevWorldTransform(SourceComponent->ModifiedAssetComponentToWorldAtBake)
	{
	}

	virtual void ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override
	{
		FPrimitiveComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);
		UBlastMeshComponent* NewComponent = CastChecked<UBlastMeshComponent>(Component);

		//Make sure this is current
		NewComponent->UpdateComponentToWorld(EUpdateTransformFlags::SkipPhysicsUpdate);
		if (!NewComponent->GetComponentTransform().Equals(PrevWorldTransform) || !NewComponent->bSupportedByWorld)
		{
			//Old ones are stale and unneeded
			ModifiedAsset = nullptr;
			ModifiedAssetOwned = nullptr;
		}

		if (ModifiedAssetOwned)
		{
			//Reparent it
			ModifiedAssetOwned->Rename(nullptr, NewComponent);
			check(ModifiedAssetOwned->GetOuter() == NewComponent);
		}

		//This sets both members, and dirties the world build state if required
		NewComponent->SetModifiedAsset(ModifiedAssetOwned ? ModifiedAssetOwned : ModifiedAsset);
		NewComponent->SetOwningSuppportStructure(SupportStructure, SupportStructureIndex);
	}

	virtual void FindAndReplaceInstances(const TMap<UObject*, UObject*>& OldToNewInstanceMap) override
	{
		FPrimitiveComponentInstanceData::FindAndReplaceInstances(OldToNewInstanceMap);

		if (UObject* const* NewModifiedAsset = OldToNewInstanceMap.Find(ModifiedAsset))
		{
			ModifiedAsset = CastChecked<UBlastAsset>(*NewModifiedAsset, ECastCheckedType::NullAllowed);
		}

		if (UObject* const* NewModifiedAssetOwned = OldToNewInstanceMap.Find(ModifiedAssetOwned))
		{
			ModifiedAssetOwned = CastChecked<UBlastAsset>(*NewModifiedAssetOwned, ECastCheckedType::NullAllowed);
		}

		if (UObject* const* NewSupportStructure = OldToNewInstanceMap.Find(SupportStructure))
		{
			SupportStructure = CastChecked<ABlastExtendedSupportStructure>(
				*NewSupportStructure, ECastCheckedType::NullAllowed);
		}
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		FPrimitiveComponentInstanceData::AddReferencedObjects(Collector);
		Collector.AddReferencedObject(ModifiedAsset);
		Collector.AddReferencedObject(ModifiedAssetOwned);
		Collector.AddReferencedObject(SupportStructure);
	}

	bool ContainsData() const
	{
		return FPrimitiveComponentInstanceData::ContainsData() || ModifiedAsset || ModifiedAssetOwned ||
			SupportStructure || SupportStructureIndex != INDEX_NONE;
	}

private:
	TObjectPtr<UBlastAsset> ModifiedAsset;
	TObjectPtr<UBlastAsset> ModifiedAssetOwned;
	TObjectPtr<ABlastExtendedSupportStructure> SupportStructure;
	int32 SupportStructureIndex;
	FTransform PrevWorldTransform;
};

//Since we contain an instanced subobject of the glue data we need to implement a custom instance data so preserve that when we are re-instanced during BP compilation
//which happens a lot (on map load for example) since FActorComponentInstanceData::FActorComponentInstanceData skips those
TStructOnScope<FActorComponentInstanceData> UBlastMeshComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FBlastMeshComponentInstanceData>(this);
}

void UBlastMeshComponent::InvalidateLightingCacheDetailed(bool bInvalidateBuildEnqueuedLighting, bool bTranslationOnly)
{
	Super::InvalidateLightingCacheDetailed(bInvalidateBuildEnqueuedLighting, bTranslationOnly);

	SetModifiedAsset(nullptr);
	MarkDirtyOwningSuppportStructure();
}

bool UBlastMeshComponent::ShouldRenderSelected() const
{
	if (OwningSupportStructure && OwningSupportStructure->IsSelected())
	{
		return true;
	}
	return Super::ShouldRenderSelected();
}

//Copied from UPoseableMeshComponent::FillComponentSpaceTransforms, just does a simple update from BoneSpaceTransforms -> GetEditableComponentSpaceTransforms
void UBlastMeshComponent::FillInitialComponentSpaceTransformsFromMesh()
{
	if (!GetSkinnedAsset())
	{
		return;
	}

	const auto& BoneSpaceTransforms = GetSkinnedAsset()->GetRefSkeleton().GetRefBonePose();

	// right now all this does is to convert to SpaceBases
	check(GetSkinnedAsset()->GetRefSkeleton().GetNum() == BoneSpaceTransforms.Num());
	check(GetSkinnedAsset()->GetRefSkeleton().GetNum() == GetNumComponentSpaceTransforms());
	check(GetSkinnedAsset()->GetRefSkeleton().GetNum() == GetBoneVisibilityStates().Num());

	const int32 NumBones = BoneSpaceTransforms.Num();

#if DO_GUARD_SLOW
	/** Keep track of which bones have been processed for fast look up */
	TArray<uint8, TInlineAllocator<256>> BoneProcessed;
	BoneProcessed.AddZeroed(NumBones);
#endif
	// Build in 3 passes.
	const FTransform* LocalTransformsData = BoneSpaceTransforms.GetData();
	FTransform* SpaceBasesData = GetEditableComponentSpaceTransforms().GetData();

	GetEditableComponentSpaceTransforms()[0] = BoneSpaceTransforms[0];
#if DO_GUARD_SLOW
	BoneProcessed[0] = 1;
#endif

	for (int32 BoneIndex = 1; BoneIndex < BoneSpaceTransforms.Num(); BoneIndex++)
	{
		FPlatformMisc::Prefetch(SpaceBasesData + BoneIndex);

#if DO_GUARD_SLOW
		// Mark bone as processed
		BoneProcessed[BoneIndex] = 1;
#endif
		// For all bones below the root, final component-space transform is relative transform * component-space transform of parent.
		const int32 ParentIndex = GetSkinnedAsset()->GetRefSkeleton().GetParentIndex(BoneIndex);
		FPlatformMisc::Prefetch(SpaceBasesData + ParentIndex);

#if DO_GUARD_SLOW
		// Check the precondition that Parents occur before Children in the RequiredBones array.
		checkSlow(BoneProcessed[ParentIndex] == 1);
#endif
		FTransform::Multiply(SpaceBasesData + BoneIndex, LocalTransformsData + BoneIndex, SpaceBasesData + ParentIndex);

		checkSlow(GetEditableComponentSpaceTransforms()[BoneIndex].IsRotationNormalized());
		checkSlow(!GetEditableComponentSpaceTransforms()[BoneIndex].ContainsNaN());
	}
	bNeedToFlipSpaceBaseBuffers = true;
}

void UBlastMeshComponent::RebuildChunkVisibility()
{
	TArray<uint8>& EditableBoneVisibilityStates = GetEditableBoneVisibilityStates();
	FMemory::Memzero(EditableBoneVisibilityStates.GetData(), EditableBoneVisibilityStates.Num());
	const auto* ChunkIndexToBoneIndex = BlastMesh->ChunkIndexToBoneIndex.GetData();
	for (TConstSetBitIterator<> It(ChunkVisibility); It; ++It) //TConstSetBitIterator automatically skips unset bits
	{
		int32 BoneIndex = ChunkIndexToBoneIndex[It.GetIndex()];
		if (EditableBoneVisibilityStates.IsValidIndex(BoneIndex))
		{
			EditableBoneVisibilityStates[BoneIndex] = BVS_Visible;
		}
	}

	bBoneVisibilityDirty = true;
	bChunkVisibilityChanged = false;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// Send visible chunks to render thread for collision debug render 
	//Need to check SceneProxy since we don't know when to set BlastProxy to null
	if (BlastProxy && SceneProxy)
	{
		ENQUEUE_RENDER_COMMAND(VisibleBonesForDebugDataCommand)(
			[BlastProxy{BlastProxy}, Chunks{ChunkVisibility}](FRHICommandList& RHICmdList) mutable
			{
				BlastProxy->UpdateVisibleChunks(MoveTemp(Chunks));
			});
	}
#endif
}

bool UBlastMeshComponent::AllocateTransformData()
{
	// Allocate transforms if not present.
	if (USkinnedMeshComponent::AllocateTransformData())
	{
		FillInitialComponentSpaceTransformsFromMesh();
		FinalizeBoneTransform();
		return true;
	}

	return false;
}


bool UBlastMeshComponent::ShouldCreatePhysicsState() const
{
	UBlastAsset* BlastAsset = GetBlastAsset();
	return (BlastAsset && BlastAsset->GetLoadedAsset() && OwningSupportStructureIndex == INDEX_NONE);
}

bool UBlastMeshComponent::HasValidPhysicsState() const
{
	return BlastFamily.IsValid();
}

void UBlastMeshComponent::OnRegister()
{
	Super::OnRegister();

	ConditionalUpdateComponentToWorld();

#if WITH_EDITOR
	//	Invalidate support data if ComponentToWorld has changed since last bake
	if (ModifiedAsset && !ModifiedAssetComponentToWorldAtBake.Equals(GetComponentTransform()))
	{
		if (OwningSupportStructure != nullptr && OwningSupportStructure->GetExtendedSupportMeshComponent() != nullptr)
		{
			OwningSupportStructure->GetExtendedSupportMeshComponent()->InvalidateSupportData();
		}
	}
#endif

	if (LeaderPoseComponent.IsValid())
	{
		UE_LOG(LogBlast, Warning, TEXT("LeaderPoseComponent cannot be set on UBlastMeshComponent"));
		LeaderPoseComponent.Reset();
	}

	ChunkVisibility.Reset();
	ChunkToActorIndex.Reset();
	if (BlastMesh == nullptr)
	{
		SetSkinnedAsset(nullptr);
	}
	else
	{
		SetSkinnedAsset(BlastMesh->Mesh);

		ChunkVisibility.Init(false, BlastMesh->GetChunkCount());
		ChunkToActorIndex.SetNumUninitialized(BlastMesh->GetChunkCount());
		for (int32 C = 0; C < ChunkToActorIndex.Num(); C++)
		{
			ChunkToActorIndex[C] = INDEX_NONE;
		}

		//Show the root chunks for preview even if the physics is not created
		ShowRootChunks();
	}

	bChunkVisibilityChanged = true;
	bAddedOrRemovedActorSinceLastRefresh = true;
	bHasValidBoneTransform = false;
	MarkRenderDynamicDataDirty();
}

bool UBlastMeshComponent::ShouldUpdateTransform(bool bLODHasChanged) const
{
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		//bRecentlyRendered doesn't work if the view is non-realtime
		return true;
	}
#endif
	return USkinnedMeshComponent::ShouldUpdateTransform(bLODHasChanged);
}

bool UBlastMeshComponent::ShouldTickPose() const
{
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		//bRecentlyRendered doesn't work if the view is non-realtime
		return true;
	}
#endif
	return USkinnedMeshComponent::ShouldTickPose();
}


void UBlastMeshComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags,
                                            ETeleportType Teleport /*= ETeleportType::None*/)
{
	//We handle the Physics update
	Super::OnUpdateTransform(UpdateTransformFlags | EUpdateTransformFlags::SkipPhysicsUpdate, Teleport);

	if (ModifiedAsset && !ModifiedAssetComponentToWorldAtBake.Equals(GetComponentTransform()))
	{
		UWorld* World = GetWorld();
		if (World && !World->IsGameWorld() && !World->bIsRunningConstructionScript)
		{
			SetModifiedAsset(nullptr);
			MarkDirtyOwningSuppportStructure();
		}
	}

	if (!HasValidPhysicsState() || !!(UpdateTransformFlags & EUpdateTransformFlags::SkipPhysicsUpdate))
	{
		return;
	}

	TOptional<FScopedSceneLock_Chaos> Lock;
	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		FActorData& Actor = BlastActors[ActorIndex];
		if (Actor.BodyInstance && Actor.bIsAttachedToComponent)
		{
			if (!Lock)
			{
				//There might be none, so only lock if we need to
				Lock = FScopedSceneLock_Chaos(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Write);
			}
			//Actor transform pivots are all at component origin
			Actor.BodyInstance->SetBodyTransform(GetComponentTransform(), Teleport);
			Actor.BodyInstance->UpdateBodyScale(GetComponentTransform().GetScale3D());
		}
	}
}

void UBlastMeshComponent::ForEachBody(TFunctionRef<void(FBodyInstance*)> Worker)
{
	for (int32 Idx = BlastActorsBeginLive; Idx < BlastActorsEndLive; Idx++)
	{
		if (BlastActors[Idx].BodyInstance)
		{
			Worker(BlastActors[Idx].BodyInstance);
		}
	}
}

void UBlastMeshComponent::ForEachBody(TFunctionRef<void(const FBodyInstance*)> Worker) const
{
	for (int32 Idx = BlastActorsBeginLive; Idx < BlastActorsEndLive; Idx++)
	{
		if (BlastActors[Idx].BodyInstance)
		{
			Worker(BlastActors[Idx].BodyInstance);
		}
	}
}

void UBlastMeshComponent::ForEachBodyEx(TFunctionRef<void(FBodyInstance*, bool&)> Worker)
{
	bool bDone = false;
	for (int32 Idx = BlastActorsBeginLive; Idx < BlastActorsEndLive; Idx++)
	{
		if (BlastActors[Idx].BodyInstance)
		{
			Worker(BlastActors[Idx].BodyInstance, bDone);
		}
		if (bDone)
		{
			break;
		}
	}
}

void UBlastMeshComponent::ForEachBodyEx(TFunctionRef<void(const FBodyInstance*, bool&)> Worker) const
{
	bool bDone = false;
	for (int32 Idx = BlastActorsBeginLive; Idx < BlastActorsEndLive; Idx++)
	{
		if (BlastActors[Idx].BodyInstance)
		{
			Worker(BlastActors[Idx].BodyInstance, bDone);
		}
		if (bDone)
		{
			break;
		}
	}
}

bool UBlastMeshComponent::OverlapComponent(const FVector& Pos, const FQuat& Rot,
                                           const FCollisionShape& CollisionShape) const
{
	bool bSuccess = false;
	ForEachBodyEx(
		[&](const FBodyInstance* Body, bool& bDone)
		{
			if (Body->OverlapTest(Pos, Rot, CollisionShape))
			{
				bDone = true;
				bSuccess = true;
			}
		});

	return bSuccess;
}

bool UBlastMeshComponent::UpdateOverlapsImpl(const TOverlapArrayView* PendingOverlaps, bool bDoNotifies,
                                             const TOverlapArrayView* OverlapsAtEndLocation)
{
	return UPrimitiveComponent::UpdateOverlapsImpl(PendingOverlaps, bDoNotifies, OverlapsAtEndLocation);
}

bool UBlastMeshComponent::ComponentOverlapComponentImpl(class UPrimitiveComponent* PrimComp, const FVector Pos,
                                                        const FQuat& Quat, const struct FCollisionQueryParams& Params)
{
	// we do not support skinned mesh vs skinned mesh overlap test
	if (PrimComp->IsA<USkinnedMeshComponent>())
	{
		UE_LOG(LogCollision, Warning,
		       TEXT("ComponentOverlapComponent : (%s) Does not support skinnedmesh with Physics Asset"),
		       *PrimComp->GetPathName());
		return false;
	}

	TArray<FBodyInstance*> Bodies;
	Bodies.Reserve(BlastActors.Num());
	ForEachBody(
		[&Bodies](FBodyInstance* Body)
		{
			Bodies.Add(Body);
		});

	if (const FBodyInstance* Body = PrimComp->GetBodyInstance())
	{
		return Body->OverlapTestForBodies(Pos, Quat, Bodies);
	}

	return false;
}

bool UBlastMeshComponent::ComponentOverlapMultiImpl(TArray<struct FOverlapResult>& OutOverlaps, const UWorld* World,
                                                    const FVector& Pos, const FQuat& Quat,
                                                    ECollisionChannel TestChannel,
                                                    const struct FComponentQueryParams& Params,
                                                    const struct FCollisionObjectQueryParams& ObjectQueryParams) const
{
	OutOverlaps.Reset();

	const FTransform WorldToComponent(GetComponentTransform().Inverse());
	const FCollisionResponseParams ResponseParams(GetCollisionResponseToChannels());

	FComponentQueryParams ParamsWithSelf = Params;
	ParamsWithSelf.AddIgnoredComponent(this);

	bool bHaveBlockingHit = false;
	ForEachBody(
		[&](const FBodyInstance* Body)
		{
			checkSlow(Body);
			if (Body->OverlapMulti(OutOverlaps, World, &WorldToComponent, Pos, Quat, TestChannel, ParamsWithSelf,
			                       ResponseParams, ObjectQueryParams))
			{
				bHaveBlockingHit = true;
			}
		});

	return bHaveBlockingHit;
}

bool UBlastMeshComponent::SweepComponent(FHitResult& OutHit, const FVector Start, const FVector End,
                                         const FQuat& ShapeWorldRotation, const FCollisionShape& CollisionShape,
                                         bool bTraceComplex)
{
	bool bHaveHit = false;

	FHitResult Hit;
	ForEachBody([&](FBodyInstance* Body)
	{
		checkSlow(Body);
		if (Body->Sweep(Hit, Start, End, ShapeWorldRotation, CollisionShape, bTraceComplex))
		{
			if (!bHaveHit || Hit.Time < OutHit.Time)
			{
				OutHit = Hit;
			}
			bHaveHit = true;
		}
	});

	return bHaveHit;
}

UBlastAsset* UBlastMeshComponent::GetBlastAsset(bool bAllowModifiedAsset) const
{
	if (!BlastMesh || !BlastMesh->IsValidBlastMesh())
	{
		return nullptr;
	}

	if (ModifiedAsset != nullptr && bAllowModifiedAsset)
	{
		return ModifiedAsset;
	}

	return BlastMesh;
}

void UBlastMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bBindOnHitDelegate)
	{
		OnComponentHit.AddDynamic(this, &UBlastMeshComponent::OnHit);
	}
}

void UBlastMeshComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
	auto MeshResource = (ShouldRender() && GetSkinnedAsset()) ? GetSkinnedAsset()->GetResourceForRendering() : nullptr;
	if (MeshResource)
	{
		//Need to update it next draw if only the renderstate is recreated, and we are not re-registered
		//Can't call MarkRenderDynamicDataDirty(); since we could already be in an End-of-frame update
		RebuildChunkVisibility();

		//Force a refresh
		bAddedOrRemovedActorSinceLastRefresh = true;
		bHasValidBoneTransform = false;
	}
}

void UBlastMeshComponent::DestroyRenderState_Concurrent()
{
	Super::DestroyRenderState_Concurrent();

	BlastProxy = nullptr;
}

void UBlastMeshComponent::SendRenderDynamicData_Concurrent()
{
	//Must be done before calling the base class if using bone visibility since that updates the mesh object
	if (bChunkVisibilityChanged && BlastMesh)
	{
		RebuildChunkVisibility();
	}

#if WITH_EDITOR
	//Need to check SceneProxy since we don't know when to set BlastProxy to null
	if (BlastProxy && SceneProxy)
	{
		ENQUEUE_RENDER_COMMAND(DebugLinesCommand)(
			[BlastProxy{BlastProxy}, Lines{MoveTemp(PendingDebugLines)}, Points{MoveTemp(PendingDebugPoints)}](
			FRHICommandList& RHICmdList) mutable
			{
				BlastProxy->UpdateDebugDrawLines(MoveTemp(Lines), MoveTemp(Points));
			});
	}
#endif

	Super::SendRenderDynamicData_Concurrent();
}

void UBlastMeshComponent::SetBlastMesh(UBlastMesh* NewBlastMesh)
{
	if (BlastMesh != NewBlastMesh || (BlastMesh && NewBlastMesh && BlastMesh->Mesh != NewBlastMesh->Mesh))
	{
		FComponentReregisterContext ReregisterComponent(this);
		BlastMesh = NewBlastMesh;
		SetSkinnedAsset(BlastMesh ? BlastMesh->Mesh : nullptr);
		ModifiedAsset = nullptr;
		ModifiedAssetOwned = nullptr;
#if WITH_EDITOR
		if (IsWorldSupportDirty())
		{
			UBlastGlueWorldTag::SetDirty(GetWorld());
		}
		if (IsExtendedSupportDirty())
		{
			SetOwningSuppportStructure(nullptr, INDEX_NONE);
			UBlastGlueWorldTag::SetExtendedSupportDirty(GetWorld());
		}
#endif
	}
}

#if WITH_EDITOR
bool UBlastMeshComponent::IsWorldSupportDirty() const
{
	bool bWorldCanBeGlued = false;
	UWorld* World = GetWorld();
	//Only dirty worlds which could be re-glued, if bIsRunningConstructionScript we might not be done assigning all the members
	if (World && !World->IsGameWorld() && !World->bIsRunningConstructionScript)
	{
		bWorldCanBeGlued = true;
	}

	if (BlastMesh && bSupportedByWorld && !ModifiedAsset)
	{
		return bWorldCanBeGlued;
	}
	else if (BlastMesh && ModifiedAsset && BlastMesh->GetAssetGUID() != ModifiedAsset->GetAssetGUID())
	{
		return bWorldCanBeGlued;
	}
	return false;
}

bool UBlastMeshComponent::IsExtendedSupportDirty() const
{
	bool bWorldCanSupported = false;
	UWorld* World = GetWorld();
	//Only dirty worlds which could be re-glued, if bIsRunningConstructionScript we might not be done assigning all the members
	if (World && !World->IsGameWorld() && !World->bIsRunningConstructionScript)
	{
		bWorldCanSupported = true;
	}

	if (BlastMesh && OwningSupportStructure && OwningSupportStructureIndex == INDEX_NONE)
	{
		return bWorldCanSupported;
	}
	else if (BlastMesh && OwningSupportStructure)
	{
		UBlastExtendedSupportMeshComponent* ExtSupport = OwningSupportStructure->GetExtendedSupportMeshComponent();
		const TArray<FBlastExtendedStructureComponent>& SavedComponents = ExtSupport->GetSavedComponents();
		if (SavedComponents.IsValidIndex(OwningSupportStructureIndex))
		{
			const auto& Saved = SavedComponents[OwningSupportStructureIndex];
			if (Saved.GUIDAtMerge == BlastMesh->GetAssetGUID() && Saved.TransformAtMerge.
			                                                            Equals(GetComponentTransform()))
			{
				return false;
			}
		}
		return bWorldCanSupported;
	}
	return false;
}

#endif

void UBlastMeshComponent::SetOwningSuppportStructure(ABlastExtendedSupportStructure* NewStructure, int32 Index)
{
	if (NewStructure != OwningSupportStructure || Index != OwningSupportStructureIndex)
	{
		FComponentReregisterContext ReregisterComponent(this);
#if WITH_EDITOR
		if (OwningSupportStructure != NewStructure)
		{
			OwningSupportStructureIndex = INDEX_NONE;
			//Make sure don't try and read invalid data inside InvalidateSupportData()
			if (OwningSupportStructure)
			{
				OwningSupportStructure->RemoveStructureActor(GetOwner());
			}
		}
#endif
		OwningSupportStructure = NewStructure;
		OwningSupportStructureIndex = NewStructure ? Index : INDEX_NONE;
#if WITH_EDITOR
		if (IsWorldSupportDirty())
		{
			UBlastGlueWorldTag::SetDirty(GetWorld());
		}
		if (IsExtendedSupportDirty())
		{
			UBlastGlueWorldTag::SetExtendedSupportDirty(GetWorld());
		}
#endif
	}
}

void UBlastMeshComponent::MarkDirtyOwningSuppportStructure()
{
	OwningSupportStructureIndex = INDEX_NONE;
	//Consider mark dirty other components of OwningSupportStructure 
}

bool UBlastMeshComponent::HasBeenFractured() const
{
	return bHasBeenFractured;
}

void UBlastMeshComponent::Reset()
{
	UninitBlastFamily();
	InitBlastFamily();
}

void UBlastMeshComponent::BroadcastOnDamaged(FName ActorName, const FVector& DamageOrigin, const FRotator& DamageRot,
                                             FName DamageType)
{
	OnDamaged.Broadcast(this, ActorName, DamageOrigin, DamageRot, DamageType);
}

void UBlastMeshComponent::BroadcastOnActorCreated(FName ActorName)
{
	OnActorCreated.Broadcast(this, ActorName);
}

void UBlastMeshComponent::BroadcastOnActorDestroyed(FName ActorName)
{
	OnActorDestroyed.Broadcast(this, ActorName);
}

void UBlastMeshComponent::BroadcastOnActorCreatedFromDamage(FName ActorName, const FVector& DamageOrigin,
                                                            const FRotator& DamageRot, FName DamageType)
{
	OnActorCreatedFromDamage.Broadcast(this, ActorName, DamageOrigin, DamageRot, DamageType);
}

void UBlastMeshComponent::BroadcastOnBondsDamaged(FName ActorName, bool bIsSplit, FName DamageType,
                                                  const TArray<FBondDamageEvent>& Events)
{
	OnBondsDamaged.Broadcast(this, ActorName, bIsSplit, DamageType, Events);
}

void UBlastMeshComponent::BroadcastOnChunksDamaged(FName ActorName, bool bIsSplit, FName DamageType,
                                                   const TArray<FChunkDamageEvent>& Events)
{
	OnChunksDamaged.Broadcast(this, ActorName, bIsSplit, DamageType, Events);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageComponent(UBlastBaseDamageComponent* DamageComponent, FVector Origin,
                                                             FRotator Rot, FName BoneName)
{
	FQuat QuatRot = Rot.Quaternion();
	return ApplyDamageProgram(*DamageComponent->GetDamagePorgram(), Origin, QuatRot, BoneName);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageComponentOverlap(UBlastBaseDamageComponent* DamageComponent,
                                                                    FVector Origin, FRotator Rot)
{
	FQuat QuatRot = Rot.Quaternion();
	return ApplyDamageProgramOverlap(*DamageComponent->GetDamagePorgram(), Origin, QuatRot);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageComponentOverlapAll(UBlastBaseDamageComponent* DamageComponent,
                                                                       FVector Origin, FRotator Rot)
{
	FQuat QuatRot = Rot.Quaternion();
	return ApplyDamageProgramOverlapAll(*DamageComponent->GetDamagePorgram(), Origin, QuatRot);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageProgramOverlap(const FBlastBaseDamageProgram& DamageProgram,
                                                                  FVector Origin, FQuat Rot)
{
	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		return OwningSupportStructure->GetExtendedSupportMeshComponent()->ApplyDamageProgramOverlap(
			DamageProgram, Origin, Rot);
	}

	return ApplyDamageProgramOverlapFiltered(this, DamageProgram, Origin, Rot);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageProgramOverlapAll(const FBlastBaseDamageProgram& DamageProgram,
                                                                     FVector Origin, FQuat Rot)
{
	return ApplyDamageProgramOverlapFiltered(nullptr, DamageProgram, Origin, Rot);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageProgram(const FBlastBaseDamageProgram& DamageProgram, FVector Origin,
                                                           FQuat Rot, FName BoneName)
{
	if (bIgnoreDamage)
	{
		return EBlastDamageResult::None;
	}

	if (OwningSupportStructure && OwningSupportStructureIndex != INDEX_NONE)
	{
		return OwningSupportStructure->GetExtendedSupportMeshComponent()->ApplyDamageProgram(
			DamageProgram, Origin, Rot, BoneName);
	}

	EBlastDamageResult totalResult = EBlastDamageResult::None;
	if (BoneName.IsNone())
	{
		FScopedSceneLock_Chaos Lock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Read);
		for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
		{
			EBlastDamageResult result = ApplyDamageOnActor(ActorIndex, DamageProgram, Origin, Rot, &Lock);
			if (result > totalResult)
			{
				totalResult = result;
			}
		}
	}
	else
	{
		const int32 ActorIndex = ActorNameToActorIndex(BoneName);
		if (BlastActors.IsValidIndex(ActorIndex))
		{
			totalResult = ApplyDamageOnActor(ActorIndex, DamageProgram, Origin, Rot);
		}
	}

	return totalResult;
}

EBlastDamageResult UBlastMeshComponent::ApplyRadialDamage(FVector Origin, float MinRadius, float MaxRadius,
                                                          float Damage, float ImpulseStrength, bool bImpulseVelChange,
                                                          bool bRandomizeImpulse)
{
	BlastRadialDamageProgram program(Damage, MinRadius, MaxRadius, ImpulseStrength, bImpulseVelChange,
	                                 bRandomizeImpulse);
	return ApplyDamageProgramOverlap(program, Origin);
}

EBlastDamageResult UBlastMeshComponent::ApplyRadialDamageAll(FVector Origin, float MinRadius, float MaxRadius,
                                                             float Damage, float ImpulseStrength,
                                                             bool bImpulseVelChange)
{
	BlastRadialDamageProgram program(Damage, MinRadius, MaxRadius, ImpulseStrength, bImpulseVelChange);
	return ApplyDamageProgramOverlapAll(program, Origin);
}

EBlastDamageResult UBlastMeshComponent::ApplyShearDamage(FVector Origin, FQuat Direction, float MinRadius,
                                                         float MaxRadius, float Damage,
                                                         float ImpulseStrength, bool bImpulseVelChange)
{
	BlastShearDamageProgram program(Damage, MinRadius, MaxRadius, ImpulseStrength, bImpulseVelChange);
	return ApplyDamageProgramOverlap(program, Origin, Direction);
}

EBlastDamageResult UBlastMeshComponent::ApplyShearDamageAll(FVector Origin, float MinRadius, float MaxRadius,
                                                            float Damage, float ImpulseStrength, bool bImpulseVelChange)
{
	BlastShearDamageProgram program(Damage, MinRadius, MaxRadius, ImpulseStrength, bImpulseVelChange);
	return ApplyDamageProgramOverlapAll(program, Origin);
}

EBlastDamageResult UBlastMeshComponent::ApplyCapsuleDamage(FVector Origin, FRotator Rot, float HalfHeight,
                                                           float MinRadius, float MaxRadius, float Damage,
                                                           float ImpulseStrength, bool bImpulseVelChange)
{
	BlastCapsuleDamageProgram program(Damage, HalfHeight, MinRadius, MaxRadius, ImpulseStrength, bImpulseVelChange);
	FQuat QuatRot = Rot.Quaternion();
	return ApplyDamageProgramOverlap(program, Origin, QuatRot);
}

EBlastDamageResult UBlastMeshComponent::ApplyCapsuleDamageAll(FVector Origin, FRotator Rot, float HalfHeight,
                                                              float MinRadius, float MaxRadius, float Damage,
                                                              float ImpulseStrength, bool bImpulseVelChange)
{
	BlastCapsuleDamageProgram program(Damage, HalfHeight, MinRadius, MaxRadius, ImpulseStrength, bImpulseVelChange);
	FQuat QuatRot = Rot.Quaternion();
	return ApplyDamageProgramOverlapAll(program, Origin, QuatRot);
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageProgramOverlapFiltered(
	UBlastMeshComponent* mesh, const FBlastBaseDamageProgram& DamageProgram, const FVector& Origin, const FQuat& Rot)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	if (mesh)
	{
		ObjectParams.AddObjectTypesToQuery(mesh->BodyInstance.GetObjectType());
		ObjectParams.AddObjectTypesToQuery(mesh->SmallChunkBodyInstance.GetObjectType()); // Also check for small chunks
	}
	else
	{
		ObjectParams = FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
	}
	static FName BlastDamageOverlapName(TEXT("BlastDamageOverlap"));
	FCollisionQueryParams Params(BlastDamageOverlapName, false);
	GWorld->OverlapMultiByObjectType(Overlaps, Origin, Rot, ObjectParams, DamageProgram.GetCollisionShape(), Params);
	EBlastDamageResult totalResult = EBlastDamageResult::None;
	for (FOverlapResult& OverlapResult : Overlaps)
	{
		if (mesh == nullptr || OverlapResult.Component.Get() == mesh)
		{
			UBlastMeshComponent* owner = Cast<UBlastMeshComponent>(OverlapResult.Component.Get());
			if (owner != nullptr && !owner->bIgnoreDamage)
			{
				uint32 actorIndex = OverlapResult.ItemIndex;
				EBlastDamageResult result = owner->ApplyDamageOnActor(actorIndex, DamageProgram, Origin, Rot);
				if (result > totalResult)
				{
					totalResult = result;
				}
			}
		}
	}

	return totalResult;
}

EBlastDamageResult UBlastMeshComponent::ApplyDamageOnActor(uint32 actorIndex,
                                                           const FBlastBaseDamageProgram& DamageProgram,
                                                           const FVector& Origin, const FQuat& Rot,
                                                           FScopedSceneLock_Chaos* SceneLock)
{
	//Should never happen for a sub-component
	check(!OwningSupportStructure || OwningSupportStructureIndex == INDEX_NONE);

	if (!BlastActors.IsValidIndex(actorIndex) || bIgnoreDamage)
	{
		return EBlastDamageResult::None;
	}

	FActorData& ActorData = BlastActors[actorIndex];

	NvBlastActor* Actor = ActorData.BlastActor;
	if (!Actor)
	{
		return EBlastDamageResult::None;
	}

	if (!NvBlastActorCanFracture(Actor, Nv::Blast::logLL))
	{
		UE_LOG(LogBlast, Verbose, TEXT("Can't fracture actor \"%s\" further."),
		       *(ActorIndexToActorName(actorIndex).ToString()));

		if (bCrumbleInmostChunks)
		{
			FBodyInstance* BodyInst = GetActorBodyInstance(actorIndex);
			if (BodyInst)
			{
				if (SceneLock)
				{
					SceneLock->Release();
				}

				FScopedSceneLock_Chaos WriteLock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Write);
				BreakDownBlastActor(actorIndex);
				WriteLock.Release();
				if (SceneLock)
				{
					*SceneLock = FScopedSceneLock_Chaos(GetWorld()->GetPhysicsScene(),
					                                    EPhysicsInterfaceScopedLockType::Read);
				}
				bNeedToFlipSpaceBaseBuffers = true;
				bAddedOrRemovedActorSinceLastRefresh = true;
			}
			return EBlastDamageResult::Crumbled;
		}
		return EBlastDamageResult::None;
	}

	FBodyInstance* BodyInst = ActorData.BodyInstance;
	check(BodyInst);

	//This is kind of confusing but it seems like Blast operates 100% in component space and not in actor space, but the original component space since it doesn't track transform changes
	FTransform WT = SceneLock ? BodyInst->GetUnrealWorldTransform_AssumesLocked() : BodyInst->GetUnrealWorldTransform();
	WT.SetScale3D(BodyInst->Scale3D);
	const FTransform invWT = WT.Inverse();

	FQuat WorldRotation = Rot; // without this line (quat * quat) crashes for some reason

	FBlastBaseDamageProgram::FInput ProgramInput;
	ProgramInput.worldOrigin = FVector3f(Origin);
	ProgramInput.worldRot = FQuat4f(WorldRotation);
	ProgramInput.localOrigin = FVector3f(invWT.TransformPosition(Origin));
	ProgramInput.localRot = FQuat4f(invWT.GetRotation() * WorldRotation);
	ProgramInput.material = &GetUsedBlastMaterial();

	if (StressSolver)
	{
		DamageProgram.ExecuteStress(*StressSolver, actorIndex, BodyInst, ProgramInput, *this);
	}

	RecentDamageEventsBuffer.Reset();

	if (DamageProgram.Execute(actorIndex, BodyInst, ProgramInput, *this))
	{
		DamageProgram.ExecutePostDamage(actorIndex, BodyInst, ProgramInput, *this);
		BroadcastOnDamaged(ActorIndexToActorName(actorIndex), Origin, Rot.Rotator(), DamageProgram.DamageType);
		if (HandlePostDamage(Actor, DamageProgram.DamageType, &DamageProgram, &ProgramInput, SceneLock))
		{
			// If the damage program wants to do anything else after the split, let it do so here (physics impulse)
			DamageProgram.ExecutePostSplit(ProgramInput, *this);
			return EBlastDamageResult::Split;
		}
		else
		{
			return EBlastDamageResult::Damaged;
		}
	}

	return EBlastDamageResult::None;
}

bool UBlastMeshComponent::ExecuteBlastDamageProgram(uint32 actorIndex, const NvBlastDamageProgram& program,
                                                    const NvBlastExtProgramParams& programParams, FName DamageType)
{
	check(BlastActors.IsValidIndex(actorIndex));
	NvBlastActor* Actor = BlastActors[actorIndex].BlastActor;
	check(Actor != nullptr);

	NvBlastFractureBuffers fractureBuffers;
	FBlastFractureScratch::getInstance().getFractureBuffers(fractureBuffers);

	// Take the program and params above and generate fracture commands into FractureBuffers
	NvBlastActorGenerateFracture(&fractureBuffers, Actor, program, &programParams, Nv::Blast::logLL, nullptr);

	// Apply generated fracture commands
	if (fractureBuffers.bondFractureCount > 0 || fractureBuffers.chunkFractureCount > 0)
	{
		ApplyFracture(actorIndex, fractureBuffers, DamageType);
		return true;
	}
	else
	{
		return false;
	}
}

void UBlastMeshComponent::ApplyFracture(uint32 actorIndex, const NvBlastFractureBuffers& fractureBuffers,
                                        FName DamageType)
{
	FActorData& ActorData = BlastActors[actorIndex];
	NvBlastActor* Actor = ActorData.BlastActor;
	bHasBeenFractured = true;
	SetCanEverAffectNavigation(false);
	// Apply the generated fracture commands to the actor that was hit.
	NvBlastActorApplyFracture(nullptr, Actor, &fractureBuffers, Nv::Blast::logLL, nullptr);

	// Fire chunks/bonds damage events if anyone is subscribed
	const bool bFireBondEvents = OnBondsDamagedBound();
	const bool bFireChunkEvents = OnChunksDamagedBound();

	if (bFireBondEvents || bFireChunkEvents)
	{
		// Reset buffer
		RecentDamageEventsBuffer.Reset();
		RecentDamageEventsBuffer.ActorIndex = actorIndex;
		RecentDamageEventsBuffer.DamageType = DamageType;

		const NvBlastAsset* LLBlastAsset = GetBlastAsset()->GetLoadedAsset();
		const NvBlastBond* Bonds = NvBlastAssetGetBonds(LLBlastAsset, Nv::Blast::logLL);
		const NvBlastChunk* Chunks = NvBlastAssetGetChunks(LLBlastAsset, Nv::Blast::logLL);
		const NvBlastSupportGraph Graph = NvBlastAssetGetSupportGraph(LLBlastAsset, Nv::Blast::logLL);
		const float* BondHealths = NvBlastActorGetBondHealths(Actor, Nv::Blast::logLL);
		const float MaterialHealth = GetUsedBlastMaterial().Health;

		FTransform ActorSpaceToWorldSpace = ActorData.BodyInstance->GetUnrealWorldTransform();
		ActorSpaceToWorldSpace.SetScale3D(ActorData.BodyInstance->Scale3D);

		// Bond damage events
		if (bFireBondEvents)
		{
			RecentDamageEventsBuffer.BondEvents.Empty(fractureBuffers.bondFractureCount);
			for (uint32 i = 0; i < fractureBuffers.bondFractureCount; i++)
			{
				const NvBlastBondFractureData& FractureData = fractureBuffers.bondFractures[i];
				for (uint32 AdjacencyIndex = Graph.adjacencyPartition[FractureData.nodeIndex0]; AdjacencyIndex < Graph.
				     adjacencyPartition[FractureData.nodeIndex0 + 1]; AdjacencyIndex++)
				{
					if (Graph.adjacentNodeIndices[AdjacencyIndex] == FractureData.nodeIndex1)
					{
						uint32 BondIndex = Graph.adjacentBondIndices[AdjacencyIndex];
						const NvBlastBond& SolverBond = Bonds[BondIndex];
						const FVector LocalCentroid(FromNvVector(SolverBond.centroid));
						const FVector LocalNormal(FromNvVector(SolverBond.normal));
						const uint32 chunk0 = Graph.chunkIndices[FractureData.nodeIndex0];
						const uint32 chunk1 = Graph.chunkIndices[FractureData.nodeIndex1];

						FBondDamageEvent BondDmgEvent;
						BondDmgEvent.ChunkIndex = chunk0 < chunk1 ? int32(chunk0) : int32(chunk1);
						BondDmgEvent.OtherChunkIndex = chunk0 < chunk1 ? int32(chunk1) : int32(chunk0);
						BondDmgEvent.Damage = FractureData.health * MaterialHealth;
						BondDmgEvent.HealthLeft = BondHealths[BondIndex] * MaterialHealth;
						BondDmgEvent.BondArea = SolverBond.area;
						BondDmgEvent.WorldCentroid = ActorSpaceToWorldSpace.TransformPosition(LocalCentroid);
						BondDmgEvent.WorldNormal = ActorSpaceToWorldSpace.TransformVector(LocalNormal);
						RecentDamageEventsBuffer.BondEvents.Add(BondDmgEvent);
						break;
					}
				}
			}
		}

		// Chunk damage events
		if (bFireChunkEvents)
		{
			RecentDamageEventsBuffer.ChunkEvents.Empty(fractureBuffers.chunkFractureCount);
			for (uint32 i = 0; i < fractureBuffers.chunkFractureCount; i++)
			{
				const NvBlastChunkFractureData& FractureData = fractureBuffers.chunkFractures[i];
				const NvBlastChunk& SolverChunk = Chunks[FractureData.chunkIndex];
				const FVector LocalCentroid(FromNvVector(SolverChunk.centroid));

				FChunkDamageEvent ChunkDmgEvent;
				ChunkDmgEvent.ChunkIndex = (int32)FractureData.chunkIndex;
				ChunkDmgEvent.Damage = FractureData.health * MaterialHealth;
				ChunkDmgEvent.WorldCentroid = ActorSpaceToWorldSpace.TransformPosition(LocalCentroid);
				RecentDamageEventsBuffer.ChunkEvents.Add(ChunkDmgEvent);
			}
		}
	}
}

bool UBlastMeshComponent::HandlePostDamage(NvBlastActor* actor, FName DamageType,
                                           const FBlastBaseDamageProgram* DamageProgram,
                                           const FBlastBaseDamageProgram::FInput* Input,
                                           FScopedSceneLock_Chaos* SceneLock)
{
	// At this point we can split off some new actors

	uint32 chunkCount = GetBlastAsset()->GetChunkCount();

	TArray<NvBlastActor*> newActorsBuffer;
	newActorsBuffer.SetNum(chunkCount);

	TArray<char> splitScratch;
	splitScratch.SetNum(NvBlastActorGetRequiredScratchForSplit(actor, Nv::Blast::logLL));

	NvBlastActorSplitEvent splitEvent;
	splitEvent.newActors = newActorsBuffer.GetData();
	splitEvent.deletedActor = nullptr;

	uint32 parentActorIndex = NvBlastActorGetIndex(actor, Nv::Blast::logLL);

	uint32 newActorsCount = NvBlastActorSplit(&splitEvent, actor, newActorsBuffer.Num(), splitScratch.GetData(),
	                                          Nv::Blast::logLL, nullptr);
	bool bIsSplit = (splitEvent.deletedActor != nullptr);

	// Now we know if split is going to happen, we can fire buffered damage events
	if (OnChunksDamagedBound() && RecentDamageEventsBuffer.ChunkEvents.Num() > 0)
	{
		check(RecentDamageEventsBuffer.ActorIndex == parentActorIndex);
		// If it fails some damage logic must have changed and we need more clever event buffer (per-actor probably)
		BroadcastOnChunksDamaged(ActorIndexToActorName(parentActorIndex), bIsSplit, DamageType,
		                         RecentDamageEventsBuffer.ChunkEvents);
	}
	if (OnBondsDamagedBound() && RecentDamageEventsBuffer.BondEvents.Num() > 0)
	{
		check(RecentDamageEventsBuffer.ActorIndex == parentActorIndex);
		// If it fails some damage logic must have changed and we need more clever event buffer (per-actor probably)
		BroadcastOnBondsDamaged(ActorIndexToActorName(parentActorIndex), bIsSplit, DamageType,
		                        RecentDamageEventsBuffer.BondEvents);
	}
	RecentDamageEventsBuffer.Reset();


	if (bIsSplit)
	{
		FBodyInstance* ParentBodyInstance = BlastActors[parentActorIndex].BodyInstance;
		FTransform ParentWorldTransform = SceneLock
			                                  ? ParentBodyInstance->GetUnrealWorldTransform_AssumesLocked()
			                                  : ParentBodyInstance->GetUnrealWorldTransform();
		ParentWorldTransform.SetScale3D(ParentBodyInstance->Scale3D);
		FVector ParentLinVel = SceneLock
			                       ? ParentBodyInstance->GetUnrealWorldVelocity_AssumesLocked()
			                       : ParentBodyInstance->GetUnrealWorldVelocity();
		FVector ParentAngVel = SceneLock
			                       ? ParentBodyInstance->GetUnrealWorldAngularVelocityInRadians_AssumesLocked()
			                       : ParentBodyInstance->GetUnrealWorldAngularVelocityInRadians();
		FVector ParentCOM = ParentBodyInstance->GetCOMPosition();

		//Cannot have the read lock when doing BreakDownBlastActor since it can't upgrade to a write lock

		if (SceneLock)
		{
			SceneLock->Release();
		}

		FScopedSceneLock_Chaos WriteLock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Write);

		BreakDownBlastActor(parentActorIndex);
		for (uint32 actorIdx = 0; actorIdx < newActorsCount; actorIdx++)
		{
			// Setup the new BlastActor, referencing the parent that was deleted.
			FBlastActorCreateInfo CreateInfo(ParentWorldTransform);
			CreateInfo.ParentActorLinVel = ParentLinVel;
			CreateInfo.ParentActorAngVel = ParentAngVel;
			CreateInfo.ParentActorCOM = ParentCOM;
			SetupNewBlastActor(newActorsBuffer[actorIdx], CreateInfo, DamageProgram, Input, DamageType);
		}

		WriteLock.Release();

		if (SceneLock)
		{
			*SceneLock = FScopedSceneLock_Chaos(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Read);
		}

		return true;
	}

	return false;
}

void UBlastMeshComponent::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                FVector NormalImpulse, const FHitResult& Hit)
{
	const FBlastImpactDamageProperties& UsedImpactProperties = GetUsedImpactDamageProperties();
	if (!UsedImpactProperties.AdvancedSettings.bSelfCollision && HitComponent == OtherComp)
		return;

	const FName& OurBoneName = Hit.Component == this ? Hit.BoneName : Hit.MyBoneName;
	const FName& OtherBoneName = Hit.Component == this ? Hit.MyBoneName : Hit.BoneName;
	if (OurBoneName.IsNone())
	{
		UE_LOG(LogBlast, Warning,
		       TEXT(
			       "BlastMeshComponent was hit but BoneName is empty. Add BoneName in order impact damage/stress solver/damage component to work."
		       ));
		return;
	}
	const int32 ActorIndex = ActorNameToActorIndex(OurBoneName);
	if (!BlastActors.IsValidIndex(ActorIndex))
	{
		return;
	}

	// Look for a BlastDamageComponent on the actor that hit us.
	UBlastBaseDamageComponent* DamageComponent = OtherActor
		                                             ? OtherActor->FindComponentByClass<UBlastBaseDamageComponent>()
		                                             : nullptr;
	if (!DamageComponent || !DamageComponent->bDamageOnHit)
	{
		// Look for a BlastDamageComponent on us then.
		DamageComponent = this->GetOwner()->FindComponentByClass<UBlastBaseDamageComponent>();
	}

	// Apply Damage with DamageComponent if any
	if (DamageComponent && DamageComponent->bDamageOnHit)
	{
		ApplyDamageOnActor(ActorIndex, *DamageComponent->GetDamagePorgram(), Hit.ImpactPoint, FQuat::Identity);
	}

	// Impact damage
	const FBlastStressProperties& UsedStressProperties = GetUsedStressProperties();
	const FBlastMaterial& UsedBlastMaterial = GetUsedBlastMaterial();
	if (UsedImpactProperties.bEnabled || UsedStressProperties.bApplyImpactImpulses)
	{
		FActorData& ActorData = BlastActors[ActorIndex];

		NvBlastActor* Actor = ActorData.BlastActor;
		FBodyInstance* BodyInst = ActorData.BodyInstance;
		FBodyInstance* OtherBodyInst = OtherComp->GetBodyInstance(OtherBoneName);
		if (BodyInst && OtherBodyInst && Actor && NvBlastActorCanFracture(Actor, Nv::Blast::logLL))
		{
			FScopedSceneLock_Chaos Lock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Read);

			// Reduced mass 
			const float Mass0 = BodyInst->GetBodyMass();
			const float Mass1 = OtherBodyInst->GetBodyMass();
			float ReducedMass;
			if (Mass0 == 0.0f)
			{
				ReducedMass = Mass1;
			}
			else if (Mass1 == 0.0f)
			{
				ReducedMass = Mass0;
			}
			else
			{
				ReducedMass = Mass0 * Mass1 / (Mass0 + Mass1);
			}

			// Impact Impulse
			const FVector VelocityDelta = BodyInst->GetUnrealWorldVelocity_AssumesLocked() - OtherBodyInst->
				GetUnrealWorldVelocity_AssumesLocked();
			const float ImpactVelocity = FMath::Abs<float>(Hit.ImpactNormal | VelocityDelta);
			const float ImpactImpulse = ImpactVelocity * ReducedMass;

			// Pass impact impulse to stress solver?
			if (Actor && BodyInst && UsedStressProperties.bApplyImpactImpulses && UsedStressProperties.bCalculateStress
				&& StressSolver)
			{
				FTransform WT = BodyInst->GetUnrealWorldTransform_AssumesLocked();
				WT.SetScale3D(BodyInst->Scale3D);
				const FTransform invWT = WT.Inverse();

				const float ForceScale = 1.f / (BodyInst->Scale3D.X * BodyInst->Scale3D.X * BodyInst->Scale3D.X *
					BodyInst->Scale3D.X);
				// assuming uniform. (p = mv; X*X*X goes for the volume, and one more X for velocity)
				NvcVec3 LocalPosition = ToNvVector(invWT.TransformPosition(Hit.ImpactPoint));
				NvcVec3 LocalForce = ToNvVector(
					(invWT.TransformVector(Hit.ImpactNormal)).GetSafeNormal() * ImpactImpulse * UsedStressProperties.
					ImpactImpulseToStressImpulseFactor * ForceScale);

				StressSolver->addForce(*Actor, (NvcVec3&)LocalPosition, (NvcVec3&)LocalForce);
			}

			// Apply impact impulse damage ?
			if (UsedImpactProperties.bEnabled)
			{
				const float DamageImpulse = ImpactVelocity * (UsedImpactProperties.AdvancedSettings.bVelocityBased
					                                              ? 1.0f
					                                              : ReducedMass);
				const float Impulse01 = FMath::Clamp<float>(
					FMath::GetRangePct(0.f, UsedBlastMaterial.Health * UsedImpactProperties.Hardness, DamageImpulse),
					0.f, UsedImpactProperties.AdvancedSettings.MaxDamageThreshold);
				if (Impulse01 > UsedImpactProperties.AdvancedSettings.MinDamageThreshold)
				{
					const float Damage = UsedBlastMaterial.Health * Impulse01;

					const float RadiusScale = 1.f / BodyInst->Scale3D.X; // approx for non-uniform scale
					const float MinRadius = UsedImpactProperties.MaxDamageRadius * Impulse01 * RadiusScale;
					const float MaxRadius = MinRadius * FMath::Clamp<float>(
						UsedImpactProperties.AdvancedSettings.DamageFalloffRadiusFactor, 1,
						32); // 32 is just some reasonable limit here
					const FName DamageType(TEXT("Impact"));
					const FQuat NormalRot = Hit.ImpactNormal.Rotation().Quaternion();

					if (UsedImpactProperties.AdvancedSettings.bUseShearDamage)
					{
						BlastShearDamageProgram AppliedDamageProgram(Damage, MinRadius, MaxRadius);
						AppliedDamageProgram.ImpulseStrength = ImpactImpulse * UsedImpactProperties.
							PhysicalImpulseFactor;
						AppliedDamageProgram.DamageType = DamageType;
						ApplyDamageOnActor(ActorIndex, AppliedDamageProgram, Hit.ImpactPoint, NormalRot, &Lock);
					}
					else
					{
						BlastRadialDamageProgram AppliedDamageProgram(Damage, MinRadius, MaxRadius);
						AppliedDamageProgram.ImpulseStrength = ImpactImpulse * UsedImpactProperties.
							PhysicalImpulseFactor;
						AppliedDamageProgram.DamageType = DamageType;
						ApplyDamageOnActor(ActorIndex, AppliedDamageProgram, Hit.ImpactPoint, NormalRot, &Lock);
					}
				}
			}
		}
	}
}

void UBlastMeshComponent::UpdateFractureBufferSize()
{
	UBlastAsset* BlastAsset = GetBlastAsset();
	if (BlastMesh == nullptr)
	{
		return;
	}

	FBlastFractureScratch::getInstance().ensureFractureBuffersSize(BlastAsset->GetChunkCount(),
	                                                               BlastAsset->GetBondCount());
}

bool UBlastMeshComponent::IsSimulatingPhysics(FName BoneName /*= NAME_None*/) const
{
	return true;
}

void UBlastMeshComponent::AddRadialImpulse(FVector Origin, float Radius, float Strength,
                                           enum ERadialImpulseFalloff Falloff, bool bVelChange /*= false*/)
{
	for (int32 A = BlastActorsBeginLive; A < BlastActorsEndLive; A++)
	{
		FBodyInstance* BodyInst = BlastActors[A].BodyInstance;
		if (BodyInst)
		{
			BodyInst->AddRadialImpulseToBody(Origin, Radius, Strength, Falloff, bVelChange);
		}
	}
}


void UBlastMeshComponent::AddRadialForce(FVector Origin, float Radius, float Strength,
                                         enum ERadialImpulseFalloff Falloff, bool bAccelChange /*= false*/)
{
	for (int32 A = BlastActorsBeginLive; A < BlastActorsEndLive; A++)
	{
		FBodyInstance* BodyInst = BlastActors[A].BodyInstance;
		if (BodyInst)
		{
			BodyInst->AddRadialForceToBody(Origin, Radius, Strength, Falloff, bAccelChange);
		}
	}
}

void UBlastMeshComponent::Serialize(FArchive& Ar)
{
	//This is kind of tricky since the USkeletalMesh we use is not a root asset, it can't be referenced in the saved level, however the Blast mesh can.
	//We can't mark the property transient since it comes from the base class so we need to fake it out
	if (Ar.IsSaving())
	{
		SetSkinnedAsset(nullptr);
	}
	Super::Serialize(Ar);
	SetSkinnedAsset(BlastMesh ? BlastMesh->Mesh : nullptr);
}

void UBlastMeshComponent::SetupNewBlastActor(NvBlastActor* actor, const FBlastActorCreateInfo& CreateInfo,
                                             const FBlastBaseDamageProgram* DamageProgram,
                                             const FBlastBaseDamageProgram::FInput* Input, FName DamageType,
                                             bool bIsFirstActor)
{
	uint32 actorIndex = NvBlastActorGetIndex(actor, Nv::Blast::logLL);

	FActorData& ActorData = BlastActors[actorIndex];
	check(ActorData.BlastActor == nullptr);
	ActorData.BlastActor = actor;

	//Extend the live range
	if (BlastActorsBeginLive == BlastActorsEndLive)
	{
		//First actor, range was empty before
		BlastActorsBeginLive = actorIndex;
		BlastActorsEndLive = actorIndex + 1;
	}
	else
	{
		if ((int32)actorIndex < BlastActorsBeginLive)
		{
			BlastActorsBeginLive = actorIndex;
		}

		if ((int32)actorIndex >= BlastActorsEndLive)
		{
			BlastActorsEndLive = actorIndex + 1;
		}
	}


	auto& VisibleChunks = ActorData.Chunks;

	const uint32 VisibleChunkCount = NvBlastActorGetVisibleChunkCount(ActorData.BlastActor, Nv::Blast::logLL);
	VisibleChunks.SetNum(VisibleChunkCount);

	TArray<uint32> VisibleChunksTemp;
	VisibleChunksTemp.SetNumUninitialized(VisibleChunkCount);
	NvBlastActorGetVisibleChunkIndices(VisibleChunksTemp.GetData(), VisibleChunkCount, ActorData.BlastActor,
	                                   Nv::Blast::logLL);

	check(VisibleChunkCount > 0);

	for (uint32 vc = 0; vc < VisibleChunkCount; vc++)
	{
		FActorChunkData& VisibleChunk = VisibleChunks[vc];
		VisibleChunk.ChunkIndex = VisibleChunksTemp[vc];
	}


	InitBodyForActor(ActorData, actorIndex, CreateInfo.Transform, GetWorld()->GetPhysicsScene(), bIsFirstActor);
	ShowActorsVisibleChunks(actorIndex);

	FTransform BodyWorldTransform = ActorData.BodyInstance->GetUnrealWorldTransform();
	BodyWorldTransform.SetScale3D(ActorData.BodyInstance->Scale3D);
	FBox AABB = ActorBodySetups[actorIndex]->AggGeom.CalcAABB(BodyWorldTransform);
	ActorData.StartLocation = AABB.GetCenter();

	// set velocities (passing velocities from parent actor)
	if (!ActorData.bIsAttachedToComponent)
	{
		const FVector ActorCOM = ActorData.BodyInstance->GetCOMPosition();
		const FVector LinVel = CreateInfo.ParentActorLinVel + FVector::CrossProduct(
			CreateInfo.ParentActorAngVel, (ActorCOM - CreateInfo.ParentActorCOM));
		ActorData.BodyInstance->SetLinearVelocity(LinVel, false);
		ActorData.BodyInstance->SetAngularVelocityInRadians(CreateInfo.ParentActorAngVel, false);
	}

	bAddedOrRemovedActorSinceLastRefresh = true;

	if (StressSolver)
	{
		StressSolver->notifyActorCreated(*ActorData.BlastActor);
	}

	if (DamageProgram && Input)
	{
		DamageProgram->ExecutePostActorCreated(actorIndex, ActorData.BodyInstance, *Input, *this);
	}

	if (!DamageType.IsNone())
	{
		BroadcastOnActorCreatedFromDamage(ActorIndexToActorName(actorIndex),
		                                  Input ? FVector(Input->worldOrigin) : FVector::ZeroVector,
		                                  Input ? FQuat(Input->worldRot).Rotator() : FRotator::ZeroRotator, DamageType);
	}

	BroadcastOnActorCreated(ActorIndexToActorName(actorIndex));
}

int32 UBlastMeshComponent::HasChunkInSphere(FVector center, float radius) const
{
	if (BlastMesh == nullptr)
	{
		return 0;
	}
	float r2 = radius * radius;
	for (uint32 i = 0; i < BlastMesh->GetChunkCount(); ++i)
	{
		if ((GetChunkCenterWorldPosition(i) - center).SizeSquared() <= r2)
		{
			return 1;
		}
	}
	return 0;
}

void UBlastMeshComponent::BreakDownBlastActor(uint32 actorIndex)
{
	check(BlastActors.IsValidIndex(actorIndex));
	FActorData& ActorData = BlastActors[actorIndex];
	check(ActorData.BlastActor != nullptr);

	if (StressSolver)
	{
		StressSolver->notifyActorDestroyed(*ActorData.BlastActor);
	}

	if (ActorData.TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ActorData.TimerHandle);
		ActorData.TimerHandle.Invalidate();
		DebrisCount--;
	}

	BroadcastOnActorDestroyed(ActorIndexToActorName(actorIndex));

	HideActorsVisibleChunks(actorIndex);
	FBodyInstance* BodyInst = ActorData.BodyInstance;

	// Remove the FBodyInstance from the PhysicsScene
	BodyInst->TermBody();
	delete BodyInst;

	ActorBodySetups[actorIndex] = nullptr;

	for (const FActorChunkData& C : ActorData.Chunks)
	{
		checkSlow(ChunkToActorIndex[C.ChunkIndex] == actorIndex);
		ChunkToActorIndex[C.ChunkIndex] = INDEX_NONE;
	}

	//Reset the entry
	ActorData = FActorData();

	//Shrink the live range
	if (actorIndex == BlastActorsBeginLive)
	{
		while (BlastActorsBeginLive < BlastActorsEndLive && BlastActors[BlastActorsBeginLive].BlastActor == nullptr)
		{
			BlastActorsBeginLive++;
		}
	}

	if ((actorIndex + 1) == BlastActorsEndLive)
	{
		while (BlastActorsEndLive > BlastActorsBeginLive && BlastActors[BlastActorsEndLive - 1].BlastActor == nullptr)
		{
			BlastActorsEndLive--;
		}
	}

	bAddedOrRemovedActorSinceLastRefresh = true;
}

void UBlastMeshComponent::InitBodyForActor(FActorData& ActorData, uint32 ActorIndex,
                                           const FTransform& ParentActorWorldTransform, FPhysScene* PhysScene,
                                           bool bIsFirstActor)
{
	const UBlastAsset* BlastAsset = GetBlastAsset();
	const auto& VisibleChunks = ActorData.Chunks;

	UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None, RF_Transient);
	check(ActorBodySetups[ActorIndex] == nullptr);
	ActorBodySetups[ActorIndex] = NewBodySetup;

	//This is not a real bone since the body is at the component origin and made up
	NewBodySetup->BoneName = ActorIndexToActorName(ActorIndex);

	const TArray<FBlastCookedChunkData>& CookedData = BlastMesh->GetCookedChunkData_AssumeUpToDate();
	const NvBlastChunk* ChunkData = NvBlastAssetGetChunks(BlastAsset->GetLoadedAsset(), Nv::Blast::logLL);

	bool bContainsRootChunks = false;
	bool bIsKinematicActor = bIsFirstActor && bIsInitiallyKinematic;
	bool bIsAllLeafChunks = true;
	for (int32 i = 0; i < VisibleChunks.Num(); i++)
	{
		const uint32 ChunkIndex = VisibleChunks[i].ChunkIndex;
		bContainsRootChunks |= (BlastAsset->GetChunkDepth(ChunkIndex) == 0);
		bIsKinematicActor |= BlastAsset->IsChunkStatic(ChunkIndex); // one static chunk is enough
		bIsAllLeafChunks &= (ChunkData[ChunkIndex].firstChildIndex == ChunkData[ChunkIndex].childIndexStop);
		if (i == 0)
		{
			CookedData[ChunkIndex].PopulateBodySetup(NewBodySetup);
		}
		else
		{
			CookedData[ChunkIndex].AppendToBodySetup(NewBodySetup);
		}
		checkSlow(ChunkToActorIndex[ChunkIndex] == INDEX_NONE || ChunkToActorIndex[ChunkIndex] == ActorIndex);
		ChunkToActorIndex[ChunkIndex] = ActorIndex;
	}

	// At this point we have a UBodySetup with all of the collision from the visible chunks the actor has, so create a FBodyInstance using it and add init it.

	FBodyInstance* BodyInst = new FBodyInstance();

	// Check if bound to world ('glue' way to make actor kinematic)
	if (ActorData.BlastActor != nullptr && !bIsKinematicActor)
	{
		bIsKinematicActor |= NvBlastActorIsBoundToWorld(ActorData.BlastActor, Nv::Blast::logLL);
	}

	if (bIsKinematicActor)
	{
		BodyInst->CopyBodyInstancePropertiesFrom(&BodyInstance);
	}
	else
	{
		if (bShouldAllChildrenChunksBeSmallChunks && bHasBeenFractured)
		{
			ActorData.bIsSmallChunk = true;
		}
		else if (!bHasBeenFractured && ActorIndex == 0)
		{
			ActorData.bIsSmallChunk = false;
		}
		else
		{
			ActorData.bIsSmallChunk =
				NewBodySetup->AggGeom.CalcAABB(GetComponentTransform()).GetSize().Size() / 2.f <= SmallChunkRadius;
		}
		if (ActorData.bIsSmallChunk)
		{
			BodyInst->CopyBodyInstancePropertiesFrom(&SmallChunkBodyInstance);
		}
		else
		{
			BodyInst->CopyBodyInstancePropertiesFrom(&DynamicChunkBodyInstance);
		}
	}

	BodyInst->bSimulatePhysics = !bIsKinematicActor;
	BodyInst->InstanceBodyIndex = ActorIndex; // let it be actor index
	if (bIsAllLeafChunks && !GetUsedBlastMaterial().bGenerateHitEventsForLeafActors)
	{
		BodyInst->bNotifyRigidBodyCollision = false;
	}

	BodyInst->bStartAwake = true; // Default to true - should we be taking this from higher up?
	BodyInst->DOFMode = EDOFMode::None;

	BodyInst->InitBody(NewBodySetup, ParentActorWorldTransform, this, PhysScene);

	// set max contact impulse for impact damage
	const FBlastImpactDamageProperties& UsedImpactProperties = GetUsedImpactDamageProperties();
	if (UsedImpactProperties.bEnabled && !BodyInst->bSimulatePhysics && UsedImpactProperties.AdvancedSettings.
		KinematicsMaxContactImpulse >= 0.f)
	{
#if BLAST_USE_PHYSX
		if (BodyInst->ActorHandle.IsValid() && FPhysicsInterface::IsRigidBody(BodyInst->ActorHandle))
		{
			FPhysicsCommand::ExecuteWrite(BodyInst->ActorHandle, [&](const FPhysicsActorHandle& Actor)
				{
					if (PxRigidBody* PRigidBody = FPhysicsInterface::GetPxRigidBody_AssumesLocked(Actor))
					{
						PRigidBody->setMaxContactImpulse(UsedImpactProperties.AdvancedSettings.KinematicsMaxContactImpulse);
#if PX_PHYSICS_VERSION >= (((3<<24) + (4<<16) + (1<<8) + 0)) // available only since 3.4.1
						PRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD_MAX_CONTACT_IMPULSE, true);
#endif
					}
				});
		}
#endif
	}

	BodyInst->UpdateMassProperties();

	ActorData.BodyInstance = BodyInst;
	//This is not totally right, isBoundToWorld actors shouldn't move with the component probably.
	//Maybe we need to add PhysX constraints to the thing they are touching in world, but for now lump them in with root chunks
	ActorData.bIsAttachedToComponent = bIsKinematicActor || bContainsRootChunks;
}

#if WITH_EDITOR
//It's ok to use the normal debug drawing API here since it's not called in tick
bool UBlastMeshComponent::GetSupportChunksInVolumes(const TArray<ABlastGlueVolume*>& Volumes,
                                                    TArray<uint32>& OverlappingChunks, TArray<FVector>& GlueVectors,
                                                    TSet<class ABlastGlueVolume*>& OverlappingVolumes, bool bDrawDebug)
{
	OverlappingChunks.Reset();
	GlueVectors.Reset();
	OverlappingVolumes.Reset();

	// NOTE: We shouldn't be using the modified asset to generate a modified asset!
	UBlastAsset* BlastAsset = GetBlastAsset(false);
	if (!BlastAsset)
	{
		UE_LOG(LogBlast, Error, TEXT("GetSupportChunksInVolume, BlastMesh is invalid"));
		return false;
	}

	NvBlastAsset* LLBlastAsset = BlastAsset->GetLoadedAsset();

	uint32 chunkCount = GetBlastAsset()->GetChunkCount();

	check(chunkCount > 0);

	const NvBlastSupportGraph supportGraph = NvBlastAssetGetSupportGraph(LLBlastAsset, Nv::Blast::logLL);

	// Now get the convexes for each of these support chunks and see if they're overlapping the provided worldspace volume
	UPhysicsAsset* const PhysicsAsset = BlastMesh->PhysicsAsset;

	for (uint32 i = 0; i < supportGraph.nodeCount; i++)
	{
		uint32 ChunkIndex = supportGraph.chunkIndices[i];
		if (ChunkIndex == 0xFFFFFFFF)
		{
			continue;
		}

		uint32 BoneIndex = BlastMesh->ChunkIndexToBoneIndex[ChunkIndex];
		int32 BodySetupIndex = PhysicsAsset->FindBodyIndex(GetSkinnedAsset()->GetRefSkeleton().GetBoneName(BoneIndex));
		if (BodySetupIndex != INDEX_NONE)
		{
			UBodySetup* PhysicsAssetBodySetup = PhysicsAsset->SkeletalBodySetups[BodySetupIndex];
			FTransform BodyXForm = GetBoneTransform(BoneIndex);
			for (FKConvexElem& convex : PhysicsAssetBodySetup->AggGeom.ConvexElems)
			{
				FTransform CombinedTransform = convex.GetTransform() * BodyXForm;
				FBoxSphereBounds bounds = convex.ElemBox.TransformBy(CombinedTransform);

				FLinearColor debugDrawColor = FLinearColor::White;

				ABlastGlueVolume* MostOverlappingGlueVolume = nullptr;
				float MostOverlappingDistanceToBox = 0;
				//Find the glue volume which overlaps the most, (eg the bounding sphere overlaps to it and has the minimum distance to it's surface)
				//Could issue an error if the component overlaps multiple volumes, but that seems very annoying in practice
				for (ABlastGlueVolume* GlueVolume : Volumes)
				{
					float DistanceToBox = -1.0f;
					if (GlueVolume->EncompassesPoint(bounds.Origin, bounds.SphereRadius, &DistanceToBox))
					{
						//UEB-44 Sphere BB may lead to glueing chunks not in volume. So we need to test all chunk vertices
						bool isOverlapping = false;
						float Dist = DistanceToBox;
						for (auto& Vertex : convex.VertexData)
						{
							if (GlueVolume->EncompassesPoint(CombinedTransform.TransformPosition(Vertex), 0.f, &Dist) &&
								Dist <= DistanceToBox)
							{
								isOverlapping = Dist <= 0.f;
								DistanceToBox = Dist;
							}
						}
						if (isOverlapping && (MostOverlappingGlueVolume == nullptr || DistanceToBox <
							MostOverlappingDistanceToBox))
						{
							MostOverlappingGlueVolume = GlueVolume;
							MostOverlappingDistanceToBox = DistanceToBox;
						}
					}
				}

				if (MostOverlappingGlueVolume)
				{
					OverlappingChunks.Add(ChunkIndex);
					GlueVectors.Add(MostOverlappingGlueVolume->GlueVector);
					OverlappingVolumes.Add(MostOverlappingGlueVolume);

					if (bDrawDebug)
					{
						for (FVector& vertex : convex.VertexData)
						{
							FVector worldVertex = CombinedTransform.TransformPosition(vertex);
							if (MostOverlappingGlueVolume->EncompassesPoint(worldVertex))
							{
								debugDrawColor = FLinearColor::Red;
								::DrawDebugPoint(GetWorld(), worldVertex, 5, FColor::Yellow, true, 10.0f, 5);
							}
							else
							{
								::DrawDebugPoint(GetWorld(), worldVertex, 5, FColor::Cyan, true, 5.0f, 5);
							}
						}
					}
				}

				if (bDrawDebug)
				{
					::DrawDebugBox(GetWorld(), bounds.Origin, bounds.BoxExtent, FQuat::Identity,
					               debugDrawColor.QuantizeRound(), true, 5.0f, 0, 2);
				}
			}
		}
	}

	return OverlappingChunks.Num() > 0;
}

#endif

void UBlastMeshComponent::RefreshDynamicChunkBodyInstanceFromBodyInstance()
{
	//Save the collision related info since that's what's editable through the UI
	ECollisionEnabled::Type CollisionEnabled = DynamicChunkBodyInstance.GetCollisionEnabled();
	ECollisionChannel ObjectType = DynamicChunkBodyInstance.GetObjectType();
	FName CollsionProfileName = DynamicChunkBodyInstance.GetCollisionProfileName();
	FCollisionResponse CollisionResponse = DynamicChunkBodyInstance.GetCollisionResponse();

	DynamicChunkBodyInstance.CopyBodyInstancePropertiesFrom(&BodyInstance);

	DynamicChunkBodyInstance.SetCollisionEnabled(CollisionEnabled);
	DynamicChunkBodyInstance.SetObjectType(ObjectType);
	DynamicChunkBodyInstance.SetResponseToChannels(CollisionResponse.GetResponseContainer());
	//This must be done last or else it will invalidate the previous stuff
	DynamicChunkBodyInstance.SetCollisionProfileName(CollsionProfileName);
}


void UBlastMeshComponent::TickStressSolver()
{
	FVector Gravity;
#if BLAST_USE_PHYSX
	Gravity = GetPXScene()->getGravity();
#else
	Gravity = GetWorld()->GetPhysicsScene()->GetSolver()->GetEvolution()->GetGravityForces().GetAcceleration(0);
#endif

	// Apply all relevant forces on actors in stress solver
	FScopedSceneLock_Chaos Lock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Read);
	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		FActorData& ActorData = BlastActors[ActorIndex];
		NvBlastActor* actor = ActorData.BlastActor;
		if (!actor)
		{
			continue;
		}

		uint32 nodeCount = NvBlastActorGetGraphNodeCount(actor, Nv::Blast::logLL);
		if (nodeCount <= 1)
			// subsupport chunks don't have graph nodes and only 1 node actor doesn't make sense to be drawn
			continue;

		FBodyInstance* BodyInst = ActorData.BodyInstance;
		if (BodyInst->bSimulatePhysics)
		{
			// should we apply centrifugal force? Add a toggle-parameter setting here?
			const FVector LocalAngularVelocity = BodyInst->GetUnrealWorldTransform_AssumesLocked().
			                                               InverseTransformVectorNoScale(
				                                               BodyInst->
				                                               GetUnrealWorldAngularVelocityInRadians_AssumesLocked());
			StressSolver->addCentrifugalAcceleration(*actor, ToNvVector(BodyInst->GetMassSpaceLocal().GetLocation()),
			                                         ToNvVector(LocalAngularVelocity));
		}
		else
		{
			const FVector LocalGravity = BodyInst->GetUnrealWorldTransform_AssumesLocked().
			                                       InverseTransformVectorNoScale(Gravity);
			StressSolver->addGravity(*actor, ToNvVector(LocalGravity));
		}
	}
	Lock.Release();

	// Stress Solver update
	const auto& UsedStressProperties = GetUsedStressProperties();
	{
		Nv::Blast::ExtStressSolverSettings settings;
		settings.compressionElasticLimit = UsedStressProperties.CompressionElasticLimit;
		settings.compressionFatalLimit = UsedStressProperties.CompressionFatalLimit;
		settings.tensionElasticLimit = UsedStressProperties.TensionElasticLimit;
		settings.tensionFatalLimit = UsedStressProperties.TensionFatalLimit;
		settings.shearElasticLimit = UsedStressProperties.ShearElasticLimit;
		settings.shearFatalLimit = UsedStressProperties.ShearFatalLimit;
		settings.graphReductionLevel = UsedStressProperties.GraphReductionLevel;
		settings.maxSolverIterationsPerFrame = UsedStressProperties.MaxSolverIterationsPerFrame;
		StressSolver->setSettings(settings);

		StressSolver->update();
	}

	// For in editor Tick don't apply damage (only update() to show debug render)
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}
#endif

	// Break overstressed bonds
	if (StressSolver->getOverstressedBondCount() > 0)
	{
		for (int32 ActorIndex = 0; ActorIndex < ActorBodySetups.Num(); ActorIndex++)
		{
			FActorData& ActorData = BlastActors[ActorIndex];
			NvBlastActor* actor = ActorData.BlastActor;
			if (!actor)
			{
				continue;
			}

			uint32 nodeCount = NvBlastActorGetGraphNodeCount(actor, Nv::Blast::logLL);
			if (nodeCount > 1)
			{
				NvBlastFractureBuffers commands;
				StressSolver->generateFractureCommands(*actor, commands);
				if (commands.bondFractureCount > 0)
				{
					FName StressDamageType(TEXT("Stress"));
					ApplyFracture(ActorIndex, commands, StressDamageType);
					if (commands.bondFractureCount > 0)
					{
						struct ImpulseOnlyDamageProgram final : public FBlastBaseDamageProgram
						{
							float ImpulseStrength;
							float Radius;

							virtual bool Execute(uint32 actorIndex, FBodyInstance* actorBody, const FInput& input,
							                     UBlastMeshComponent& owner) const override { return false; }

							virtual void ExecutePostActorCreated(uint32 actorIndex, FBodyInstance* actorBody,
							                                     const FInput& input,
							                                     UBlastMeshComponent& owner) const override
							{
								actorBody->AddRadialImpulseToBody(FVector(input.worldOrigin), Radius, ImpulseStrength,
								                                  0, true);
							}
						};

						if (StressProperties.SplitImpulseStrength > 0.f)
						{
							// Apply radial force to all new actors from the COM of parent actor
							ImpulseOnlyDamageProgram ImpulseProgram;
							ImpulseProgram.Radius = ActorData.BodyInstance->GetBodyBounds().GetSize().GetMax();
							ImpulseProgram.ImpulseStrength = StressProperties.SplitImpulseStrength;
							FBlastBaseDamageProgram::FInput ProgramInput;
							ProgramInput.worldOrigin = FVector3f(ActorData.BodyInstance->GetCOMPosition());
							HandlePostDamage(actor, StressDamageType, &ImpulseProgram, &ProgramInput);
						}
						else
						{
							HandlePostDamage(actor, StressDamageType);
						}
					}
				}
			}
		}
	}
}

void UBlastMeshComponent::UpdateDebris()
{
	//destroy debris with inactive timer
	if (DebrisCount > 0)
	{
		for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
		{
			const FActorData& BlastActor = BlastActors[ActorIndex];
			if (BlastActor.BodyInstance && BlastActor.TimerHandle.IsValid())
			{
				if (!GetWorld()->GetTimerManager().IsTimerActive(BlastActor.TimerHandle))
				{
					FScopedSceneLock_Chaos Lock(GetWorld()->GetPhysicsScene(), EPhysicsInterfaceScopedLockType::Write);
					BreakDownBlastActor(ActorIndex);
				}
			}
		}
	}
}

void UBlastMeshComponent::UpdateDebris(int32 ActorIndex, const FTransform& ActorTransform,
                                       FScopedSceneLock_Chaos* SceneLock)
{
	check(SceneLock);

	const FBlastDebrisProperties& debrisProp = GetUsedDebrisProperties();
	if (debrisProp.DebrisFilters.IsEmpty())
	{
		return;
	}

	FActorData& BlastActor = BlastActors[ActorIndex];

	//skip empty BlastActors and BlastActors with countdown to destroy
	if (BlastActor.BodyInstance && BlastActor.Chunks.Num() && !BlastActor.TimerHandle.IsValid())
	{
		FBox AABB = ActorBodySetups[ActorIndex]->AggGeom.CalcAABB(ActorTransform);
		float lifetime = TNumericLimits<float>::Max();

		for (const FBlastDebrisFilter& filter : debrisProp.DebrisFilters)
		{
			bool isDebris = true;
			if (filter.bUseDebrisDepth)
			{
				uint32 depth = TNumericLimits<uint32>::Max();
				for (const FActorChunkData& ChunkData : BlastActor.Chunks)
				{
					depth = FMath::Min(BlastMesh->GetChunkDepth(ChunkData.ChunkIndex), depth);
				}
				isDebris &= filter.DebrisDepth <= depth;
			}
			if (filter.bUseDebrisMaxSeparation)
			{
				isDebris &= FVector::Dist(BlastActor.StartLocation, AABB.GetCenter()) > filter.DebrisMaxSeparation;
			}
			if (filter.bUseValidBounds)
			{
				isDebris &= !filter.ValidBounds.IsInside(AABB.GetCenter());
			}
			if (filter.bUseDebrisMaxSize)
			{
				isDebris &= AABB.GetExtent().GetAbsMax() * 2.f < filter.DebrisMaxSize;
			}

			if (isDebris && (filter.bUseDebrisDepth || filter.bUseDebrisMaxSeparation || filter.bUseValidBounds ||
				filter.bUseDebrisMaxSize))
			{
				if (filter.DebrisLifetimeMin < filter.DebrisLifetimeMax)
				{
					lifetime = FMath::Min(
						lifetime, FMath::RandRange(filter.DebrisLifetimeMin, filter.DebrisLifetimeMax));
				}
				else
				{
					lifetime = FMath::Min(lifetime, 0.5f * (filter.DebrisLifetimeMin + filter.DebrisLifetimeMax));
				}
				if (lifetime < 1e-2) //destroy debris immediately if its lifetime less then 0.01 s
				{
					SceneLock->Release();
					*SceneLock = FScopedSceneLock_Chaos(GetWorld()->GetPhysicsScene(),
					                                    EPhysicsInterfaceScopedLockType::Write);
					BreakDownBlastActor(ActorIndex);
					SceneLock->Release();
					*SceneLock = FScopedSceneLock_Chaos(GetWorld()->GetPhysicsScene(),
					                                    EPhysicsInterfaceScopedLockType::Read);
					break;
				}
			}
		}

		if (lifetime < TNumericLimits<float>::Max()) //activate lifetime timer for debris
		{
			GetWorld()->GetTimerManager().SetTimer(BlastActor.TimerHandle, lifetime, false);
			DebrisCount++;
		}
	}
}

#if WITH_EDITOR

inline FLinearColor bondHealthColor(float healthFraction)
{
	const FLinearColor BOND_HEALTHY_COLOR(0, 1.0f, 0, 1.0f);
	const FLinearColor BOND_MID_COLOR(1.0f, 1.0f, 0, 1.0f);
	const FLinearColor BOND_BROKEN_COLOR(1.0f, 0, 0, 1.0f);
	return healthFraction < 0.5
		       ? FMath::Lerp(BOND_BROKEN_COLOR, BOND_MID_COLOR, 2.0f * healthFraction)
		       : FMath::Lerp(BOND_MID_COLOR, BOND_HEALTHY_COLOR, 2.0f * healthFraction - 1.0f);
}

void UBlastMeshComponent::DrawDebugChunkCentroids()
{
	UBlastAsset* BlastAsset = GetBlastAsset();
	if (!BlastAsset)
	{
		return;
	}

	const NvBlastAsset* LLBlastAsset = BlastAsset->GetLoadedAsset();
	const NvBlastChunk* Chunks = NvBlastAssetGetChunks(LLBlastAsset, Nv::Blast::logLL);
	const uint32 ChunkCount = BlastAsset->GetChunkCount();
	TBitArray<> NeedsToDraw(true, ChunkCount);

	//Bond centroids are always in mesh-relative worldspace, not bone space, but in the original position of the mesh
	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		FActorData& ActorData = BlastActors[ActorIndex];
		NvBlastActor* actor = ActorData.BlastActor;
		if (!actor)
		{
			continue;
		}
		FTransform RestSpaceToWorldSpace = ActorData.BodyInstance->GetUnrealWorldTransform();
		RestSpaceToWorldSpace.SetScale3D(ActorData.BodyInstance->Scale3D);

		for (const FActorChunkData& Chunk : ActorData.Chunks)
		{
			const NvBlastChunk& LLChunk = Chunks[Chunk.ChunkIndex];
			NeedsToDraw[Chunk.ChunkIndex] = false;
			uint32 Parent = LLChunk.parentChunkIndex;
			while (Parent != ~0U)
			{
				NeedsToDraw[Parent] = false;
				Parent = Chunks[Parent].parentChunkIndex;
			}

			FVector worldCentroid = RestSpaceToWorldSpace.TransformPosition(FromNvVector(LLChunk.centroid));
			DrawDebugBox(worldCentroid, FVector(10.0f),
			             ActorData.bIsAttachedToComponent ? FLinearColor::White : FLinearColor::Green);
		}
	}

	for (TConstSetBitIterator<> It(NeedsToDraw); It; ++It)
	{
		const NvBlastChunk& LLChunk = Chunks[It.GetIndex()];
		FVector worldCentroid = GetComponentTransform().TransformPosition(FromNvVector(LLChunk.centroid));
		DrawDebugBox(worldCentroid, FVector(10.0f), FLinearColor::Blue);
	}
}

void UBlastMeshComponent::DrawDebugSupportGraph()
{
	UBlastAsset* BlastAsset = GetBlastAsset();
	if (!BlastAsset)
	{
		return;
	}

	const NvBlastAsset* LLBlastAsset = BlastAsset->GetLoadedAsset();
	const NvBlastBond* bonds = NvBlastAssetGetBonds(LLBlastAsset, Nv::Blast::logLL);
	const NvBlastSupportGraph graph = NvBlastAssetGetSupportGraph(LLBlastAsset, Nv::Blast::logLL);
	const uint32 chunkCount = BlastAsset->GetChunkCount();

	// TODO: better scan all bonds once somewhere
	const float bondHealthMax = 1.0f;

	const FLinearColor BOND_NORMAL_COLOR(0, 204, 255, 255);
	const FLinearColor BOND_INVISIBLE_COLOR(166, 41, 41, 255);

	//Bond centroids are always in mesh-relative worldspace, not bone space, but in the original position of the mesh
	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		FActorData& ActorData = BlastActors[ActorIndex];
		NvBlastActor* actor = ActorData.BlastActor;
		if (!actor)
		{
			continue;
		}

		uint32 nodeCount = NvBlastActorGetGraphNodeCount(actor, Nv::Blast::logLL);
		if (nodeCount <= 1)
			// subsupport chunks don't have graph nodes and only 1 node actor doesn't make sense to be drawn
			continue;

		FTransform RestSpaceToWorldSpace = ActorData.BodyInstance->GetUnrealWorldTransform();
		RestSpaceToWorldSpace.SetScale3D(ActorData.BodyInstance->Scale3D);

		TArray<uint32> Nodes;
		Nodes.SetNum(nodeCount);
		nodeCount = NvBlastActorGetGraphNodeIndices(Nodes.GetData(), nodeCount, actor, Nv::Blast::logLL);

		const float* bondHealths = NvBlastActorGetBondHealths(actor, Nv::Blast::logLL);

		for (uint32 i = 0; i < nodeCount; i++)
		{
			const uint32 node0 = Nodes[i];
			const uint32 chunkIndex0 = graph.chunkIndices[node0];

			for (uint32 adjacencyIndex = graph.adjacencyPartition[node0]; adjacencyIndex < graph.adjacencyPartition[
				     node0 + 1]; adjacencyIndex++)
			{
				const uint32 node1 = graph.adjacentNodeIndices[adjacencyIndex];
				const uint32 chunkIndex1 = graph.chunkIndices[node1];
				if (node0 > node1)
					continue;

				bool invisibleBond = chunkIndex0 >= chunkCount || chunkIndex1 >= chunkCount;

				// health
				uint32 bondIndex = graph.adjacentBondIndices[adjacencyIndex];
				float healthVal = FMath::Clamp(bondHealths[bondIndex] / bondHealthMax, 0.0f, 1.0f);

				const FLinearColor color = bondHealthColor(healthVal);

				const NvBlastBond& solverBond = bonds[bondIndex];
				const FVector centroid(FromNvVector(solverBond.centroid));

				// centroid
				if (true)
				{
					FVector worldCentroid = RestSpaceToWorldSpace.TransformPosition(centroid);
					float extent = FMath::Sqrt(solverBond.area) * 0.5f; // approximation
					extent /= 10.0f; // scale down for visual candy
					DrawDebugBox(worldCentroid, FVector(extent), invisibleBond ? BOND_INVISIBLE_COLOR : color);
				}

				// chunk connection (bond)
				if (!invisibleBond)
				{
					DrawDebugLine(
						RestSpaceToWorldSpace.TransformPosition(
							FromNvVector(BlastAsset->GetChunkInfo(chunkIndex0).centroid)),
						RestSpaceToWorldSpace.TransformPosition(
							FromNvVector(BlastAsset->GetChunkInfo(chunkIndex1).centroid)),
						color);
				}
			}
		}
	}
}

FLinearColor UnpackColor(uint32 color)
{
	const FLinearColor LinColor{FColor(color)};
	return FLinearColor(LinColor.B, LinColor.G, LinColor.R, LinColor.A);
}

void UBlastMeshComponent::DrawDebugStressGraph()
{
	if (!StressSolver || StressDebugMode == EBlastStressDebugRenderMode::None)
	{
		return;
	}

	TArray<uint32> Nodes;
	for (int32 ActorIndex = BlastActorsBeginLive; ActorIndex < BlastActorsEndLive; ActorIndex++)
	{
		FActorData& ActorData = BlastActors[ActorIndex];
		NvBlastActor* actor = ActorData.BlastActor;
		if (!actor)
		{
			continue;
		}

		uint32 nodeCount = NvBlastActorGetGraphNodeCount(actor, Nv::Blast::logLL);
		if (nodeCount <= 1)
			// subsupport chunks don't have graph nodes and only 1 node actor doesn't make sense to be drawn
			continue;

		Nodes.SetNum(nodeCount);
		nodeCount = NvBlastActorGetGraphNodeIndices(Nodes.GetData(), static_cast<uint32>(Nodes.Max()), actor,
		                                            Nv::Blast::logLL);

		FTransform RestSpaceToWorldSpace = ActorData.BodyInstance->GetUnrealWorldTransform();
		RestSpaceToWorldSpace.SetScale3D(ActorData.BodyInstance->Scale3D);

		const Nv::Blast::ExtStressSolver::DebugBuffer Buf = StressSolver->fillDebugRender(
			Nodes.GetData(), nodeCount,
			static_cast<Nv::Blast::ExtStressSolver::DebugRenderMode>(static_cast<int>(StressDebugMode) - 1), 0.01f);

		for (uint32 Idx = 0; Idx < Buf.lineCount; Idx++)
		{
			const Nv::Blast::ExtStressSolver::DebugLine& line = Buf.lines[Idx];
			const FLinearColor color = FMath::Lerp(UnpackColor(line.color0), UnpackColor(line.color1), 0.5f);
			const FVector p0 = RestSpaceToWorldSpace.TransformPosition(FromNvVector(line.pos0));
			const FVector p1 = RestSpaceToWorldSpace.TransformPosition(FromNvVector(line.pos1));
			DrawDebugLine(p0, p1, color);
		}
	}
}

void UBlastMeshComponent::DrawDebugLine(FVector const& LineStart, FVector const& LineEnd, FLinearColor const& Color,
                                        uint8 DepthPriority /*= 0*/, float Thickness /*= 0.f*/)
{
	//We don't use the lifetime member
	PendingDebugLines.Emplace(LineStart, LineEnd, Color, 0, Thickness, DepthPriority);
}

void UBlastMeshComponent::DrawDebugBox(FVector const& Center, FVector const& Extent, FLinearColor const& Color,
                                       uint8 DepthPriority /*= 0*/, float Thickness /*= 0.f*/)
{
	DrawDebugLine(Center + FVector(Extent.X, Extent.Y, Extent.Z), Center + FVector(Extent.X, -Extent.Y, Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(Extent.X, -Extent.Y, Extent.Z), Center + FVector(-Extent.X, -Extent.Y, Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(-Extent.X, -Extent.Y, Extent.Z), Center + FVector(-Extent.X, Extent.Y, Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(-Extent.X, Extent.Y, Extent.Z), Center + FVector(Extent.X, Extent.Y, Extent.Z),
	              Color, DepthPriority, Thickness);

	DrawDebugLine(Center + FVector(Extent.X, Extent.Y, -Extent.Z), Center + FVector(Extent.X, -Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(Extent.X, -Extent.Y, -Extent.Z), Center + FVector(-Extent.X, -Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z), Center + FVector(-Extent.X, Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(-Extent.X, Extent.Y, -Extent.Z), Center + FVector(Extent.X, Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);

	DrawDebugLine(Center + FVector(Extent.X, Extent.Y, Extent.Z), Center + FVector(Extent.X, Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(Extent.X, -Extent.Y, Extent.Z), Center + FVector(Extent.X, -Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(-Extent.X, -Extent.Y, Extent.Z), Center + FVector(-Extent.X, -Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
	DrawDebugLine(Center + FVector(-Extent.X, Extent.Y, Extent.Z), Center + FVector(-Extent.X, Extent.Y, -Extent.Z),
	              Color, DepthPriority, Thickness);
}

void UBlastMeshComponent::DrawDebugPoint(FVector const& Position, float Size, FLinearColor const& PointColor,
                                         uint8 DepthPriority /*= 0*/)
{
	//We don't use the lifetime member
	PendingDebugPoints.Emplace(Position, PointColor, Size, 0, DepthPriority);
}

#endif

const FName UBlastMeshComponent::ActorBaseName("Actor");

FPrimitiveSceneProxy* UBlastMeshComponent::CreateSceneProxy()
{
	ERHIFeatureLevel::Type SceneFeatureLevel = GetWorld()->GetFeatureLevel();
	FBlastMeshSceneProxy* Result = nullptr;
	FSkeletalMeshRenderData* SkelMeshRenderData = GetSkeletalMeshRenderData();

	if (ShouldRender() && SkelMeshRenderData &&
		SkelMeshRenderData->LODRenderData.IsValidIndex(GetPredictedLODLevel()) &&
		!bHideSkin &&
		MeshObject)
	{
		// Only create a scene proxy if the bone count being used is supported, or if we don't have a skeleton (this is the case with destructibles)
		int32 MaxBonesPerChunk = SkelMeshRenderData->GetMaxBonesPerSection();
		if (MaxBonesPerChunk <= FGPUBaseSkinVertexFactory::GetMaxGPUSkinBones())
		{
			Result = ::new FBlastMeshSceneProxy(this, SkelMeshRenderData);
		}
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	SendRenderDebugPhysics(Result);
#endif
	BlastProxy = Result;
	return Result;
}

FBlastMeshSceneProxy::FBlastMeshSceneProxy(const UBlastMeshComponent* Component,
                                           FSkeletalMeshRenderData* InSkelMeshRenderData) :
	FBlastMeshSceneProxyBase(Component),
	FSkeletalMeshSceneProxy(Component, InSkelMeshRenderData)
{
	PhysicsAssetForDebug = Component->GetBlastMesh()->PhysicsAsset;

	const UBlastMeshComponent* BlastMeshComponent = Cast<const UBlastMeshComponent>(Component);
	if (BlastMeshComponent && BlastMeshComponent->bPerBoneMotionBlur)
	{
		bAlwaysHasVelocity = true;
	}
}

void FBlastMeshSceneProxy::DebugDrawPhysicsAsset(int32 ViewIndex, FMeshElementCollector& Collector,
                                                 const FEngineShowFlags& EngineShowFlags) const
{
	FMatrix ProxyLocalToWorld, WorldToLocal;
	if (!GetWorldMatrices(ProxyLocalToWorld, WorldToLocal))
	{
		return; // Cannot draw this, world matrix not valid
	}

	const TArray<FTransform>* BoneSpaceBases = FSkeletalMeshSceneProxy::MeshObject->GetComponentSpaceTransforms();
	RenderPhysicsAsset(ViewIndex, Collector, EngineShowFlags, ProxyLocalToWorld, BoneSpaceBases);
}

void FBlastMeshSceneProxyBase::RenderPhysicsAsset(int32 ViewIndex, FMeshElementCollector& Collector,
                                                  const FEngineShowFlags& EngineShowFlags,
                                                  const FMatrix& ProxyLocalToWorld,
                                                  const TArray<FTransform>* BoneSpaceBases) const
{
	FMatrix ScalingMatrix = ProxyLocalToWorld;
	FVector TotalScale = ScalingMatrix.ExtractScaling();

	// Only if valid
	if (!TotalScale.IsNearlyZero())
	{
		FTransform LocalToWorldTransform(ProxyLocalToWorld);
		const auto& ChunkIndexToBoneIndex = BlastMeshForDebug->ChunkIndexToBoneIndex;
		const auto& CookedChunkData = BlastMeshForDebug->GetCookedChunkData_AssumeUpToDate();

		if (BoneSpaceBases && EngineShowFlags.Collision)
		{
			for (TConstSetBitIterator<> It(VisibleChunks); It; ++It)
			{
				if (ChunkIndexToBoneIndex.IsValidIndex(It.GetIndex()))
				{
					int32 BoneIndex = ChunkIndexToBoneIndex[It.GetIndex()];
					if (BoneSpaceBases->IsValidIndex(BoneIndex))
					{
						FTransform BoneTransform = BlastMeshForDebug->GetComponentSpaceInitialBoneTransform(BoneIndex) *
							(*BoneSpaceBases)[BoneIndex] * LocalToWorldTransform;
						CookedChunkData[It.GetIndex()].CookedBodySetup->CreatePhysicsMeshes();
						CookedChunkData[It.GetIndex()].CookedBodySetup->AggGeom.GetAggGeom(
							BoneTransform, FColor::Orange, nullptr, false, false, false, ViewIndex, Collector);
					}
				}
			}
		}
	}
}

#if WITH_EDITOR
void FBlastMeshSceneProxyBase::RenderDebugLines(const TArray<const FSceneView*>& Views,
                                                const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
                                                FMeshElementCollector& Collector) const
{
	if (DebugDrawLines.Num() > 0 || DebugDrawPoints.Num() > 0)
	{
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
			//Most of our lines are normal so reserve that, it doesn't need to be exact
			PDI->AddReserveLines(0, DebugDrawLines.Num());
			for (const FBatchedLine& Line : DebugDrawLines)
			{
				PDI->DrawLine(Line.Start, Line.End, Line.Color, Line.DepthPriority, Line.Thickness);
			}

			for (const FBatchedPoint& Point : DebugDrawPoints)
			{
				PDI->DrawPoint(Point.Position, Point.Color, Point.PointSize, Point.DepthPriority);
			}
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
