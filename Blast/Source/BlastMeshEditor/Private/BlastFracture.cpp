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
#include "NvBounds3.h"

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

#if BLAST_USE_PHYSX
#include "PhysicsPublicCore.h"
#include "IPhysXCookingModule.h"
#include "IPhysXCooking.h"
#include "PhysXPublic.h"
#else
#include "Chaos/Core.h"
#include "Chaos/Convex.h"
#endif

#include <vector>

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
		UE_LOG(LogBlastMeshEditor, Verbose, TEXT("Blast Profiler: %s - Execution time, s: %f"), *Name, Duration / (double)CLOCKS_PER_SEC);
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
	FCollisionHull() {};
	FCollisionHull(const CollisionHull& hullToCopy)
	{
		pointsCount = hullToCopy.pointsCount;
		indicesCount = hullToCopy.indicesCount;
		polygonDataCount = hullToCopy.polygonDataCount;

		points = SAFE_ARRAY_NEW(NvcVec3, pointsCount);
		indices = SAFE_ARRAY_NEW(uint32, indicesCount);
		polygonData = SAFE_ARRAY_NEW(Nv::Blast::HullPolygon, polygonDataCount);
		memcpy(points, hullToCopy.points, sizeof(points[0]) * pointsCount);
		memcpy(indices, hullToCopy.indices, sizeof(indices[0]) * indicesCount);
		memcpy(polygonData, hullToCopy.polygonData, sizeof(polygonData[0]) * polygonDataCount);
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
	virtual ~FCollisionBuilder() {}

	Nv::Blast::CollisionHull* buildCollisionGeometry(uint32 verticesCount, const NvcVec3* vData) override
	{
#if BLAST_USE_PHYSX
		std::vector<NvcVec3> vertexData(verticesCount);
		FMemory::Memcpy(vertexData.data(), vData, sizeof(NvcVec3) * verticesCount);
		NvBounds3 bounds;
		bounds.setEmpty();
		for (uint32 i = 0; i < vertexData.size(); ++i)
		{
			bounds.include(vertexData[i]);
		}
		NvcVec3 bbCenter = bounds.getCenter();

		// Scale chunk to unit cube size, to avoid numerical errors
		PxConvexMeshDesc convexMeshDescr;
		float scale = FMath::Max(FMath::Abs(bounds.getExtents(0)), FMath::Max(FMath::Abs(bounds.getExtents(1)), FMath::Abs(bounds.getExtents(2))));
		for (uint32 i = 0; i < vertexData.size(); ++i)
		{
			vertexData[i] = vertexData[i] - bbCenter;
			vertexData[i] *= (1.0f / scale);
		}
		bounds.setEmpty();
		for (uint32 i = 0; i < vertexData.size(); ++i)
		{
			bounds.include(vertexData[i]);
		}
		convexMeshDescr.points.data = vertexData.data();
		convexMeshDescr.points.stride = sizeof(NvcVec3);
		convexMeshDescr.points.count = (uint32)vertexData.size();
		convexMeshDescr.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		PxConvexMesh* resultConvexMesh = mCooking->createConvexMesh(convexMeshDescr, *mInsertionCallback);
		if (!resultConvexMesh)
		{
			vertexData.clear();
			vertexData.push_back(bounds.minimum);
			vertexData.push_back(NvVec3(bounds.minimum.x, bounds.maximum.y, bounds.minimum.z));
			vertexData.push_back(NvVec3(bounds.maximum.x, bounds.maximum.y, bounds.minimum.z));
			vertexData.push_back(NvVec3(bounds.maximum.x, bounds.minimum.y, bounds.minimum.z));
			vertexData.push_back(NvVec3(bounds.minimum.x, bounds.minimum.y, bounds.maximum.z));
			vertexData.push_back(NvVec3(bounds.minimum.x, bounds.maximum.y, bounds.maximum.z));
			vertexData.push_back(NvVec3(bounds.maximum.x, bounds.maximum.y, bounds.maximum.z));
			vertexData.push_back(NvVec3(bounds.maximum.x, bounds.minimum.y, bounds.maximum.z));
			convexMeshDescr.points.data = vertexData.data();
			convexMeshDescr.points.count = (uint32)vertexData.size();
			resultConvexMesh = mCooking->createConvexMesh(convexMeshDescr, *mInsertionCallback);
		}

		Nv::Blast::CollisionHull* output = new FCollisionHull();
		output->polygonDataCount = resultConvexMesh->getNbPolygons();
		if (output->polygonDataCount)
			output->polygonData = SAFE_ARRAY_NEW(Nv::Blast::HullPolygon, output->polygonDataCount);
		output->pointsCount = resultConvexMesh->getNbVertices();
		output->points = SAFE_ARRAY_NEW(NvcVec3, output->pointsCount);
		int32 indicesCount = 0;
		PxHullPolygon hPoly;
		for (uint32 i = 0; i < resultConvexMesh->getNbPolygons(); ++i)
		{
			Nv::Blast::HullPolygon& pd = output->polygonData[i];
			resultConvexMesh->getPolygonData(i, hPoly);
			pd.indexBase = hPoly.mIndexBase;
			pd.vertexCount = hPoly.mNbVerts;
			pd.plane[0] = hPoly.mPlane[0];
			pd.plane[1] = hPoly.mPlane[1];
			pd.plane[2] = hPoly.mPlane[2];
			pd.plane[3] = hPoly.mPlane[3];

			pd.plane[0] /= scale;
			pd.plane[1] /= scale;
			pd.plane[2] /= scale;
			pd.plane[3] -= (pd.plane[0] * bbCenter.x + pd.plane[1] * bbCenter.y + pd.plane[2] * bbCenter.z);
			float length = FMath::Sqrt(pd.plane[0] * pd.plane[0] + pd.plane[1] * pd.plane[1] + pd.plane[2] * pd.plane[2]);
			pd.plane[0] /= length;
			pd.plane[1] /= length;
			pd.plane[2] /= length;
			pd.plane[3] /= length;
			indicesCount = FMath::Max(indicesCount, pd.indexBase + pd.vertexCount);
		}
		output->indicesCount = indicesCount;
		output->indices = SAFE_ARRAY_NEW(uint32, indicesCount);
		for (uint32 i = 0; i < resultConvexMesh->getNbVertices(); ++i)
		{
			NvcVec3 p = resultConvexMesh->getVertices()[i] * scale + bbCenter;
			output->points[i] = { p.x, p.y, p.z };
		}
		for (int32 i = 0; i < indicesCount; ++i)
		{
			output->indices[i] = resultConvexMesh->getIndexBuffer()[i];
		}
		resultConvexMesh->release();
		return output;
#else
		// Create the corner vertices for the convex
		TArray<Chaos::FConvex::FVec3Type> ConvexVertices;
		ConvexVertices.AddUninitialized(verticesCount);
		FMemory::Memcpy(ConvexVertices.GetData(), vData, sizeof(Chaos::FConvex::FVec3Type) * verticesCount);

		// Margin is always zero on convex shapes - they are intended to be instanced
		const Chaos::FConvex GeneratedHull(ConvexVertices, 0.0f);

		Nv::Blast::CollisionHull* output = new FCollisionHull();

		output->pointsCount = GeneratedHull.NumVertices();
		output->points = SAFE_ARRAY_NEW(NvcVec3, output->pointsCount);
		FMemory::Memcpy(output->points, GeneratedHull.GetVertices().GetData(), sizeof(Chaos::FConvex::FVec3Type) * output->pointsCount);

		TArray<int32> Indices;
		Chaos::FConvexBuilder::BuildIndices(GeneratedHull.GetVertices(), Indices);
		output->indicesCount = Indices.Num();
		output->indices = SAFE_ARRAY_NEW(uint32, output->indicesCount);
		FMemory::Memcpy(output->indices, Indices.GetData(), sizeof(uint32) * output->indicesCount);

		output->polygonDataCount = GeneratedHull.NumPlanes();
		if (output->polygonDataCount)
		{
			output->polygonData = SAFE_ARRAY_NEW(Nv::Blast::HullPolygon, output->polygonDataCount);
		}

		int32 NumIndices = 0;
		for (uint32 i = 0; i < output->polygonDataCount; ++i)
		{
			Nv::Blast::HullPolygon& HullPoly = output->polygonData[i];

			const Chaos::FConvex::FPlaneType& ChaosPlane = GeneratedHull.GetPlaneRaw(i);
			ToNvPlane4(FPlane(FVector(ChaosPlane.X()), FVector(ChaosPlane.Normal())), HullPoly.plane);
			HullPoly.vertexCount = GeneratedHull.NumPlaneVertices(i);
			HullPoly.indexBase = NumIndices;
			NumIndices += HullPoly.vertexCount;
		}
		return output;
#endif
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

void ReorderFractureToolData(FBlastFractureToolData& Dst, const FBlastFractureToolData& Src, const int32* Map, int32 ChunkCount)
{
	Dst.VerticesOffset[0] = 0;
	Dst.EdgesOffset[0] = 0;
	Dst.FacesOffset[0] = 0;
	for (int32 ChunkIndex = 0; ChunkIndex < ChunkCount; ++ChunkIndex)
	{
		// Get mapped old chunk chunk index for the new ChunkIndex (in asset order)
		const int32 OldChunkIndex = Map[ChunkIndex];

		// Find the old buffer ranges
		const uint32 vb = Src.VerticesOffset[OldChunkIndex], ve = Src.VerticesOffset[OldChunkIndex + 1];
		const uint32 eb = Src.EdgesOffset[OldChunkIndex], ee = Src.EdgesOffset[OldChunkIndex + 1];
		const uint32 fb = Src.FacesOffset[OldChunkIndex], fe = Src.FacesOffset[OldChunkIndex + 1];

		// Create new offsets
		Dst.VerticesOffset[ChunkIndex + 1] = Dst.VerticesOffset[ChunkIndex] + ve - vb;
		Dst.EdgesOffset[ChunkIndex + 1] = Dst.EdgesOffset[ChunkIndex] + ee - eb;
		Dst.FacesOffset[ChunkIndex + 1] = Dst.FacesOffset[ChunkIndex] + fe - fb;

		// Copy data for this chunk
		FMemory::Memcpy(reinterpret_cast<Nv::Blast::Vertex*>(Dst.Vertices.GetData()) + Dst.VerticesOffset[ChunkIndex], reinterpret_cast<const Nv::Blast::Vertex*>(Src.Vertices.GetData()) + vb, (ve - vb) * sizeof(Nv::Blast::Vertex));
		FMemory::Memcpy(reinterpret_cast<Nv::Blast::Edge*>(Dst.Edges.GetData()) + Dst.EdgesOffset[ChunkIndex], reinterpret_cast<const Nv::Blast::Edge*>(Src.Edges.GetData()) + eb, (ee - eb) * sizeof(Nv::Blast::Edge));
		FMemory::Memcpy(reinterpret_cast<Nv::Blast::Facet*>(Dst.Faces.GetData()) + Dst.FacesOffset[ChunkIndex], reinterpret_cast<const Nv::Blast::Facet*>(Src.Faces.GetData()) + fb, (fe - fb) * sizeof(Nv::Blast::Facet));
	}
}

TSharedPtr<FFractureSession> FBlastFracture::StartFractureSession(UBlastMesh* InBlastMesh, UStaticMesh* InSourceStaticMesh, UBlastFractureSettings* Settings, bool ForceLoadFracturedMesh)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	TSharedPtr<FFractureSession> FractureSession = MakeShareable(new FFractureSession());
	FractureSession->FractureTool = TSharedPtr<Nv::Blast::FractureTool>(NvBlastExtAuthoringCreateFractureTool(), [](Nv::Blast::FractureTool* p)
		{
			p->release();
		});
	FractureSession->BlastMesh = InBlastMesh;
	if (!FractureSession->FractureTool.IsValid() || InBlastMesh == nullptr)
	{
		UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to start fracture session"));
		return nullptr;
	}

	FTransform UE4ToBlastTransform;

	if (InSourceStaticMesh != nullptr)
	{
		UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(nullptr);

		// Fill required material map
		TMap<FName, int32> MaterialMap;
		for (int32 MaterialIndex = 0; MaterialIndex < InSourceStaticMesh->GetStaticMaterials().Num(); ++MaterialIndex)
		{
			MaterialMap.Add(InSourceStaticMesh->GetStaticMaterials()[MaterialIndex].ImportedMaterialSlotName, MaterialIndex);
		}

		FRawMesh InSourceRawMesh;
		FStaticMeshOperations::ConvertToRawMesh(*InSourceStaticMesh->GetMeshDescription(0), InSourceRawMesh, MaterialMap);

		BuildSmoothingGroups(InSourceRawMesh); //Retrieve mesh just assign default smoothing group 1 for each face. So we need to generate it.

		Nv::Blast::Mesh* Mesh = CreateAuthoringMeshFromRawMesh(InSourceRawMesh, UE4ToBlastTransform);
		FractureSession->FractureTool->setSourceMeshes(&Mesh, 1, nullptr);

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
		uint32 ChunkCount = FTD.VerticesOffset.Num() - 1;
		bool ChunkFlagsMissing = false;

		if (InBlastMesh->ChunkIndexMap.Num() > 0)
		{
			ReorderFractureToolData(InBlastMesh->FractureHistory.GetCurrentToolData(), InBlastMesh->FractureToolData, InBlastMesh->ChunkIndexMap.GetData(), ChunkCount);
			InBlastMesh->ChunkIndexMap.Empty();
			//			ForceLoadFracturedMesh = true;
		}

		if (ForceLoadFracturedMesh || (ChunkCount == InBlastMesh->GetChunkCount() && FTD.VerticesOffset.Num() != 0 && FTD.EdgesOffset.Num() == ChunkCount + 1 && FTD.FacesOffset.Num() == ChunkCount + 1))
		{
			ChunkFlagsMissing = FTD.VerticesOffset.Num() > 0 && FTD.ChunkFlags.Num() == 0;
			if (ChunkFlagsMissing)
			{
				FTD.ChunkFlags.SetNum(FTD.VerticesOffset.Num() - 1);
				for (int i = 0; i < FTD.ChunkFlags.Num(); ++i)
				{
					FTD.ChunkFlags[i] = EBlastMeshChunkFlags::ApproximateBonding;
				}
			}

			for (uint32 ChunkIndex = 0; ChunkIndex < ChunkCount; ChunkIndex++)
			{
				uint32 vb = FTD.VerticesOffset[ChunkIndex], ve = FTD.VerticesOffset[ChunkIndex + 1];
				uint32 eb = FTD.EdgesOffset[ChunkIndex], ee = FTD.EdgesOffset[ChunkIndex + 1];
				uint32 fb = FTD.FacesOffset[ChunkIndex], fe = FTD.FacesOffset[ChunkIndex + 1];
				Nv::Blast::Mesh* ChunkMesh = NvBlastExtAuthoringCreateMeshFromFacets((Nv::Blast::Vertex*)FTD.Vertices.GetData() + vb,
					(Nv::Blast::Edge*)FTD.Edges.GetData() + eb, (Nv::Blast::Facet*)FTD.Faces.GetData() + fb, ve - vb, ee - eb, fe - fb);
				if (ChunkIndex == 0)
				{
					FractureSession->FractureTool->setSourceMeshes(&ChunkMesh, 1, nullptr);
				}
				else
				{
					FractureSession->FractureTool->setChunkMesh(ChunkMesh, InBlastMesh->GetChunkInfo(ChunkIndex).parentChunkIndex);
				}
				FractureSession->FractureTool->setApproximateBonding(ChunkIndex, EnumHasAnyFlags(FTD.ChunkFlags[ChunkIndex], EBlastMeshChunkFlags::ApproximateBonding));
			}
		}
		else //If there is no saved fracture tool state, we can load chunks from SkeletalMesh. Note: smoothing groups will be lost.
		{
			UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(Cast<UFbxSkeletalMeshImportData>(InBlastMesh->Mesh->GetAssetImportData()));
			TArray<FRawMesh> RawMeshes;
			RawMeshes.SetNum(InBlastMesh->GetChunkCount());
			{
				FBlastScopedProfiler SCMP("GetRenderMesh");
				InBlastMesh->GetRenderMesh(0, RawMeshes);
			}
			for (int32 ChunkId = 0; ChunkId < RawMeshes.Num(); ChunkId++)
			{
				Nv::Blast::Mesh* ChunkMesh = CreateAuthoringMeshFromRawMesh(RawMeshes[ChunkId], UE4ToBlastTransform);
				if (ChunkId == 0)
				{
					FractureSession->FractureTool->setSourceMeshes(&ChunkMesh, 1, nullptr);
				}
				else
				{
					FractureSession->FractureTool->setChunkMesh(ChunkMesh, InBlastMesh->GetChunkInfo(ChunkId).parentChunkIndex);
				}
			}
		}
		if (ForceLoadFracturedMesh)
		{
			LoadFracturedMesh(FractureSession, SupportLevel, nullptr);
		}
		else
			if (!FractureSession->IsRootFractured)
			{
				LoadFractureData(FractureSession, SupportLevel, nullptr);
				if (FTD.VerticesOffset.Num() == 0)	// This is a way of dealing with old assets that have no fracture tool data serialized
				{
					InBlastMesh->FractureHistory.Collapse();	// Reduce history to one entry
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

void FBlastFracture::GetVoronoiSites(TSharedPtr<FFractureSession> FractureSession, int32 ChunkId, TArray<FVector>& Sites)
{
	if (!FractureSession.IsValid() || !FractureSession->IsRootFractured)
	{
		return;
	}
	auto Generator = FractureSession->SitesGeneratorMap.Find(GetFractureChunkId(FractureSession, ChunkId));
	if (Generator == nullptr || !Generator->Generator.IsValid())
	{
		return;
	}
	const NvcVec3* sites = nullptr;
	int32 sitesCount = Generator->Generator->getVoronoiSites(sites);
	Sites.Reset(sitesCount);
	auto SkelMeshImportData = Cast<UFbxSkeletalMeshImportData>(FractureSession->BlastMesh->Mesh->GetAssetImportData());
	auto Converter = UBlastMeshFactory::GetTransformBlastToUE4CoordinateSystem(SkelMeshImportData);
	for (int32 i = 0; i < sitesCount; i++)
	{
		Sites.Add(Converter.TransformPosition(FVector(sites[i].x, sites[i].y, sites[i].z)));
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

void FBlastFracture::Fracture(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices, int32 ClickedChunkIndex)
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
			if ((Mat.ImportedMaterialSlotName.IsNone() ? Mat.MaterialSlotName : Mat.ImportedMaterialSlotName) == Settings->InteriorMaterialSlotName)
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

	auto UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(Cast<UFbxSkeletalMeshImportData>(FractureSession->BlastMesh->Mesh->GetAssetImportData()));

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
				Settings->VoronoiUniformFracture->CellAnisotropy, Settings->VoronoiUniformFracture->CellRotation,
				Settings->VoronoiUniformFracture->ForceReset))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::VoronoiClustered:
			if (!FractureClusteredVoronoi(FractureSession, FractureChunkId, RandomSeed, IsReplace,
				Settings->VoronoiClusteredFracture->CellCount, Settings->VoronoiClusteredFracture->ClusterCount,
				Settings->VoronoiClusteredFracture->ClusterRadius,
				Settings->VoronoiClusteredFracture->CellAnisotropy, Settings->VoronoiClusteredFracture->CellRotation,
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
				Settings->UniformSlicingFracture->SlicesCount, Settings->UniformSlicingFracture->AngleVariation,
				Settings->UniformSlicingFracture->OffsetVariation,
				Settings->UniformSlicingFracture->Amplitude, Settings->UniformSlicingFracture->Frequency,
				Settings->UniformSlicingFracture->OctaveNumber, Settings->UniformSlicingFracture->SamplingInterval))
			{
				IsCancel = true;
			}
			break;
		case EBlastFractureMethod::Cutout:
			if (!FractureCutout(FractureSession, FractureChunkId, RandomSeed, IsReplace,
				Settings->CutoutFracture->Pattern, Settings->CutoutFracture->Origin, Settings->CutoutFracture->Normal,
				Settings->CutoutFracture->Size, Settings->CutoutFracture->RotationZ, Settings->CutoutFracture->Aperture,
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
		FractureSession->FractureTool->finalizeFracturing();
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

void FBlastFracture::BuildChunkHierarchy(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices, uint32 Threshold, uint32 TargetedClusterSize, bool RemoveMergedOriginalChunks)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	if (Settings == nullptr || !Settings->FractureSession.IsValid() || !Settings->FractureSession->IsRootFractured)
	{
		return;
	}
	int32 SupportLevel = Settings->bDefaultSupportDepth ? Settings->DefaultSupportDepth : -1;

	Nv::Blast::AuthoringResult* PreviousResult = Settings->FractureSession->FractureData.Get();
	std::vector<NvcVec2i> AdjChunks(PreviousResult->bondCount);
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
	std::vector<uint32> SelectionBuffer;
	SelectionBuffer.reserve(SelectedChunkIndices.Num());
	for (int32 i : SelectedChunkIndices)
	{
		const int32 ChunkIndex = GetChunkToolIndexFromSessionIndex(Settings->FractureSession, i);
		if (ChunkIndex < 0)
		{
			return;
		}
		SelectionBuffer.push_back((uint32)ChunkIndex);
	}
	Settings->FractureSession->FractureTool->uniteChunks(Threshold, TargetedClusterSize, SelectionBuffer.data(), (uint32)SelectionBuffer.size(),
		AdjChunks.data(), (uint32)AdjChunks.size(), RemoveMergedOriginalChunks);

	SelectedChunkIndices.Empty();

	Settings->FractureSession->IsMeshModified = true;
	LoadFracturedMesh(Settings->FractureSession, SupportLevel, nullptr, Settings->InteriorMaterial);
}

bool CreatePhysXAsset(TSharedPtr<Nv::Blast::AuthoringResult> FractureData, UBlastMesh* BlastMesh, UFbxSkeletalMeshImportData* SkelMeshImportData = nullptr)
{
	if (SkelMeshImportData == nullptr)
	{
		SkelMeshImportData = Cast<UFbxSkeletalMeshImportData>(BlastMesh->Mesh->GetAssetImportData());
	}
	FBlastScopedProfiler PACP("PhysicsAssetCreation");
	TMap<FName, TArray<FBlastCollisionHull>> hulls;
	hulls.Empty();
	auto Converter = UBlastMeshFactory::GetTransformBlastToUE4CoordinateSystem(SkelMeshImportData);
	for (uint32 ci = 0; ci < FractureData->chunkCount; ci++)
	{
		auto& chunkHulls = hulls.Add(*(FString("chunk_") + FString::FromInt(ci)));
		for (uint32 hi = FractureData->collisionHullOffset[ci]; hi < FractureData->collisionHullOffset[ci + 1]; hi++)
		{
			FBlastCollisionHull h;
			auto fh = FractureData->collisionHull[hi];
			for (uint32 pi = 0; pi < fh->pointsCount; pi++)
			{
				auto p = fh->points[pi];
				h.Points.Add(Converter.TransformPosition(FVector(p.x, p.y, p.z)));
			}
			for (uint32 ii = 0; ii < fh->indicesCount; ii++)
			{
				h.Indices.Add(fh->indices[ii]);
			}
			for (uint32 pdi = 0; pdi < fh->polygonDataCount; pdi++)
			{
				h.PolygonData.Add(*reinterpret_cast<FBlastCollisionHull::HullPolygon*>(&fh->polygonData[pdi]));
			}
			chunkHulls.Add(h);
		}
	}
	if (!UBlastMeshFactory::RebuildPhysicsAsset(BlastMesh, hulls))
	{
		return false;
	}
	return true;
}

void FBlastFracture::RebuildCollisionMesh(UBlastFractureSettings* Settings, uint32 MaxNumOfConvex, uint32 Resolution, float Concavity, const TSet<int32>& ChunkIndices)
{
	FScopeLock Lock(&ExclusiveFractureSection);

	if (Settings == nullptr || !Settings->FractureSession.IsValid() || !Settings->FractureSession->IsRootFractured)
	{
		return;
	}
	FScopedSlowTask SlowTask(1, LOCTEXT("RebuildCollisionMesh", "Recalculating collision mesh, this may take a while."));

	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();

	auto FractureData = Settings->FractureSession->FractureData;
	Nv::Blast::ConvexDecompositionParams Param;
	Param.maximumNumberOfHulls = MaxNumOfConvex;
	Param.voxelGridResolution = Resolution;
	Param.concavity = Concavity;

	FCollisionBuilder CollisionBuilder;
	NvBlastExtAuthoringBuildCollisionMeshes(*FractureData, CollisionBuilder, Param, ChunkIndices.Num(), ChunkIndices.Num() ? (uint32*)&ChunkIndices.Array()[0] : nullptr);

	if (FractureData->collisionHull != nullptr)
	{
		CreatePhysXAsset(FractureData, Settings->FractureSession->BlastMesh);
	}
}

void FBlastFracture::RemoveSubhierarchy(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices, bool bIncludeRoot)
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

void BoneRearragementDfs(uint32 v, TArray<TArray<uint32>>& graph, TArray<bool>& used, TArray<uint32>& chunkListInOrder)
{
	used[v] = true;
	for (int32 id = 0; id < graph[v].Num(); ++id)
	{
		if (!used[graph[v][id]])
		{
			BoneRearragementDfs(graph[v][id], graph, used, chunkListInOrder);
		}
	}
	chunkListInOrder.Add(v);
}

void FBlastFracture::ReloadGraphicsMesh(FFractureSessionPtr FractureSession, int32 DefaultSupportDepth, UStaticMesh* InSourceStaticMesh, UMaterialInterface* InteriorMaterial)
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
				NvBlastExtAuthoringProcessFracture(*FS->FractureTool, *BondGenerator, CollisionBuilder, param, DefaultSupportDepth),
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


	if (FS->ChunkToBoneIndex.Num() == 0)
	{
		TArray<uint32> chunkListInOrder;

		{
			TArray<TArray<uint32> > graph;
			graph.SetNum(FS->FractureData->chunkCount);
			for (uint32 ci = 0; ci < FS->FractureData->chunkCount; ++ci)
			{
				if (FS->FractureData->chunkDescs[ci].parentChunkDescIndex != UINT32_MAX)
				{
					graph[FS->FractureData->chunkDescs[ci].parentChunkDescIndex].Add(ci);
				}
			}

			TArray<bool> used;
			used.SetNumZeroed(FS->FractureData->chunkCount);

			chunkListInOrder.Add(0);
			used[0] = true;
			for (uint32 ci = 0; ci < FS->FractureData->chunkCount; ++ci)
			{
				if (!used[ci])
				{
					BoneRearragementDfs(ci, graph, used, chunkListInOrder);
				}
			}
		}

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

bool FBlastFracture::LoadFractureData(FFractureSessionPtr FractureSession, int32 DefaultSupportDepth, UStaticMesh* InSourceStaticMesh)
{
	auto FS = FractureSession.Pin();
	UBlastMesh* BlastMesh = FS->BlastMesh;
	if (!FS.IsValid() || (InSourceStaticMesh == nullptr && BlastMesh == nullptr))
	{
		return false;
	}

	if (FS->FractureData.IsValid())
	{
		FS->FractureIdMap.Reset(FS->FractureData->chunkCount);
		for (uint32 i = 0; i < FS->FractureData->chunkCount; i++)
		{
			FS->FractureIdMap.Add(GetFractureChunkId(FS, i));
		}
	}
	else
	{
		FS->FractureIdMap.Empty();
	}

	FCollisionBuilder CollisionBuilder;
	auto BondGenerator = NvBlastExtAuthoringCreateBondGenerator(&CollisionBuilder);
	Nv::Blast::ConvexDecompositionParams param;
	param.maximumNumberOfHulls = 1;
	param.voxelGridResolution = 0;
	{
		FBlastScopedProfiler EAPFP("NvBlastExtAuthoringProcessFracture");
		FS->FractureData = TSharedPtr<Nv::Blast::AuthoringResult>(
			NvBlastExtAuthoringProcessFracture(*FS->FractureTool, *BondGenerator, CollisionBuilder, param, DefaultSupportDepth),
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

	TArray<uint32> chunkListInOrder;

	{
		TArray<TArray<uint32> > graph;
		graph.SetNum(FS->FractureData->chunkCount);
		for (uint32 ci = 0; ci < FS->FractureData->chunkCount; ++ci)
		{
			if (FS->FractureData->chunkDescs[ci].parentChunkDescIndex != UINT32_MAX)
			{
				graph[FS->FractureData->chunkDescs[ci].parentChunkDescIndex].Add(ci);
			}
		}

		TArray<bool> used;
		used.SetNumZeroed(FS->FractureData->chunkCount);

		chunkListInOrder.Add(0);
		used[0] = true;
		for (uint32 ci = 0; ci < FS->FractureData->chunkCount; ++ci)
		{
			if (!used[ci])
			{
				BoneRearragementDfs(ci, graph, used, chunkListInOrder);
			}
		}
	}

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
	uint32 ChunkCount = FS->FractureTool->getChunkCount();
	FTD->VerticesOffset.Reset(ChunkCount + 1);
	FTD->EdgesOffset.Reset(ChunkCount + 1);
	FTD->FacesOffset.Reset(ChunkCount + 1);
	FTD->VerticesOffset.Push(0);
	FTD->EdgesOffset.Push(0);
	FTD->FacesOffset.Push(0);
	for (uint32 AssetIndex = 0; AssetIndex < ChunkCount; AssetIndex++)
	{
		const uint32 ChunkIndex = FS->FractureTool->getChunkInfoIndex(FS->FractureData->chunkDescs[AssetIndex].userData);
		auto& Info = FS->FractureTool->getChunkInfo(ChunkIndex);
		FTD->VerticesOffset.Push(FTD->VerticesOffset.Last() + Info.getMesh()->getVerticesCount());
		FTD->EdgesOffset.Push(FTD->EdgesOffset.Last() + Info.getMesh()->getEdgesCount());
		FTD->FacesOffset.Push(FTD->FacesOffset.Last() + Info.getMesh()->getFacetCount());
	}
	FTD->Vertices.SetNumUninitialized(FTD->VerticesOffset.Last() * sizeof(Nv::Blast::Vertex));
	FTD->Edges.SetNumUninitialized(FTD->EdgesOffset.Last() * sizeof(Nv::Blast::Edge));
	FTD->Faces.SetNumUninitialized(FTD->FacesOffset.Last() * sizeof(Nv::Blast::Facet));
	FTD->ChunkFlags.SetNumZeroed(ChunkCount);

	for (uint32 AssetIndex = 0; AssetIndex < ChunkCount; AssetIndex++)
	{
		const uint32 ChunkIndex = FS->FractureTool->getChunkInfoIndex(FS->FractureData->chunkDescs[AssetIndex].userData);
		auto& Info = FS->FractureTool->getChunkInfo(ChunkIndex);
		FMemory::Memcpy(FTD->Vertices.GetData() + FTD->VerticesOffset[AssetIndex] * sizeof(Nv::Blast::Vertex),
			Info.getMesh()->getVertices(), Info.getMesh()->getVerticesCount() * sizeof(Nv::Blast::Vertex));
		for (uint32 v = 0; v < Info.getMesh()->getVerticesCount(); v++)
		{
			((Nv::Blast::Vertex*)FTD->Vertices.GetData() + FTD->VerticesOffset[AssetIndex] + v)->p = Info.getTmToWorld().transformPos(Info.getMesh()->getVertices()[v].p);
		}
		FMemory::Memcpy(FTD->Edges.GetData() + FTD->EdgesOffset[AssetIndex] * sizeof(Nv::Blast::Edge),
			Info.getMesh()->getEdges(), Info.getMesh()->getEdgesCount() * sizeof(Nv::Blast::Edge));
		FMemory::Memcpy(FTD->Faces.GetData() + FTD->FacesOffset[AssetIndex] * sizeof(Nv::Blast::Facet),
			Info.getMesh()->getFacetsBuffer(), Info.getMesh()->getFacetCount() * sizeof(Nv::Blast::Facet));
		if (Info.flags & (uint32)Nv::Blast::ChunkInfo::APPROXIMATE_BONDING)
		{
			FTD->ChunkFlags[AssetIndex] |= EBlastMeshChunkFlags::ApproximateBonding;
		}
	}
}

void FBlastFracture::LoadFracturedMesh(FFractureSessionPtr FractureSession, int32 DefaultSupportDepth, UStaticMesh* InSourceStaticMesh, UMaterialInterface* InteriorMaterial)
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

	if (!CreatePhysXAsset(FS->FractureData, BlastMesh, SkelMeshImportData))
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
			if (MatSlot.ImportedMaterialSlotName.IsEqual(FBlastFracture::InteriorMaterialID, ENameCase::IgnoreCase, bCompareTrailingNumber))
			{
				Settings->InteriorMaterialSlotName = MatSlot.ImportedMaterialSlotName;
				break;
			}
		}
	}
}

TSharedPtr <Nv::Blast::VoronoiSitesGenerator> FBlastFracture::GetVoronoiSitesGenerator(TSharedPtr<FFractureSession> FractureSession, int32 FractureChunkId, bool ForceReset)
{
	auto SitesGenerator = FractureSession->SitesGeneratorMap.Find(FractureChunkId);
	if (SitesGenerator == nullptr || !SitesGenerator->Generator.IsValid() || ForceReset)
	{
		FractureSession->SitesGeneratorMap.Remove(FractureChunkId);
		SitesGenerator = &(FractureSession->SitesGeneratorMap.Add(FractureChunkId, FFractureSession::ChunkSitesGenerator()));
		SitesGenerator->Mesh = TSharedPtr <Nv::Blast::Mesh>(FractureSession->FractureTool->createChunkMesh(FractureChunkId), [](Nv::Blast::Mesh* p)
			{
				p->release();
			});
		if (SitesGenerator->Mesh.Get() == nullptr)
		{
			return TSharedPtr <Nv::Blast::VoronoiSitesGenerator>(nullptr);
		}
		SitesGenerator->Generator = TSharedPtr <Nv::Blast::VoronoiSitesGenerator>(
			NvBlastExtAuthoringCreateVoronoiSitesGenerator(SitesGenerator->Mesh.Get(), RandomGenerator.Get()),
			[](Nv::Blast::VoronoiSitesGenerator* p)
			{
				p->release();
			});
	}
	return SitesGenerator->Generator;
};

bool FBlastFracture::FractureVoronoi(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	uint32 CellCount, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset)
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
		NvcVec3 ca = { CellAnisotropy.X, CellAnisotropy.Y, CellAnisotropy.Z };
		NvcQuat cr = { CellRotation.X, CellRotation.Y, CellRotation.Z, CellRotation.W };
		{
			FBlastScopedProfiler VFP("FractureVoronoi::voronoiFracturing");
			if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites, ca, cr, IsReplace) == 0)
			{
				return true;
			}
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi"));
	return false;
}

bool FBlastFracture::FractureClusteredVoronoi(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	uint32 CellCount, uint32 ClusterCount, float ClusterRadius, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset)
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
		NvcVec3 ca = { CellAnisotropy.X, CellAnisotropy.Y, CellAnisotropy.Z };
		NvcQuat cr = { CellRotation.X, CellRotation.Y, CellRotation.Z, CellRotation.W };
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites, ca, cr, IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with clustered Voronoi"));
	return false;
}

bool FBlastFracture::FractureRadial(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	FVector Origin, FVector Normal, float Radius, uint32 AngularSteps, uint32 RadialSteps, float AngleOffset, float Variability,
	FVector CellAnisotropy, FQuat CellRotation, bool ForceReset)
{
	FBlastScopedProfiler FRP("FractureRadial");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		Normal.Normalize();
		NvcVec3 O = { Origin.X, Origin.Y, Origin.Z };
		NvcVec3 N = { Normal.X, Normal.Y, Normal.Z };
		VoronoiSitesGenerator->radialPattern(O, N, Radius, AngularSteps, RadialSteps, AngleOffset, Variability);
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		NvcVec3 ca = { CellAnisotropy.X, CellAnisotropy.Y, CellAnisotropy.Z };
		NvcQuat cr = { CellRotation.X, CellRotation.Y, CellRotation.Z, CellRotation.W };
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites, ca, cr, IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi"));
	return false;
}

bool FBlastFracture::FractureInSphere(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	uint32 CellCount, float Radius, FVector Origin, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset)
{
	FBlastScopedProfiler FISP("FractureInSphere");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		NvcVec3 O = { Origin.X, Origin.Y, Origin.Z };
		VoronoiSitesGenerator->generateInSphere(CellCount, Radius, O);
		const NvcVec3* sites = nullptr;
		uint32 sitesCount = VoronoiSitesGenerator->getVoronoiSites(sites);
		if (FractureSession->FractureTool->getChunkDepth(FractureChunkId) == 0)
		{
			IsReplace = false;
		}
		NvcVec3 ca = { CellAnisotropy.X, CellAnisotropy.Y, CellAnisotropy.Z };
		NvcQuat cr = { CellRotation.X, CellRotation.Y, CellRotation.Z, CellRotation.W };
		if (FractureSession->FractureTool->voronoiFracturing(FractureChunkId, sitesCount, sites, ca, cr, IsReplace) == 0)
		{
			return true;
		}
	}
	UE_LOG(LogBlastMeshEditor, Error, TEXT("Failed to fracture with Voronoi in sphere"));
	return false;
}

bool FBlastFracture::RemoveInSphere(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	float Radius, FVector Origin, float Probability, bool ForceReset)
{
	FBlastScopedProfiler RISP("RemoveInSphere");
	RandomGenerator->seed(RandomSeed);
	auto VoronoiSitesGenerator = GetVoronoiSitesGenerator(FractureSession, FractureChunkId, ForceReset);
	if (VoronoiSitesGenerator.IsValid())
	{
		NvcVec3 O = { Origin.X, Origin.Y, Origin.Z };
		VoronoiSitesGenerator->deleteInSphere(Radius, O, Probability);
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

bool FBlastFracture::FractureUniformSlicing(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	FIntVector SlicesCount, float AngleVariation, float OffsetVariation,
	float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber, FVector SamplingInterval)
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

bool FBlastFracture::FractureCutout(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	UTexture2D* Pattern, FVector Origin, FVector Normal, FVector2D Size, float RotationZ, float Aperture, bool bPeriodic, bool bFillGaps,
	float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber, FVector SamplingInterval)
{
	FBlastScopedProfiler FCP("FractureCutout");
	RandomGenerator->seed(RandomSeed);

	Normal.Normalize();
	auto UE4ToBlastTransform = UBlastMeshFactory::GetTransformUE4ToBlastCoordinateSystem(Cast<UFbxSkeletalMeshImportData>(FractureSession->BlastMesh->Mesh->GetAssetImportData()));
	//FTransform ScaleTr; ScaleTr.SetScale3D(FVector(Scale.X, Scale.Y, 1.f) * 0.5f);
	FTransform YawTr(FQuat(FVector(0., 0., 1.), FMath::DegreesToRadians(RotationZ)));
	FTransform Tr(FQuat::FindBetweenNormals(FVector(0., 0., 1.), Normal), Origin);
	Tr = YawTr * Tr * UE4ToBlastTransform;

	Nv::Blast::CutoutConfiguration CutoutConfig;
	CutoutConfig.transform.q = ToNvQuat(Tr.GetRotation());
	CutoutConfig.transform.p = ToNvVector(Tr.GetLocation());
	CutoutConfig.scale = { static_cast<float>(Size.X), static_cast<float>(Size.Y) };
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

		TArray64 <uint8_t> Buf, Mip;
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
		NvBlastExtAuthoringBuildCutoutSet(*CutoutConfig.cutoutSet, Buf.GetData(), Pattern->Source.GetSizeX(), Pattern->Source.GetSizeY(), SegmentationErrorThreshold, SnapThreshold, bPeriodic, bFillGaps);
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
	if (FractureChunkId != 0 || FractureSession->BlastMesh == nullptr)
	{
		return false;
	}
	FractureSession->FractureTool->islandDetectionAndRemoving(0, true);
	return true;
}

bool FBlastFracture::FractureCut(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
	FVector Origin, FVector Normal,
	float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber, FVector SamplingInterval)
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
	if (FractureSession->FractureTool->cut(FractureChunkId, ToNvVector(Normal), ToNvVector(Origin), NoiseConfig, IsReplace, RandomGenerator.Get()) != 0)
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
