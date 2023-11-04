// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BlastFracture.h"
#include "BlastFractureSettings.h"
#include "BlastMeshEditorModule.h"
#include "BlastMesh.h"
#include "BlastMeshFactory.h"
#include "BlastMeshUtilities.h"
#include "BlastGlobals.h"

#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringTypes.h"
#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoringConvexMeshBuilder.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlast.h"
#include "NvBlastGlobals.h"

#include "Misc/ScopedSlowTask.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "Math/RandomStream.h"
#include "Misc/ScopeLock.h"

#include "ComponentReregisterContext.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Components/SkinnedMeshComponent.h"
#include "RawMesh.h"
#include "Engine/Texture2D.h"
#include "MeshDescriptionOperations.h"

#include "Chaos/Core.h"
#include "Chaos/Convex.h"

#define LOCTEXT_NAMESPACE "BlastMeshEditor"

#if UE_BUILD_DEVELOPMENT
#include <ctime>

class FBlastScopedProfiler
{
	std::clock_t Clock;
	double Duration = 0;
	bool IsStarted = false;
	FString Name;

public:
	FBlastScopedProfiler(const FString& InName, bool ShouldStart = true)
	{
		Name = InName;
		if (ShouldStart)
		{
			Start();
		}
	}

	~FBlastScopedProfiler()
	{
		Stop();
		UE_LOG(LogBlastMeshEditor, Verbose, TEXT("Blast Profiler: %s - Execution time, s: %f"), *Name,
		       Duration / (double)CLOCKS_PER_SEC);
	}

	void Start()
	{
		if (IsStarted)
		{
			Stop();
		}
		Clock = std::clock();
		IsStarted = true;
	}

	void Stop()
	{
		if (IsStarted)
		{
			Duration += (std::clock() - Clock); // (double)CLOCKS_PER_SEC;
			IsStarted = false;
		}
	}
};
#else
class FBlastScopedProfiler
{
public:
	FBlastScopedProfiler(const FString& InName, bool ShouldStart = true) {}
	void Start() {}
	void Stop() {}
};
#endif

#define SAFE_ARRAY_NEW(T, x) ((x) > 0) ? reinterpret_cast<T*>(NVBLAST_ALLOC(sizeof(T) * (x))) : nullptr;
#define SAFE_ARRAY_DELETE(x)                                                                                           \
	if (x != nullptr)                                                                                                  \
	{                                                                                                                  \
		NVBLAST_FREE(x);                                                                                               \
		x = nullptr;                                                                                                   \
	}

struct FCollisionHull : public Nv::Blast::CollisionHull
{
	FCollisionHull()
	{
	};

	FCollisionHull(const CollisionHull& hullToCopy)
	{
		pointsCount = hullToCopy.pointsCount;
		indicesCount = hullToCopy.indicesCount;
		polygonDataCount = hullToCopy.polygonDataCount;

		points = SAFE_ARRAY_NEW(NvcVec3, pointsCount);
		indices = SAFE_ARRAY_NEW(uint32, indicesCount);
		polygonData = SAFE_ARRAY_NEW(Nv::Blast::HullPolygon, polygonDataCount);
		FMemory::Memcpy(points, hullToCopy.points, sizeof(NvcVec3) * pointsCount);
		FMemory::Memcpy(indices, hullToCopy.indices, sizeof(uint32) * indicesCount);
		FMemory::Memcpy(polygonData, hullToCopy.polygonData, sizeof(Nv::Blast::HullPolygon) * polygonDataCount);
	}

	~FCollisionHull()
	{
		SAFE_ARRAY_DELETE(points);
		SAFE_ARRAY_DELETE(indices);
		SAFE_ARRAY_DELETE(polygonData);
	}
};


class FCollisionBuilder : public Nv::Blast::ConvexMeshBuilder
{
public:
	FCollisionBuilder()
	{
#if BLAST_USE_PHYSX
		mCooking = GetPhysXCookingModule()->GetPhysXCooking()->GetCooking();
		mInsertionCallback = &GPhysXSDK->getPhysicsInsertionCallback();
#endif
	}

	virtual ~FCollisionBuilder()
	{
	}

	Nv::Blast::CollisionHull* buildCollisionGeometry(uint32 verticesCount, const NvcVec3* vData) override
	{
		// Create the corner vertices for the convex
		TArray<Chaos::FConvex::FVec3Type> ConvexVertices;
		ConvexVertices.AddUninitialized(verticesCount);
		FMemory::Memcpy(ConvexVertices.GetData(), vData, sizeof(Chaos::FConvex::FVec3Type) * verticesCount);

		// Margin is always zero on convex shapes - they are intended to be instanced
		const Chaos::FConvex GeneratedHull(ConvexVertices, 0.0f);

		Nv::Blast::CollisionHull* output = new FCollisionHull();

		output->pointsCount = GeneratedHull.NumVertices();
		output->points = SAFE_ARRAY_NEW(NvcVec3, output->pointsCount);
		FMemory::Memcpy(output->points, GeneratedHull.GetVertices().GetData(),
		                sizeof(Chaos::FConvex::FVec3Type) * output->pointsCount);

		output->polygonDataCount = GeneratedHull.NumPlanes();
		output->polygonData = SAFE_ARRAY_NEW(Nv::Blast::HullPolygon, output->polygonDataCount);

		TArray<int32> Indices;
		Indices.Reserve(3 * output->polygonDataCount); // at minimum, we will have this many indices
		for (uint32 PlaneIdx = 0; PlaneIdx < output->polygonDataCount; ++PlaneIdx)
		{
			Nv::Blast::HullPolygon& HullPoly = output->polygonData[PlaneIdx];

			HullPoly.vertexCount = GeneratedHull.NumPlaneVertices(PlaneIdx);
			HullPoly.indexBase = Indices.Num();
			Indices.AddUninitialized(HullPoly.vertexCount);

			const Chaos::FConvex::FPlaneType& ChaosPlane = GeneratedHull.GetFaces()[PlaneIdx];
			for (int32 VertIdx = 0; VertIdx < HullPoly.vertexCount; VertIdx++)
			{
				Indices[HullPoly.indexBase + VertIdx] = GeneratedHull.GetPlaneVertex(PlaneIdx, VertIdx);
			}
			ToNvPlane4(FPlane4f(ChaosPlane.X(), ChaosPlane.Normal()), HullPoly.plane);
		}

		output->indicesCount = Indices.Num();
		output->indices = SAFE_ARRAY_NEW(uint32, output->indicesCount);
		FMemory::Memcpy(output->indices, Indices.GetData(), sizeof(uint32) * output->indicesCount);

		return output;
	}

	void releaseCollisionHull(Nv::Blast::CollisionHull* hull) const override
	{
		if (hull)
		{
			SAFE_ARRAY_DELETE(hull->indices);
			SAFE_ARRAY_DELETE(hull->points);
			SAFE_ARRAY_DELETE(hull->polygonData);
			delete hull;
		}
	}

