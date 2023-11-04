#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Serialization/BulkData.h"

#include "BlastAsset.generated.h"

struct NvBlastAsset;

UENUM()
enum class EBlastAssetChunkFlags : uint8
{
	None = 0x00,  // None
	Static = 0x01 // Static/Kinematic chunk
};
ENUM_CLASS_FLAGS(EBlastAssetChunkFlags);


UENUM()
enum class EBlastMeshChunkFlags : uint8
{
	None = 0x00,  // None
	ApproximateBonding = 0x01
};
ENUM_CLASS_FLAGS(EBlastMeshChunkFlags);

struct FBlastEdge
{
    uint32 S;
    uint32 E;
};

struct FBlastVertex
{
    FVector3f P;      // Position
    FVector3f N;      // Normal
    FVector2f UV[1];  // UV-coordinates array, currently supported only one UV coordinate.
};

struct FBlastFacet
{
    int32 FirstEdgeNumber;
    uint32 EdgesCount;
    int64 UserData;
    int32 MaterialId;
    int32 SmoothingGroup;
};

struct FBlastChunkMesh
{
	TArray<FBlastVertex> Vertices;
	TArray<FBlastEdge> Edges;
	TArray<FBlastFacet> Faces;
	EBlastMeshChunkFlags ChunkFlag = EBlastMeshChunkFlags::None;
};

USTRUCT()
struct FBlastFractureToolData
{
	GENERATED_USTRUCT_BODY()

	// Chunks by asset index
	TArray<FBlastChunkMesh> ChunkMeshes;
};

#if WITH_EDITOR
struct FBlastFractureHistory
{
	enum { kDefaultCapacity = 5 };

	FBlastFractureHistory() : Tail(0), Head(0), Curr(0), Capacity(kDefaultCapacity)
	{
		ToolDataHistory.InsertDefaulted(0, Capacity);
		LoadedAssetHistory.InsertDefaulted(0, Capacity);
	}

	FBlastFractureToolData& GetCurrentToolData()
	{
		return ToolDataHistory[Curr];
	}

	const FBlastFractureToolData& GetCurrentToolData() const
	{
		return ToolDataHistory[Curr];
	}

	TSharedPtr<NvBlastAsset>& GetCurrentLoadedAsset()
	{
		return LoadedAssetHistory[Curr];
	}

	const TSharedPtr<NvBlastAsset>& GetCurrentLoadedAsset() const
	{
		return LoadedAssetHistory[Curr];
	}

	void Push()
	{
		EraseNextToHead(Curr);
		Head = Curr = Next(Curr);
		if (Tail == Head)
		{
			Tail = Next(Tail);
		}
	}

	bool Undo()
	{
		if (Curr != Tail)
		{
			Curr = Prev(Curr);
			return true;
		}
		return false;
	}

	bool Redo()
	{
		if (Curr != Head)
		{
			Curr = Next(Curr);
			return true;
		}
		return false;
	}

	bool CanUndo() const
	{
		return Curr != Tail;
	}

	bool CanRedo() const
	{
		return Curr != Head;
	}

	// Reduce history to one entry
	void Collapse()
	{
		if (Capacity > 1)
		{
			if (Curr != 0)
			{
				ToolDataHistory[0] = ToolDataHistory[Curr];
				LoadedAssetHistory[0] = LoadedAssetHistory[Curr];
			}
			Tail = Head = Curr = 0;
			Capacity = 1;
			ToolDataHistory.SetNum(Capacity, false);
			LoadedAssetHistory.SetNum(Capacity, false);
		}
	}

private:
	int32 Prev(int32 Index)
	{
		return Index ? Index - 1 : Capacity - 1;
	}

	int32 Next(int32 Index)
	{
		return Index < Capacity - 1 ? Index + 1 : 0;
	}

	void EraseNextToHead(int32 Index)
	{
		while (Index != Head)
		{
			Index = Next(Index);
			ToolDataHistory[Index] = FBlastFractureToolData();
			LoadedAssetHistory[Index] = nullptr;
		}
	}

	TArray<FBlastFractureToolData> ToolDataHistory{};

	/*
		This is the pointer to the deserialized/loaded asset. This doesn't get populated until required
		or the user tells us to load it.

		This is wrapped in a shared_ptr so that we can call release when it leaves scope.
	*/
	TArray<TSharedPtr<NvBlastAsset>> LoadedAssetHistory{};

	int32 Tail;
	int32 Head;
	int32 Curr;
	int32 Capacity;
};
#endif


/*
	This is the Blast Asset, which is the LL asset.

	Blast has chunks, which can have 1-N subchunks, these are graphics and physics only

*/
UCLASS()
class BLAST_API UBlastAsset : public UObject
{
	GENERATED_UCLASS_BODY()
public:

#if WITH_EDITOR
	/*
		Initialize with NvBlastAsset to keep. It will be serialized into buffer, pointer won't be stored.
	*/
	virtual void CopyFromLoadedAsset(const NvBlastAsset* AssetToCopy, const FGuid& NewAssetGUID = FGuid::NewGuid());
#endif
	/*
		Gets the loaded asset
	*/
	NvBlastAsset* GetLoadedAsset() const;

	bool IsLoadedAssetValid() const;

	/*
		Get assets root chunks
	*/
	const TArray<uint32>& GetRootChunks() const;

	uint32	GetChunkCount() const;

	uint32	GetBondCount() const;

	const struct NvBlastChunk& GetChunkInfo(uint32 ChunkIndex) const;

	bool IsSupportChunk(uint32 ChunkIndex) const;

	uint32 GetChunkDepth(uint32 ChunkIndex) const;

	uint32 GetMaxChunkDepth() const
	{
		return MaxChunkDepth;
	}

	bool IsChunkStatic(uint32 ChunkIndex) const;

	// (!Note) This function also changes states of other chunks:
	// If 'true' passed chunk (ChunkIndex) and all parent chunks till the root become static
	// If 'false' passed chunk (ChunkIndex) and all children chunks become non-static
	void SetChunkStatic(uint32 ChunkIndex, bool IsStatic);


	virtual void PostLoad() override;

	virtual void Serialize(FArchive& Ar) override;

	inline const FGuid& GetAssetGUID() const { return AssetGUID; }

	// NvBlastAsset serialization wrappers
	static NvBlastAsset* DeserializeBlastAsset(const void* buffer, uint64_t bufferSize);
	static uint64 SerializeBlastAsset(void*& buffer, const NvBlastAsset* asset);

private:

	void	DeserializeRawAsset();
	void	Update();

	void	BuildChunkMaxDepth();

	// Per chunk flags
	TArray<EBlastAssetChunkFlags>			ChunksFlags;

	UPROPERTY()
	FGuid									AssetGUID;

	/*
	This is the raw, serialized data of the asset. It is deserialized into LoadedAsset on demand.
	It's stored in BulkData so we don't waste memory keeping it when it's not needed
	*/
	FByteBulkData							RawAssetData;

	/*
	List of asset's root chunks, updated when asset is loaded
	*/
	TArray<uint32>							RootChunks;

	TArray<uint32>							SupportChunks;

	TArray<uint32>							ChunksDepth;

	uint32									MaxChunkDepth;

public:
#if WITH_EDITOR
	FBlastFractureHistory					FractureHistory;
#else
	NvBlastAsset* Asset;
#endif
};

