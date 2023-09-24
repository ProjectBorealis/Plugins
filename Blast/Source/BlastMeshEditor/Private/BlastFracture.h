// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "BlastFracture.generated.h"

class UMaterialInterface;
class UStaticMesh;
class UBlastMesh;
class UBlastFractureSettings;
class UBlastFractureSettingsConfig;
class UTexture2D;
struct FRawMesh;

namespace Nv
{
	namespace Blast
	{
		struct AuthoringResult;
		class FractureTool;
		class VoronoiSitesGenerator;
		class Mesh;
	}
}

UENUM()
enum class EFractureScriptParseResult : uint8
{
	OK = 0,
	ARGS_OR_CHUNKS_NOT_FOUND,
	WRONG_CHUNK_INDEX,
	CANNOT_PARSE_VORONOI_ARGS,
	CANNOT_PARSE_CLUSTERED_VORONOI_ARGS,
	CANNOT_PARSE_UNIFORM_SLICING_ARGS,
	VORONOI_FRACTURE_FAILS,
	CLUSTERED_VORONOI_FRACTURE_FAILS,
	UNIFORMS_SLICING_FRACTURE_FAILS,
	UNKNOWN_ERROR
};

struct FFractureSession
{
	UBlastMesh* BlastMesh = nullptr;
	TSharedPtr<Nv::Blast::AuthoringResult> FractureData;
	TSharedPtr<Nv::Blast::FractureTool> FractureTool;
	TArray<uint32> FractureIdMap;
	TMap<int32, int32> ChunkToBoneIndex;
	TMap<int32, int32> ChunkToBoneIndexPrev;

	bool IsRootFractured = false;
	bool IsMeshCreatedFromFractureData = false;
	bool IsMeshModified = false;

	//For interactive fracture
	struct ChunkSitesGenerator
	{
		TSharedPtr<Nv::Blast::VoronoiSitesGenerator> Generator;
		TSharedPtr<Nv::Blast::Mesh> Mesh;
	};
	TMap<int32, ChunkSitesGenerator> SitesGeneratorMap;
};
typedef TWeakPtr<FFractureSession> FFractureSessionPtr;

class FBlastFracture : public FGCObject
{
public:
	FBlastFracture();
	~FBlastFracture();

	static TSharedPtr<FBlastFracture> GetInstance();

	UBlastFractureSettings* CreateFractureSettings(class FBlastMeshEditor* Editor) const;

	UBlastFractureSettingsConfig* GetConfig()
	{
		return Config;
	}

	//Runtime fracture
	TSharedPtr<FFractureSession> StartFractureSession(UBlastMesh* InSourceBlastMesh,
		UStaticMesh* InSourceStaticMesh = nullptr, UBlastFractureSettings* Settings = nullptr, bool ForceLoadFracturedMesh = false);

	void Fracture(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices, int32 ClickedChunkIndex = INDEX_NONE);

	void BuildChunkHierarchy(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices, uint32 Threshold, uint32 TargetedClusterSize, bool RemoveMergedOriginalChunks);

	void FitUvs(UBlastFractureSettings* Settings, float Size, bool OnlySpecified, TSet<int32>& ChunkIndices);

	void RebuildCollisionMesh(UBlastFractureSettings* Settings, uint32_t MaxNumOfConvex, uint32_t Resolution, float Concavity, const TSet<int32>& ChunkIndices);

	void RemoveSubhierarchy(UBlastFractureSettings* Settings, TSet<int32>& SelectedChunkIndices, bool bIncludeRoot);

	void FinishFractureSession(FFractureSessionPtr FractureSession);

	void GetVoronoiSites(TSharedPtr<FFractureSession> FractureSession, int32 ChunkId, TArray<FVector>& Sites);

	bool CanUndo(UBlastFractureSettings* Settings) const;

	void Undo(UBlastFractureSettings* Settings);

	bool CanRedo(UBlastFractureSettings* Settings) const;

	void Redo(UBlastFractureSettings* Settings);

	const static FName InteriorMaterialID;

private:

	FBlastFracture(const FBlastFracture&) = delete;

	FBlastFracture& operator= (const FBlastFracture&) = delete;

	static TWeakPtr<FBlastFracture> Instance;

	bool LoadFractureData(FFractureSessionPtr FractureSession, int32_t DefaultSupportDepth, UStaticMesh* InSourceStaticMesh);
	void LoadFractureToolData(TSharedPtr<FFractureSession> FractureSession);
	void LoadFracturedMesh(FFractureSessionPtr FractureSession, int32_t DefaultSupportDepth, UStaticMesh* InSourceStaticMesh = nullptr, UMaterialInterface* InteriorMaterial = nullptr);
	void ReloadGraphicsMesh(FFractureSessionPtr FractureSession, int32_t DefaultSupportDepth, UStaticMesh* InSourceStaticMesh = nullptr, UMaterialInterface* InteriorMaterial = nullptr);
	void PopulateSettingsFromBlastMesh(UBlastFractureSettings* Settings, UBlastMesh* BlastMesh);

	TSharedPtr <Nv::Blast::VoronoiSitesGenerator> GetVoronoiSitesGenerator(TSharedPtr<FFractureSession> FractureSession, int32 FractureChunkId, bool ForceReset);

	bool FractureVoronoi(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		uint32 CellCount, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset);

	bool FractureClusteredVoronoi(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		uint32 CellCount, uint32 ClusterCount, float ClusterRadius, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset);

	bool FractureRadial(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		FVector Origin, FVector Normal, float Radius, uint32_t AngularSteps, uint32_t RadialSteps, float AngleOffset,
		float Variability, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset);

	bool FractureInSphere(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		uint32 CellCount, float Radius, FVector Origin, FVector CellAnisotropy, FQuat CellRotation, bool ForceReset);

	bool RemoveInSphere(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		float Radius, FVector Origin, float Probability, bool ForceReset);

	bool FractureUniformSlicing(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		FIntVector Slices, float AngleVariation, float OffsetVariation,
		float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber, FVector SamplingInterval);

	bool FractureCutout(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		UTexture2D* Pattern, FVector Origin, FVector Normal, FVector2D Size, float RotationZ, float Aperture, bool bPeriodic, bool bFillGaps,
		float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber, FVector SamplingInterval);

	bool FractureCut(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId, int32 RandomSeed, bool IsReplace,
		FVector Origin, FVector Normal,
		float NoiseAmplitude, float NoiseFrequency, int32 NoiseOctaveNumber, FVector SamplingInterval);

	bool FractureChunksFromIslands(TSharedPtr<FFractureSession> FractureSession, uint32 FractureChunkId);

private:

	FCriticalSection							ExclusiveFractureSection;

	TObjectPtr<UBlastFractureSettingsConfig>	Config;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Config);
	}

	virtual FString GetReferencerName() const override
	{
		return TEXT("FBlastFracture");
	}

	TSharedPtr <class FFractureRandomGenerator> RandomGenerator;
};