	void release() override
	{
	}

private:
#if BLAST_USE_PHYSX
	physx::PxCooking* mCooking;
	physx::PxPhysicsInsertionCallback* mInsertionCallback;
#endif
};


const FName FBlastFracture::InteriorMaterialID("Interior_Material");

inline int32 GetFractureChunkId(TSharedPtr<FFractureSession> session, uint32 ChunkIndex)
{
	if (session->IsRootFractured)
	{
		return ChunkIndex ? session->FractureData->assetToFractureChunkIdMap[ChunkIndex] : 0;
	}
	return ChunkIndex;
}

inline int32 GetChunkToolIndexFromSessionIndex(TSharedPtr<FFractureSession> session, int32 ChunkAssetIndex)
{
	const int32 ChunkId = GetFractureChunkId(session, ChunkAssetIndex);
	if (ChunkId < 0)
	{
		return -1;
	}
	return session->FractureTool->getChunkInfoIndex(ChunkId);
}


//////////////////////////////////////////////////////////////////////////
// FFractureRandomGenerator
//////////////////////////////////////////////////////////////////////////

class FFractureRandomGenerator : public Nv::Blast::RandomGeneratorBase
{
	FRandomStream RStream;

public:
	float getRandomValue()
	{
		return RStream.GetFraction();
	}

	void seed(int32 seed)
	{
		if (seed < 0)
		{
			RStream.GenerateNewSeed();
		}
		else
		{
			RStream.Initialize(seed);
		}
	}
};


FBlastFracture::FBlastFracture()
{
	RandomGenerator = MakeShareable(new FFractureRandomGenerator());
	Config = NewObject<UBlastFractureSettingsConfig>();
}

FBlastFracture::~FBlastFracture()
{
}

UBlastFractureSettings* FBlastFracture::CreateFractureSettings(class FBlastMeshEditor* Editor) const
{
	UBlastFractureSettings* Settings = NewObject<UBlastFractureSettings>();
	Settings->BlastMeshEditor = Editor;
	return Settings;
}

TSharedPtr<FFractureSession> FBlastFracture::StartFractureSession(UBlastMesh* InBlastMesh,
                                                                  UStaticMesh* InSourceStaticMesh,
                                                                  UBlastFractureSettings* Settings,
                                                                  bool ForceLoadFracturedMesh)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	TSharedPtr<FFractureSession> FractureSession = MakeShareable(new FFractureSession());
	FractureSession->FractureTool = TSharedPtr<Nv::Blast::FractureTool>(NvBlastExtAuthoringCreateFractureTool(),
	                                                                    [](Nv::Blast::FractureTool* p)
	                                                                    {
		                                                                    p->release();
	                                                                    });
	FractureSession->BlastMesh = InBlastMesh;
	if (!FractureSession->FractureTool.IsValid() || InBlastMesh == nullptr)
	{
		UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to start fracture session"));
		return nullptr;
	}

	FTransform3f UE4ToBlastTransform;

	if (InSourceStaticMesh != nullptr)
	{
		UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(nullptr);

		// Fill required material map
		TMap<FName, int32> MaterialMap;
		for (int32 MaterialIndex = 0; MaterialIndex < InSourceStaticMesh->GetStaticMaterials().Num(); ++MaterialIndex)
		{
			MaterialMap.Add(InSourceStaticMesh->GetStaticMaterials()[MaterialIndex].ImportedMaterialSlotName,
			                MaterialIndex);
		}

		Nv::Blast::Mesh* Mesh = nullptr;
		/* Experimental
		if (FStaticMeshRenderData* RenderData = InSourceStaticMesh->GetRenderData())
		{
			Mesh = CreateAuthoringMeshFromRenderData(*RenderData, MaterialMap,
			                                         UE4ToBlastTransform);
		}
		*/

		if (!Mesh)
		{
			FRawMesh InSourceRawMesh;
			FStaticMeshOperations::ConvertToRawMesh(*InSourceStaticMesh->GetMeshDescription(0), InSourceRawMesh,
			                                        MaterialMap);

			BuildSmoothingGroups(InSourceRawMesh);
			//Retrieve mesh just assign default smoothing group 1 for each face. So we need to generate it.

			Mesh = CreateAuthoringMeshFromRawMesh(InSourceRawMesh, UE4ToBlastTransform);
		}

		FractureSession->FractureTool->setChunkMesh(Mesh, -1);

		LoadFracturedMesh(FractureSession, -1, InSourceStaticMesh);

		//need this to force saving fracture tool state
		FractureSession->IsRootFractured = true;
		FractureSession->IsMeshCreatedFromFractureData = true;
	}
	else
	{
		FBlastScopedProfiler LFTSP("Load fracture tool state");
		FBlastFractureToolData& FTD = InBlastMesh->FractureHistory.GetCurrentToolData();
		const int32 SupportLevel = Settings->bDefaultSupportDepth ? Settings->DefaultSupportDepth : -1;
		const uint32 ChunkCount = FTD.ChunkMeshes.Num();

		if (ForceLoadFracturedMesh || (ChunkCount == InBlastMesh->GetChunkCount() && ChunkCount))
		{
			for (uint32 ChunkIndex = 0; ChunkIndex < ChunkCount; ChunkIndex++)
			{
				const FBlastChunkMesh& Chunk = FTD.ChunkMeshes[ChunkIndex];
				Nv::Blast::Mesh* ChunkMesh = NvBlastExtAuthoringCreateMeshFromFacets(Chunk.Vertices.GetData(),
					Chunk.Edges.GetData(), Chunk.Faces.GetData(), Chunk.Vertices.Num(), Chunk.Edges.Num(),
					Chunk.Faces.Num());

				int32 chunkID = InBlastMesh->GetChunkInfo(ChunkIndex).userData;
				int32 parentChunkId = InBlastMesh->GetChunkInfo(ChunkIndex).parentChunkIndex;
				if (parentChunkId >= 0) // need to convert to chunk ID - not index!!
				{
					parentChunkId = InBlastMesh->GetChunkInfo(parentChunkId).userData;
				}

				FractureSession->FractureTool->setChunkMesh(ChunkMesh, parentChunkId, chunkID);

				FractureSession->FractureTool->setApproximateBonding(
					ChunkIndex, EnumHasAnyFlags(Chunk.ChunkFlag, EBlastMeshChunkFlags::ApproximateBonding));
			}
		}
		else
		//If there is no saved fracture tool state, we can load chunks from SkeletalMesh. Note: smoothing groups will be lost.
		{
			UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(
				Cast<UFbxSkeletalMeshImportData>(InBlastMesh->Mesh->GetAssetImportData()));
			TArray<FRawMesh> RawMeshes;
			RawMeshes.SetNum(InBlastMesh->GetChunkCount());
			{
				FBlastScopedProfiler SCMP("GetRenderMesh");
				InBlastMesh->GetRenderMesh(0, RawMeshes);
			}
			for (int32 ChunkIdx = 0; ChunkIdx < RawMeshes.Num(); ChunkIdx++)
			{
				Nv::Blast::Mesh* ChunkMesh = CreateAuthoringMeshFromRawMesh(RawMeshes[ChunkIdx], UE4ToBlastTransform);

				int32 parentChunkId = InBlastMesh->GetChunkInfo(ChunkIdx).parentChunkIndex;
				if (parentChunkId >= 0) // need to convert to chunk ID - not index!!
				{
					parentChunkId = InBlastMesh->GetChunkInfo(parentChunkId).userData;
				}
				FractureSession->FractureTool->setChunkMesh(ChunkMesh, parentChunkId,
				                                            (int32)InBlastMesh->GetChunkInfo(ChunkIdx).userData);
			}
		}
		if (ForceLoadFracturedMesh)
		{
			LoadFracturedMesh(FractureSession, SupportLevel, nullptr);
		}
		else if (!FractureSession->IsRootFractured)
		{
			LoadFractureData(FractureSession, SupportLevel, nullptr);
			if (FTD.ChunkMeshes.IsEmpty())
			// This is a way of dealing with old assets that have no fracture tool data serialized
			{
				InBlastMesh->FractureHistory.Collapse(); // Reduce history to one entry
				const uint32 ToolChunkCount = FractureSession->FractureTool->getChunkCount();
				for (uint32 ChunkIndex = 0; ChunkIndex < ToolChunkCount; ++ChunkIndex)
				{
					FractureSession->FractureTool->setApproximateBonding(ChunkIndex, true);
				}
				LoadFractureToolData(FractureSession);
			}
		}
	}

	if (Settings)
	{
		PopulateSettingsFromBlastMesh(Settings, FractureSession->BlastMesh);
	}

	return FractureSession;
}

void FBlastFracture::FinishFractureSession(FFractureSessionPtr FractureSession)
{
	auto FS = FractureSession.Pin();
	if (FS->IsRootFractured && FS->IsMeshCreatedFromFractureData)
	{
		{
			FBlastScopedProfiler SOMP("Save optimized mesh");

			TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

			if (FS->BlastMesh->Mesh)
			{
				FS->BlastMesh->Mesh->ReleaseResources();
				FS->BlastMesh->Mesh->ReleaseResourcesFence.Wait();
			}
			CreateSkeletalMeshFromAuthoring(FS, true, nullptr);

			FS->BlastMesh->RebuildIndexToBoneNameMap();
			FS->BlastMesh->RebuildCookedBodySetupsIfRequired(true);

			FS->BlastMesh->PostLoad();
		}
		if (FS->FractureData.IsValid())
		{
			LoadFractureToolData(FS);
		}
	}
}

void FBlastFracture::GetVoronoiSites(TSharedPtr<FFractureSession> FractureSession, int32 ChunkAssetIdx,
                                     TArray<FVector3f>& Sites)
{
	Sites.Empty();
	if (!FractureSession.IsValid() || !FractureSession->IsRootFractured)
	{
		return;
	}

	const auto Generator = FractureSession->SitesGeneratorMap.Find(GetFractureChunkId(FractureSession, ChunkAssetIdx));
	if (!Generator || !Generator->Generator.IsValid())
	{
		return;
	}
	const NvcVec3* sites = nullptr;
	int32 sitesCount = Generator->Generator->getVoronoiSites(sites);
	Sites.SetNumUninitialized(sitesCount);
	FMemory::Memcpy(Sites.GetData(), sites, sizeof(FVector3f) * sitesCount);

	auto SkelMeshImportData = Cast<UFbxSkeletalMeshImportData>(FractureSession->BlastMesh->Mesh->GetAssetImportData());
	FTransform3f Converter = UBlastMeshFactory::GetTransformBlastToUE4CoordinateSystem(SkelMeshImportData);
	for (FVector3f& p : Sites)
	{
		p = Converter.TransformPosition(p);
	}
}

bool FBlastFracture::CanUndo(UBlastFractureSettings* Settings) const
{
	if (Settings == nullptr || !Settings->FractureSession.IsValid())
	{
		return false;
	}

	return Settings->FractureSession->BlastMesh->FractureHistory.CanUndo();
}

void FBlastFracture::Undo(UBlastFractureSettings* Settings)
{
	if (Settings == nullptr || !Settings->FractureSession.IsValid())
	{
		return;
	}

	UBlastMesh* BlastMesh = Settings->FractureSession->BlastMesh;

	FinishFractureSession(Settings->FractureSession);
	BlastMesh->FractureHistory.Undo();
	Settings->FractureSession = StartFractureSession(BlastMesh, nullptr, Settings, true);
}

bool FBlastFracture::CanRedo(UBlastFractureSettings* Settings) const
{
	if (Settings == nullptr || !Settings->FractureSession.IsValid())
	{
		return false;
	}
	return Settings->FractureSession->BlastMesh->FractureHistory.CanRedo();
}

void FBlastFracture::Redo(UBlastFractureSettings* Settings)
{
	if (Settings == nullptr || !Settings->FractureSession.IsValid())
	{
		return;
	}

	UBlastMesh* BlastMesh = Settings->FractureSession->BlastMesh;

	FinishFractureSession(Settings->FractureSession);
	BlastMesh->FractureHistory.Redo();
	Settings->FractureSession = StartFractureSession(BlastMesh, nullptr, Settings, true);
}

void FBlastFracture::Fracture(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices,
                              int32 ClickedChunkIndex)
{
	FBlastScopedProfiler FP("Fracture");
	FScopeLock Lock(&ExclusiveFractureSection);

	if (Settings == nullptr || !Settings->FractureSession.IsValid())
	{
		return;
	}
	auto FractureSession = Settings->FractureSession;

	auto InteriorMaterial = Settings->InteriorMaterial;
	if (Settings->InteriorMaterialSlotName == NAME_None)
	{
		FractureSession->FractureTool->setInteriorMaterialId(Nv::Blast::kMaterialInteriorId);
	}
	else
	{
		int32 InteriorMatID = Nv::Blast::kMaterialInteriorId;
		const auto& MatList = FractureSession->BlastMesh->Mesh->GetMaterials();
		for (int32 I = 0; I < MatList.Num(); I++)
		{
			const FSkeletalMaterial& Mat = MatList[I];
			if ((Mat.ImportedMaterialSlotName.IsNone() ? Mat.MaterialSlotName : Mat.ImportedMaterialSlotName) ==
				Settings->InteriorMaterialSlotName)
			{
				InteriorMatID = I;
				InteriorMaterial = MatList[I].MaterialInterface;
				if (InteriorMaterial != nullptr)
				{
					break;
				}
			}
		}
		FractureSession->FractureTool->setInteriorMaterialId(InteriorMatID);
	}

	if (ClickedChunkIndex != INDEX_NONE)
	{
		if (SelectedChunkIndices.Num() == 0)
		{
			SelectedChunkIndices.Reset();
			SelectedChunkIndices.Add(ClickedChunkIndex);
		}

		//case 1: clicked chunk is child of SelectedChunk and SelectedChunk the same as last fracture chunk - continue fracturing
		//case 2: otherwise - finallize previous chunk fracture and start new one for clicked chunk
		//if (!FractureSession->FractureData.IsValid() 
		//	|| FractureSession->FractureData->chunkDescs[ClickedChunkIndex].parentChunkIndex != SelectedChunkIndices[0]
		//	|| FractureSession->LastFractureId != GetFractureChunkId(FractureSession, SelectedChunkIndices[0]))
		//{
		//	FractureSession->LastFractureId = -1;
		//	SelectedChunkIndices[0] = ClickedChunkIndex;
		//}
	}
	else
	{
		//FractureSession->LastFractureId = -1;
	}
	//SelectedChunkIndices.Sort(); NOTE: Don't know is it really need here. Looks like fracture works correctly without it.
	int32 FirstInvalidChunk = FractureSession->BlastMesh->GetChunkCount();
	TArray<int32> FracturedChunks;
	FractureSession->FractureTool->setRemoveIslands(Settings->bRemoveIslands);
	bool IsCancel = false;

	FTransform3f UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(
		Cast<UFbxSkeletalMeshImportData>(FractureSession->BlastMesh->Mesh->GetAssetImportData()));

	for (int32 ChunkIndex : SelectedChunkIndices)
	{
		if (IsCancel)
		{
			break;
		}
		int32 FractureChunkId = GetFractureChunkId(FractureSession, ChunkIndex);
		if (ChunkIndex && (ChunkIndex >= FirstInvalidChunk || FractureChunkId == INDEX_NONE))
		{
			continue;
		}
		int32 RandomSeed = Settings->bUseFractureSeed ? Settings->FractureSeed : -1;
		bool IsReplace = Settings->bReplaceFracturedChunk;
		switch (Settings->FractureMethod)
		{
		case EBlastFractureMethod::VoronoiUniform:
			if (!FractureVoronoi(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                     Settings->VoronoiUniformFracture->CellCount,
			                     Settings->VoronoiUniformFracture->CellAnisotropy,
			                     Settings->VoronoiUniformFracture->CellRotation,
			                     Settings->VoronoiUniformFracture->ForceReset))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::VoronoiClustered:
			if (!FractureClusteredVoronoi(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                              Settings->VoronoiClusteredFracture->CellCount,
			                              Settings->VoronoiClusteredFracture->ClusterCount,
			                              Settings->VoronoiClusteredFracture->ClusterRadius,
			                              Settings->VoronoiClusteredFracture->CellAnisotropy,
			                              Settings->VoronoiClusteredFracture->CellRotation,
			                              Settings->VoronoiClusteredFracture->ForceReset))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::VoronoiRadial:
			if (!FractureRadial(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                    UE4ToBlastTransform.TransformPosition(Settings->RadialFracture->Origin),
			                    UE4ToBlastTransform.TransformVector(Settings->RadialFracture->Normal),
			                    Settings->RadialFracture->Radius,
			                    Settings->RadialFracture->AngularSteps, Settings->RadialFracture->RadialSteps,
			                    Settings->RadialFracture->AngleOffset, Settings->RadialFracture->Variability,
			                    Settings->RadialFracture->CellAnisotropy, Settings->RadialFracture->CellRotation,
			                    Settings->RadialFracture->ForceReset))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::VoronoiInSphere:
			if (!FractureInSphere(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                      Settings->InSphereFracture->CellCount, Settings->InSphereFracture->Radius,
			                      UE4ToBlastTransform.TransformPosition(Settings->InSphereFracture->Origin),
			                      Settings->InSphereFracture->CellAnisotropy, Settings->InSphereFracture->CellRotation,
			                      Settings->InSphereFracture->ForceReset))
			{
				//IsCancel = true;
			}
			break;
		case EBlastFractureMethod::VoronoiRemoveInSphere:
			if (!RemoveInSphere(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                    Settings->RemoveInSphere->Radius,
			                    UE4ToBlastTransform.TransformPosition(Settings->RemoveInSphere->Origin),
			                    Settings->RemoveInSphere->Probability, Settings->RemoveInSphere->ForceReset))
			{
				//IsCancel = true;
			}
			break;
		case EBlastFractureMethod::UniformSlicing:
			if (!FractureUniformSlicing(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                            Settings->UniformSlicingFracture->SlicesCount,
			                            Settings->UniformSlicingFracture->AngleVariation,
			                            Settings->UniformSlicingFracture->OffsetVariation,
			                            Settings->UniformSlicingFracture->Amplitude,
			                            Settings->UniformSlicingFracture->Frequency,
			                            Settings->UniformSlicingFracture->OctaveNumber,
			                            Settings->UniformSlicingFracture->SamplingInterval))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::Cutout:
			if (!FractureCutout(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                    Settings->CutoutFracture->Pattern, Settings->CutoutFracture->Origin,
			                    Settings->CutoutFracture->Normal,
			                    Settings->CutoutFracture->Size, Settings->CutoutFracture->RotationZ,
			                    Settings->CutoutFracture->Aperture,
			                    Settings->CutoutFracture->bPeriodic, Settings->CutoutFracture->bFillGaps,
			                    Settings->CutoutFracture->Amplitude, Settings->CutoutFracture->Frequency,
			                    Settings->CutoutFracture->OctaveNumber, Settings->CutoutFracture->SamplingInterval))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::Cut:
			if (!FractureCut(FractureSession, FractureChunkId, RandomSeed, IsReplace,
			                 UE4ToBlastTransform.TransformPosition(Settings->CutFracture->Point),
			                 UE4ToBlastTransform.TransformPosition(Settings->CutFracture->Normal),
			                 Settings->CutFracture->Amplitude, Settings->CutFracture->Frequency,
			                 Settings->CutFracture->OctaveNumber, Settings->CutFracture->SamplingInterval))
			{
				IsCancel = true;
			}
			break;

		case EBlastFractureMethod::ChunksFromIslands:
			if (!FractureChunksFromIslands(FractureSession, FractureChunkId))
			{
				IsCancel = true;
			}
			break;
		}
		FracturedChunks.Add(ChunkIndex);
	}
	if (FracturedChunks.Num())
	{
		// not necessary because it's already done in ProcessFracture
		// FractureSession->FractureTool->finalizeFracturing();
		int32 SupportLevel = Settings->bDefaultSupportDepth ? Settings->DefaultSupportDepth : -1;
		Settings->FractureSession->IsMeshModified = true;
		LoadFracturedMesh(Settings->FractureSession, SupportLevel, nullptr, InteriorMaterial);
		PopulateSettingsFromBlastMesh(Settings, FractureSession->BlastMesh);
	}
}

void FBlastFracture::FitUvs(UBlastFractureSettings* Settings, float Size, bool OnlySpecified, TSet<int32>& ChunkIndices)
{
	FScopeLock Lock(&ExclusiveFractureSection);
	if (ChunkIndices.Num() > 0 && OnlySpecified)
	{
		for (int32 ChunkIndex : ChunkIndices)
		{
			int32 ChunkId = GetFractureChunkId(Settings->FractureSession, ChunkIndex);
			Settings->FractureSession->FractureTool->fitUvToRect(Size, ChunkId);
		}
	}
	else
	{
		Settings->FractureSession->FractureTool->fitAllUvToRect(Size);
	}
	int32 SupportLevel = Settings->bDefaultSupportDepth ? Settings->DefaultSupportDepth : -1;
	ReloadGraphicsMesh(Settings->FractureSession, SupportLevel, nullptr, Settings->InteriorMaterial);
}

void FBlastFracture::BuildChunkHierarchy(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices,
                                         uint32 Threshold, uint32 TargetedClusterSize, bool RemoveMergedOriginalChunks)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	if (Settings == nullptr || !Settings->FractureSession.IsValid() || !Settings->FractureSession->IsRootFractured)
	{
		return;
	}
	int32 SupportLevel = Settings->bDefaultSupportDepth ? Settings->DefaultSupportDepth : -1;

	Nv::Blast::AuthoringResult* PreviousResult = Settings->FractureSession->FractureData.Get();
	TArray<NvcVec2i> AdjChunks;
	AdjChunks.SetNumUninitialized(PreviousResult->bondCount);
	for (uint32 i = 0; i < PreviousResult->bondCount; ++i)
	{
		const uint32* Indices = PreviousResult->bondDescs[i].chunkIndices;
		AdjChunks[i] = {
			GetChunkToolIndexFromSessionIndex(Settings->FractureSession, (int32)Indices[0]),
			GetChunkToolIndexFromSessionIndex(Settings->FractureSession, (int32)Indices[1])
		};
		if (AdjChunks[i].x < 0 || AdjChunks[i].y < 0)
		{
			return;
		}
	}
	TArray<uint32> SelectionBuffer;
	SelectionBuffer.Reserve(SelectedChunkIndices.Num());
	for (int32 i : SelectedChunkIndices)
	{
		const int32 ChunkIndex = GetChunkToolIndexFromSessionIndex(Settings->FractureSession, i);
		if (ChunkIndex < 0)
		{
			return;
		}
		SelectionBuffer.Add((uint32)ChunkIndex);
	}
	Settings->FractureSession->FractureTool->uniteChunks(Threshold, TargetedClusterSize, SelectionBuffer.GetData(),
	                                                     (uint32)SelectionBuffer.Num(),
	                                                     AdjChunks.GetData(), (uint32)AdjChunks.Num(),
	                                                     RemoveMergedOriginalChunks);

	SelectedChunkIndices.Empty();

	Settings->FractureSession->IsMeshModified = true;
	LoadFracturedMesh(Settings->FractureSession, SupportLevel, nullptr, Settings->InteriorMaterial);
}

bool CreatePhysicsAsset(TSharedPtr<Nv::Blast::AuthoringResult> FractureData, UBlastMesh* BlastMesh,
                        UFbxSkeletalMeshImportData* SkelMeshImportData = nullptr)
{
	if (SkelMeshImportData == nullptr)
	{
		SkelMeshImportData = Cast<UFbxSkeletalMeshImportData>(BlastMesh->Mesh->GetAssetImportData());
	}
	FBlastScopedProfiler PACP("PhysicsAssetCreation");
	TMap<FName, TArray<FBlastCollisionHull>> hulls;
	hulls.Empty();
	FTransform3f Converter = UBlastMeshFactory::GetTransformBlastToUE4CoordinateSystem(SkelMeshImportData);
	for (uint32 ci = 0; ci < FractureData->chunkCount; ci++)
	{
		TArray<FBlastCollisionHull>& chunkHulls = hulls.Add(FName(*(UBlastMesh::ChunkPrefix + FString::FromInt(ci))));
		chunkHulls.SetNum(FractureData->collisionHullOffset[ci + 1] - FractureData->collisionHullOffset[ci]);

		for (uint32 hi = FractureData->collisionHullOffset[ci]; hi < FractureData->collisionHullOffset[ci + 1]; hi++)
		{
			const Nv::Blast::CollisionHull* fh = FractureData->collisionHull[hi];

			FBlastCollisionHull& outputHull = chunkHulls[hi - FractureData->collisionHullOffset[ci]];
			outputHull.Points.SetNumUninitialized(fh->pointsCount);
			FMemory::Memcpy(outputHull.Points.GetData(), fh->points, sizeof(FVector3f) * fh->pointsCount);
			for (FVector3f& p : outputHull.Points)
			{
				p = Converter.TransformPosition(p);
			}

			outputHull.Indices.SetNumUninitialized(fh->indicesCount);
			FMemory::Memcpy(outputHull.Indices.GetData(), fh->indices, sizeof(uint32) * fh->indicesCount);

			outputHull.PolygonData.SetNumUninitialized(fh->polygonDataCount);
			FMemory::Memcpy(outputHull.PolygonData.GetData(), fh->polygonData,
			                sizeof(Nv::Blast::HullPolygon) * fh->polygonDataCount);
		}
	}
	if (!UBlastMeshFactory::RebuildPhysicsAsset(BlastMesh, hulls))
	{
		return false;
	}
	return true;
}

void FBlastFracture::RebuildCollisionMesh(UBlastFractureSettings* Settings, uint32 MaxNumOfConvex, uint32 Resolution,
                                          float Concavity, const TSet<int32>& ChunkIndices)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	if (Settings == nullptr || !Settings->FractureSession.IsValid() || !Settings->FractureSession->IsRootFractured)
	{
		return;
	}
	FScopedSlowTask SlowTask(1, LOCTEXT("RebuildCollisionMesh",
	                                    "Recalculating collision mesh, this may take a while."));

	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();

	auto FractureData = Settings->FractureSession->FractureData;
	Nv::Blast::ConvexDecompositionParams Param;
	Param.maximumNumberOfHulls = MaxNumOfConvex;
	Param.voxelGridResolution = Resolution;
	Param.concavity = Concavity;

	FCollisionBuilder CollisionBuilder;
	NvBlastExtAuthoringBuildCollisionMeshes(*FractureData, CollisionBuilder, Param, ChunkIndices.Num(),
	                                        ChunkIndices.Num() ? (uint32*)&ChunkIndices.Array()[0] : nullptr);

	if (FractureData->collisionHull != nullptr)
	{
		CreatePhysicsAsset(FractureData, Settings->FractureSession->BlastMesh);
	}
}

void FBlastFracture::RemoveSubhierarchy(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices,
                                        bool bIncludeRoot)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	if (Settings == nullptr || !Settings->FractureSession.IsValid() || SelectedChunkIndices.Num() == 0)
	{
		return;
	}
	auto FractureSession = Settings->FractureSession;
	int32 SupportLevel = Settings->bDefaultSupportDepth ? Settings->DefaultSupportDepth : -1;
	bool IsMeshChanged = false;
	for (int32 ChunkIndex : SelectedChunkIndices)
	{
		int32 FractureChunkId = GetFractureChunkId(FractureSession, ChunkIndex);
		IsMeshChanged |= FractureSession->FractureTool->deleteChunkSubhierarchy(FractureChunkId, bIncludeRoot);
		auto Generator = FractureSession->SitesGeneratorMap.Find(FractureChunkId);
		if (Generator != nullptr && Generator->Generator.IsValid())
		{
			//Generator->Generator->release();
			Generator->Generator.Reset();
		}
	}
	if (IsMeshChanged)
	{
		FractureSession->IsMeshModified = true;
		LoadFracturedMesh(FractureSession, SupportLevel, nullptr, nullptr);
	}
}

void FBlastFracture::ReloadGraphicsMesh(FFractureSessionPtr FractureSession, int32 DefaultSupportDepth,
                                        UStaticMesh* InSourceStaticMesh, UMaterialInterface* InteriorMaterial)
{
	TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;
	auto FS = FractureSession.Pin();
	if (FS->FractureData.Get() == nullptr)
	{
		FCollisionBuilder CollisionBuilder;
		auto BondGenerator = NvBlastExtAuthoringCreateBondGenerator(&CollisionBuilder);
		Nv::Blast::ConvexDecompositionParams param;
		param.maximumNumberOfHulls = 1;
		param.voxelGridResolution = 0;
		{
			FBlastScopedProfiler EAPFP("NvBlastExtAuthoringProcessFracture");
			FS->FractureData = TSharedPtr<Nv::Blast::AuthoringResult>(
				NvBlastExtAuthoringProcessFracture(*FS->FractureTool, *BondGenerator, CollisionBuilder, param,
				                                   DefaultSupportDepth),
				[](Nv::Blast::AuthoringResult* p)
				{
					FCollisionBuilder CollisionBuilder;
					NvBlastExtAuthoringReleaseAuthoringResult(CollisionBuilder, p);
				});
		}
		BondGenerator->release();
		if (!FS->FractureData.IsValid())
		{
			return;
		}
	}
	else
	{
		NvBlastExtAuthoringUpdateGraphicsMesh(*FS->FractureTool.Get(), *FS->FractureData.Get());
	}


	if (FS->ChunkToBoneIndex.IsEmpty())
	{
		FS->ChunkToBoneIndexPrev = FS->ChunkToBoneIndex;
		FS->ChunkToBoneIndex.Empty();
		FS->ChunkToBoneIndex.Add(INDEX_NONE, 0);
		int32 BoneIndex = 1;
		for (uint32 ci = 0; ci < FS->FractureData->chunkCount; ++ci)
		{
			FS->ChunkToBoneIndex.Add(ci, BoneIndex++);
		}
	}
	UBlastMesh* BlastMesh = FS->BlastMesh;
	if (BlastMesh->Mesh)
	{
		BlastMesh->Mesh->ReleaseResources();
		BlastMesh->Mesh->ReleaseResourcesFence.Wait();
	}
	CreateSkeletalMeshFromAuthoring(FS, false, InteriorMaterial);

	BlastMesh->RebuildIndexToBoneNameMap();
	BlastMesh->PostLoad();
};

bool FBlastFracture::LoadFractureData(FFractureSessionPtr FractureSession, int32 DefaultSupportDepth,
                                      UStaticMesh* InSourceStaticMesh)
{
	const auto FS = FractureSession.Pin();
	if (!FS.IsValid() || (InSourceStaticMesh == nullptr) && !FS->BlastMesh)
	{
		return false;
	}

	FCollisionBuilder CollisionBuilder;
	Nv::Blast::BlastBondGenerator* BondGenerator = NvBlastExtAuthoringCreateBondGenerator(&CollisionBuilder);
	Nv::Blast::ConvexDecompositionParams param;
	param.maximumNumberOfHulls = 1; // convex decomposition is slow AF, like 1 minute+, so we skip it
	{
		FBlastScopedProfiler EAPFP("NvBlastExtAuthoringProcessFracture");
		FS->FractureData = TSharedPtr<Nv::Blast::AuthoringResult>(
			NvBlastExtAuthoringProcessFracture(*FS->FractureTool, *BondGenerator, CollisionBuilder, param,
			                                   DefaultSupportDepth),
			[](Nv::Blast::AuthoringResult* p)
			{
				FCollisionBuilder CollisionBuilder;
				NvBlastExtAuthoringReleaseAuthoringResult(CollisionBuilder, p);
			});
	}
	BondGenerator->release();
	if (!FS->FractureData.IsValid())
	{
		return false;
	}
	FS->IsRootFractured = true;

	FS->ChunkToBoneIndexPrev = FS->ChunkToBoneIndex;
	FS->ChunkToBoneIndex.Empty();
	FS->ChunkToBoneIndex.Add(INDEX_NONE, 0);
	int32 BoneIndex = 1;
	for (uint32 ci = 0; ci < FS->FractureData->chunkCount; ++ci)
	{
		FS->ChunkToBoneIndex.Add(ci, BoneIndex++);
	}

	return true;
}

void FBlastFracture::LoadFractureToolData(TSharedPtr<FFractureSession> FS)
{
	FBlastFractureToolData* FTD = &FS->BlastMesh->FractureHistory.GetCurrentToolData();
	const int32 ChunkCount = FS->FractureTool->getChunkCount();
	FTD->ChunkMeshes.Reset(ChunkCount);

	for (int32 AssetIndex = 0; AssetIndex < ChunkCount; AssetIndex++)
	{
		FBlastChunkMesh& Chunk = FTD->ChunkMeshes.AddDefaulted_GetRef();
		const Nv::Blast::ChunkInfo& Info = FS->FractureTool->getChunkInfo(
			GetChunkToolIndexFromSessionIndex(FS, AssetIndex));

		const FTransform3f ChunkToWorld = FTransform3f(FQuat4f::Identity,
		                                               FVector3f(FromNvVector(Info.getTmToWorld().t)),
		                                               FVector3f(Info.getTmToWorld().s));

		Chunk.Vertices.AddUninitialized(Info.getMesh()->getVerticesCount());
		FMemory::Memcpy(Chunk.Vertices.GetData(), Info.getMesh()->getVertices(),
		                Info.getMesh()->getVerticesCount() * sizeof(Nv::Blast::Vertex));
		for (FBlastVertex& Vert : Chunk.Vertices)
		{
			Vert.P = ChunkToWorld.TransformPosition(Vert.P);
		}
		Chunk.Edges.AddUninitialized(Info.getMesh()->getEdgesCount());
		FMemory::Memcpy(Chunk.Edges.GetData(),
		                Info.getMesh()->getEdges(), Info.getMesh()->getEdgesCount() * sizeof(Nv::Blast::Edge));
		Chunk.Faces.AddUninitialized(Info.getMesh()->getFacetCount());
		FMemory::Memcpy(Chunk.Faces.GetData(),
		                Info.getMesh()->getFacetsBuffer(), Info.getMesh()->getFacetCount() * sizeof(Nv::Blast::Facet));
		if (Info.flags & (uint32)Nv::Blast::ChunkInfo::APPROXIMATE_BONDING)
		{
			Chunk.ChunkFlag |= EBlastMeshChunkFlags::ApproximateBonding;
		}
	}
}

void FBlastFracture::LoadFracturedMesh(FFractureSessionPtr FractureSession, int32 DefaultSupportDepth,
                                       UStaticMesh* InSourceStaticMesh, UMaterialInterface* InteriorMaterial)
{
	FBlastScopedProfiler LFMP("LoadFracturedMesh");
	TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

	auto FS = FractureSession.Pin();
	UBlastMesh* BlastMesh = FS->BlastMesh;

	if (!LoadFractureData(FractureSession, DefaultSupportDepth, InSourceStaticMesh))
	{
		return;
	}

	if (BlastMesh->Mesh)
	{
		BlastMesh->Mesh->ReleaseResources();
		BlastMesh->Mesh->ReleaseResourcesFence.Wait();
	}

	TArray<FSkeletalMaterial> ExistingMaterials;
	UFbxSkeletalMeshImportData* SkelMeshImportData = nullptr;

	if (InSourceStaticMesh != nullptr)
	{
		CreateSkeletalMeshFromAuthoring(FS, InSourceStaticMesh);
	}
	else
	{
		FBlastScopedProfiler USMFAP("UpdateSkeletalMeshFromAuthoring");

		SkelMeshImportData = Cast<UFbxSkeletalMeshImportData>(BlastMesh->Mesh->GetAssetImportData());

		UpdateSkeletalMeshFromAuthoring(FS, InteriorMaterial);
	}

	if (FS->IsMeshModified)
	{
		BlastMesh->FractureHistory.Push();
		LoadFractureToolData(FractureSession.Pin());
		FS->IsMeshModified = false;
	}

	//* generate NvBlastAsset and setup it for Mesh
	UBlastMeshFactory::TransformBlastAssetToUE4CoordinateSystem(FS->FractureData->asset, SkelMeshImportData);
	BlastMesh->CopyFromLoadedAsset(FS->FractureData->asset);

	if (!CreatePhysicsAsset(FS->FractureData, BlastMesh, SkelMeshImportData))
	{
		return;
	}

	// Have to manually call this, since it doesn't get called on create
	BlastMesh->RebuildIndexToBoneNameMap();
	BlastMesh->RebuildCookedBodySetupsIfRequired(true);

	BlastMesh->PostLoad();
}


void FBlastFracture::PopulateSettingsFromBlastMesh(UBlastFractureSettings* Settings, UBlastMesh* BlastMesh)
{
	//If we have an interior material from a previous fracture, default to that
	if (BlastMesh->Mesh && Settings->InteriorMaterialSlotName == NAME_None)
	{
		for (FSkeletalMaterial& MatSlot : BlastMesh->Mesh->GetMaterials())
		{
			const bool bCompareTrailingNumber = false;
			if (MatSlot.ImportedMaterialSlotName.IsEqual(FBlastFracture::InteriorMaterialID, ENameCase::IgnoreCase,
			                                             bCompareTrailingNumber))
			{
				Settings->InteriorMaterialSlotName = MatSlot.ImportedMaterialSlotName;
				break;
			}
		}
	}
}

TSharedPtr<Nv::Blast::VoronoiSitesGenerator> FBlastFracture::GetVoronoiSitesGenerator(
	TSharedPtr<FFractureSession> FractureSession, int32 FractureChunkId, bool ForceReset)
{
	auto SitesGenerator = FractureSession->SitesGeneratorMap.Find(FractureChunkId);
	if (SitesGenerator == nullptr || !SitesGenerator->Generator.IsValid() || ForceReset)
	{
		const int32 chunkInfoIdx = FractureSession->FractureTool->getChunkInfoIndex(FractureChunkId);

		FractureSession->SitesGeneratorMap.Remove(FractureChunkId);
		SitesGenerator = &(FractureSession->SitesGeneratorMap.Add(FractureChunkId,
		                                                          FFractureSession::ChunkSitesGenerator()));
		SitesGenerator->Mesh = TSharedPtr<Nv::Blast::Mesh>(FractureSession->FractureTool->createChunkMesh(chunkInfoIdx),
		                                                   [](Nv::Blast::Mesh* p)
		                                                   {
			                                                   p->release();
		                                                   });
		if (!SitesGenerator->Mesh)
		{
			return {};
		}
		SitesGenerator->Generator = TSharedPtr<Nv::Blast::VoronoiSitesGenerator>(
			NvBlastExtAuthoringCreateVoronoiSitesGenerator(SitesGenerator->Mesh.Get(), RandomGenerator.Get()),
			[](Nv::Blast::VoronoiSitesGenerator* p)
			{
				p->release();
			});
	}
	return SitesGenerator->Generator;
};

bool FBlastFracture::FractureVoronoi(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                     int32 RandomSeed, bool IsReplace,
                                     uint32 CellCount, FVector3f CellAnisotropy, FQuat4f CellRotation, bool ForceReset)
{
	FBlastScopedProfiler FVP("FractureVoronoi");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		{
			FBlastScopedProfiler UGSP("FractureVoronoi::uniformlyGenerateSitesInMesh");
			VoronoiSitesGenerator->uniformlyGenerateSitesInMesh(CellCount);
		}
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		{
			FBlastScopedProfiler VFP("FractureVoronoi::voronoiFracturing");
			if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites,
			                                                     ToNvVector(CellAnisotropy), ToNvQuat(CellRotation),
			                                                     IsReplace) == 0)
			{
				return true;
			}
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi"));
	return false;
}

bool FBlastFracture::FractureClusteredVoronoi(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                              int32 RandomSeed, bool IsReplace,
                                              uint32 CellCount, uint32 ClusterCount, float ClusterRadius,
                                              FVector3f CellAnisotropy, FQuat4f CellRotation, bool ForceReset)
{
	FBlastScopedProfiler FCVP("FractureClusteredVoronoi");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		VoronoiSitesGenerator->clusteredSitesGeneration(ClusterCount, CellCount, ClusterRadius);
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites,
		                                                     ToNvVector(CellAnisotropy), ToNvQuat(CellRotation),
		                                                     IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with clustered Voronoi"));
	return false;
}

bool FBlastFracture::FractureRadial(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                    int32 RandomSeed, bool IsReplace,
                                    FVector3f Origin, FVector3f Normal, float Radius, uint32 AngularSteps,
                                    uint32 RadialSteps, float AngleOffset, float Variability,
                                    FVector3f CellAnisotropy, FQuat4f CellRotation, bool ForceReset)
{
	FBlastScopedProfiler FRP("FractureRadial");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		Normal.Normalize();
		VoronoiSitesGenerator->radialPattern(ToNvVector(Origin), ToNvVector(Normal), Radius, AngularSteps, RadialSteps,
		                                     AngleOffset, Variability);
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites,
		                                                     ToNvVector(CellAnisotropy), ToNvQuat(CellRotation),
		                                                     IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi"));
	return false;
}

bool FBlastFracture::FractureInSphere(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                      int32 RandomSeed, bool IsReplace,
                                      uint32 CellCount, float Radius, FVector3f Origin, FVector3f CellAnisotropy,
                                      FQuat4f CellRotation, bool ForceReset)
{
	FBlastScopedProfiler FISP("FractureInSphere");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		VoronoiSitesGenerator->generateInSphere(CellCount, Radius, ToNvVector(Origin));
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites,
		                                                     ToNvVector(CellAnisotropy), ToNvQuat(CellRotation),
		                                                     IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi in sphere"));
	return false;
}

bool FBlastFracture::RemoveInSphere(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                    int32 RandomSeed, bool IsReplace,
                                    float Radius, FVector3f Origin, float Probability, bool ForceReset)
{
	FBlastScopedProfiler RISP("RemoveInSphere");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		VoronoiSitesGenerator->deleteInSphere(Radius, ToNvVector(Origin), Probability);
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites, IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi in sphere"));
	return false;
}

bool FBlastFracture::FractureUniformSlicing(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                            int32 RandomSeed, bool IsReplace,
                                            FIntVector SlicesCount, float AngleVariation, float OffsetVariation,
                                            float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber,
                                            FVector3f SamplingInterval)
{
	FBlastScopedProfiler FUSP("FractureUniformSlicing");
	RandomGenerator->seed(RandomSeed);
	Nv::Blast::SlicingConfiguration slConfig;
	slConfig.x_slices = SlicesCount.X;
	slConfig.y_slices = SlicesCount.Y;
	slConfig.z_slices = SlicesCount.Z;
	slConfig.angle_variations = AngleVariation;
	slConfig.offset_variations = OffsetVariation;
	slConfig.noise.amplitude = NoiseAmplitude;
	slConfig.noise.frequency = NoiseFrequency;
	slConfig.noise.octaveNumber = NoiseOctaveNumber;
	slConfig.noise.samplingInterval = ToNvVector(SamplingInterval);
	if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
	{
		IsReplace = false;
	}
	if (FractureSession->FractureTool->slicing(FractureChunkId, slConfig, IsReplace, RandomGenerator.Get()) != 0)
	{
		UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with slicing"));
		return false;
	}
	return true;
}

bool FBlastFracture::FractureCutout(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId,
                                    int32 RandomSeed, bool IsReplace,
                                    UTexture2D* Pattern, FVector3f Origin, FVector3f Normal, FVector2f Size,
                                    float RotationZ, float Aperture, bool bPeriodic, bool bFillGaps,
                                    float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber,
                                    FVector3f SamplingInterval)
{
	FBlastScopedProfiler FCP("FractureCutout");
	RandomGenerator->seed(RandomSeed);

	Normal.Normalize();
	FTransform3f UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(
		Cast<UFbxSkeletalMeshImportData>(FractureSession->BlastMesh->Mesh->GetAssetImportData()));
	//FTransform3f ScaleTr; ScaleTr.SetScale3D(FVector3f(Scale.X, Scale.Y, 1.f) * 0.5f);
	FTransform3f YawTr(FQuat4f(FVector3f(0.f, 0.f, 1.f), FMath::DegreesToRadians(RotationZ)));
	FTransform3f Tr(FQuat4f::FindBetweenNormals(FVector3f(0.f, 0.f, 1.f), Normal), Origin);
	Tr = YawTr * Tr * UE4ToBlastTransform;

	Nv::Blast::CutoutConfiguration CutoutConfig;
	CutoutConfig.transform.q = ToNvQuat(Tr.GetRotation());
	CutoutConfig.transform.p = ToNvVector(Tr.GetLocation());
	CutoutConfig.scale = {Size.X, Size.Y};
	CutoutConfig.isRelativeTransform = false;
	//CutoutConfig.useSmoothing = true;
	CutoutConfig.aperture = Aperture;
	CutoutConfig.noise.amplitude = NoiseAmplitude;
	CutoutConfig.noise.frequency = NoiseFrequency;
	CutoutConfig.noise.octaveNumber = NoiseOctaveNumber;
	CutoutConfig.noise.samplingInterval = ToNvVector(SamplingInterval);
	if (Pattern != nullptr)
	{
		CutoutConfig.cutoutSet = NvBlastExtAuthoringCreateCutoutSet();

		TArray64<uint8_t> Buf, Mip;
		Pattern->Source.GetMipData(Mip, 0);
		int32 sz = Pattern->Source.GetSizeX() * Pattern->Source.GetSizeY();
		Buf.Reserve(sz * 3);
		for (int32 i = 0; i < sz; i++)
		{
			Buf.Push(Mip[i * 4]);
			Buf.Push(Mip[i * 4 + 1]);
			Buf.Push(Mip[i * 4 + 2]);
		}
		float SegmentationErrorThreshold = 1e-3; //Move this to advanced settings?
		float SnapThreshold = 1.f;
		NvBlastExtAuthoringBuildCutoutSet(*CutoutConfig.cutoutSet, Buf.GetData(), Pattern->Source.GetSizeX(),
		                                  Pattern->Source.GetSizeY(), SegmentationErrorThreshold, SnapThreshold,
		                                  bPeriodic, bFillGaps);
	}
	else
	{
		UE_LOG(LogBlastMeshEditor, Error, TEXT("Cutout Fracture: Texture with cutout pattern not found."));
		return false;
	}
	if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
	{
		IsReplace = false;
	}
	if (FractureSession->FractureTool->cutout(FractureChunkId, CutoutConfig, IsReplace, RandomGenerator.Get()) != 0)
	{
		UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to perform cutout fracture"));
		return false;
	}
	return true;
}

bool FBlastFracture::FractureChunksFromIslands(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId)
{
	if (!FractureSession->BlastMesh)
	{
		return false;
	}

	return FractureSession->FractureTool->islandDetectionAndRemoving(FractureChunkId, true) > 0;
}

bool FBlastFracture::FractureCut(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed,
                                 bool IsReplace,
                                 FVector3f Origin, FVector3f Normal,
                                 float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber,
                                 FVector3f SamplingInterval)
{
	Normal.Normalize();
	RandomGenerator->seed(RandomSeed);
	Nv::Blast::NoiseConfiguration NoiseConfig;
	NoiseConfig.amplitude = NoiseAmplitude;
	NoiseConfig.frequency = NoiseFrequency;
	NoiseConfig.octaveNumber = NoiseOctaveNumber;
	NoiseConfig.samplingInterval = ToNvVector(SamplingInterval);
	if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
	{
		IsReplace = false;
	}
	if (FractureSession->FractureTool->cut(FractureChunkId, ToNvVector(Normal), ToNvVector(Origin), NoiseConfig,
	                                       IsReplace, RandomGenerator.Get()) != 0)
	{
		UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to perform cut fracture"));
		return false;
	}
	return true;
}

TWeakPtr<FBlastFracture> FBlastFracture::Instance = nullptr;

TSharedPtr<FBlastFracture> FBlastFracture::GetInstance()
{
	TSharedPtr<FBlastFracture> SharedPtr;
	if (!Instance.IsValid())
	{
		SharedPtr = MakeShared<FBlastFracture>();
		Instance = SharedPtr;
	}
	return Instance.Pin();
}

#undef LOCTEXT_NAMESPACE
