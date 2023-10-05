// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BlastMeshUtilities.h"
#include "BlastFracture.h"
#include "BlastMesh.h"
#include "BlastMeshFactory.h"
#include "BlastGlobals.h"

#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringTypes.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "Modules/ModuleManager.h"

#include "RawIndexBuffer.h"
#include "ComponentReregisterContext.h"
#include "Materials/Material.h"
#include "SkeletalMeshTypes.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include "Animation/Skeleton.h"
#include "MeshUtilities.h"
#include "MaterialDomain.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Runtime/Core/Public/Misc/FeedbackContext.h"

#include "../Private/MeshMergeHelpers.h"
#include "../Private/SkeletalMeshTools.h"
#include "GPUSkinVertexFactory.h"
#include "RawMesh.h"

#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/MultiSizeIndexContainer.h"
#include "OverlappingCorners.h"

#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Misc/CoreMisc.h"

#define LOCTEXT_NAMESPACE "BlastMeshEditor"

#define USE_FRACTURE_UPDATE 0

void BuildSmoothingGroups(FRawMesh& RawMesh)
{
	uint32 FacesCount = RawMesh.WedgeIndices.Num() / 3;
	uint32 SmoothingGroupCount = 0;
	TMap<uint32, uint32> SmoothingGroupVertexMap;
	TMap<uint32, uint32> SmoothingGroupFaceMap;
	check(FacesCount == RawMesh.FaceSmoothingMasks.Num())
		for (uint32 i = 0; i < FacesCount; i++)
		{
			uint32 FaceSmoothingGroup = 0;
			TSet<uint32> AdjacentFacesSmoothingGroup;
			for (uint32 j = 0; j < i; j++)
			{
				FVector3f P1[3], P2[3];
				for (uint32 k = 0; k < 3; k++)
				{
					P1[k] = RawMesh.VertexPositions[RawMesh.WedgeIndices[3 * i + k]];
					P2[k] = RawMesh.VertexPositions[RawMesh.WedgeIndices[3 * j + k]];
				}
				TArray<TPair<uint32, uint32>> Matches;
				for (uint32 ki = 0; ki < 3; ki++)
				{
					for (uint32 kj = 0; kj < 3; kj++)
					{
						if (FVector3f::PointsAreSame(P1[ki], P2[kj]))
						{
							Matches.Push(TPair<uint32, uint32>(ki, kj));
						}
					}
				}
				if (Matches.Num() == 2 || Matches.Num() == 1) // Adjacent faces
				{
					bool IsHardEdge = false;
					for (int32 k = 0; k < Matches.Num(); k++)
					{
						IsHardEdge |= !FVector3f::PointsAreNear(RawMesh.WedgeTangentZ[3 * i + Matches[k].Key], RawMesh.WedgeTangentZ[3 * j + Matches[k].Value], 1e-3);
					}
					uint32* SG = SmoothingGroupFaceMap.Find(j);
					if (SG != nullptr)
					{
						if (IsHardEdge)
						{
							AdjacentFacesSmoothingGroup.Add(*SG);
						}
						else
						{
							FaceSmoothingGroup = *SG;
							break;
						}
					}
				}
			}
			if (FaceSmoothingGroup == 0)
			{
				FaceSmoothingGroup = 1;
				while (FaceSmoothingGroup && AdjacentFacesSmoothingGroup.Contains(FaceSmoothingGroup))
				{
					FaceSmoothingGroup <<= 1;
				}
			}
			SmoothingGroupFaceMap.Add(i, FaceSmoothingGroup);
			RawMesh.FaceSmoothingMasks[i] = FaceSmoothingGroup;
		}
}

#if USE_FRACTURE_UPDATE
void BuildSkeletalModelFromChunks(FSkeletalMeshLODModel& LODModel, const FReferenceSkeleton& RefSkeleton, int32 MaxBonesPerChunk,
	TArray<FSkinnedMeshChunk*>& Chunks, const TArray<int32>& PointToOriginalMap, const TMap<int32, int32>& OldToNewBoneMap = {})
{
#if WITH_EDITORONLY_DATA
	// Reset 'final vertex to import vertex' map info
	LODModel.MeshToImportVertexMap.Empty();
	LODModel.MaxImportVertex = 0;
	LODModel.RawPointIndices.RemoveBulkData();

	//Remove unused bones
	TArray<int32> ChunksOldBoneNum;
	TArray<uint32> OldIndices = LODModel.IndexBuffer;

	LODModel.ActiveBoneIndices.Reset();

	for (auto& Section : LODModel.Sections)
	{
		TArray<int32> CurrentSectionBoneRemap;
		uint32 NewIndex = 0;
		for (int32 OldIndex = 0; OldIndex < Section.BoneMap.Num(); OldIndex++, NewIndex++)
		{
			if (OldToNewBoneMap[Section.BoneMap[OldIndex]] < 0)
			{
				CurrentSectionBoneRemap.Add(-1);
				NewIndex--;
			}
			else
			{
				CurrentSectionBoneRemap.Add(NewIndex);
			}
		}

		TArray<FSoftSkinVertex> OldSoftVertices;
		Exchange(OldSoftVertices, Section.SoftVertices);

		int32 RemovedVerticesCount = 0;//We don't remove duplicated vertices, so VertexIndex == Indices[VertexIndex]

		for (int32 VertexIndex = 0; VertexIndex < OldSoftVertices.Num(); VertexIndex++)
		{
			auto& V = OldSoftVertices[VertexIndex];
			if (!CurrentSectionBoneRemap.IsValidIndex(V.InfluenceBones[0]) || CurrentSectionBoneRemap[V.InfluenceBones[0]] < 0)
			{
				RemovedVerticesCount++;
			}
			else
			{
				V.InfluenceBones[0] = CurrentSectionBoneRemap[V.InfluenceBones[0]];
				Section.SoftVertices.Add(V);
			}
		}

		Section.NumVertices -= RemovedVerticesCount;
		Section.NumTriangles -= RemovedVerticesCount / 3;

		TArray<FBoneIndexType> BoneMap;
		Exchange(BoneMap, Section.BoneMap);
		for (auto BoneIndex : BoneMap)
		{
			int32 NewVal = OldToNewBoneMap[BoneIndex];
			if (NewVal >= 0)
			{
				Section.BoneMap.Add(NewVal);
			}
		}
	}

	// Setup the section and chunk arrays on the model.
	TMap<FSkelMeshSection*, int32> SectionToChunkIndexMap;
	LODModel.Sections.Reserve(LODModel.Sections.Num() + Chunks.Num());
	for (int32 ChunkIndex = 0; ChunkIndex < Chunks.Num(); ++ChunkIndex)
	{
		FSkinnedMeshChunk* SrcChunk = Chunks[ChunkIndex];
		FSkelMeshSection* Section = LODModel.Sections.FindByPredicate([&SrcChunk, &MaxBonesPerChunk](FSkelMeshSection& SkelSection)
			{
				return SkelSection.MaterialIndex == SrcChunk->MaterialIndex && SkelSection.BoneMap.Num() + SrcChunk->BoneMap.Num() < MaxBonesPerChunk;
			});
		if (Section == nullptr)
		{
			Section = new(LODModel.Sections) FSkelMeshSection();
			Section->MaterialIndex = SrcChunk->MaterialIndex;
			Section->NumTriangles = 0;
		}
		SectionToChunkIndexMap.Add(Section, ChunkIndex);
		ChunksOldBoneNum.Add(Section->BoneMap.Num());
		Section->BoneMap.Append(SrcChunk->BoneMap);
	}

	// Update the active bone indices on the LOD model.
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
	{
		FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
		for (int32 BoneIndex = 0; BoneIndex < Section.BoneMap.Num(); ++BoneIndex)
		{
			LODModel.ActiveBoneIndices.AddUnique(Section.BoneMap[BoneIndex]);
		}
	}

	// ensure parent exists with incoming active bone indices, and the result should be sorted	
	RefSkeleton.EnsureParentsExistAndSort(LODModel.ActiveBoneIndices);

	// Pack the chunk vertices into a single vertex buffer.
	int32 CurrentVertexIndex = 0; 			// current vertex index added to the index buffer for all chunks of the same material
	// rearrange the vert order to minimize the data fetched by the GPU
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
	{
		if (IsInGameThread())
		{
			GWarn->StatusUpdate(SectionIndex, LODModel.Sections.Num(), NSLOCTEXT("UnrealEd", "ProcessingSections", "Processing Sections"));
		}
		FSkelMeshSection* Section = &LODModel.Sections[SectionIndex];
		if (!SectionToChunkIndexMap.Contains(Section))
		{
			continue;
		}
		FSkinnedMeshChunk* SrcChunk = Chunks[*SectionToChunkIndexMap.Find(Section)];
		TArray<FSoftSkinBuildVertex>& ChunkVertices = SrcChunk->Vertices;
		TArray<uint32>& ChunkIndices = SrcChunk->Indices;

		// Reorder the section index buffer for better vertex cache efficiency.
		IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
		MeshUtilities.CacheOptimizeIndexBuffer(ChunkIndices);

		// Calculate the number of triangles in the section.  Note that CacheOptimize may change the number of triangles in the index buffer!
		TArray<FSoftSkinBuildVertex> OriginalVertices;
		Exchange(ChunkVertices, OriginalVertices);
		ChunkVertices.AddUninitialized(OriginalVertices.Num());

		TArray<int32> IndexCache;
		IndexCache.AddUninitialized(ChunkVertices.Num());
		FMemory::Memset(IndexCache.GetData(), INDEX_NONE, IndexCache.Num() * IndexCache.GetTypeSize());
		int32 NextAvailableIndex = 0;
		// Go through the indices and assign them new values that are coherent where possible
		for (int32 Index = 0; Index < ChunkIndices.Num(); Index++)
		{
			const int32 OriginalIndex = ChunkIndices[Index];
			const int32 CachedIndex = IndexCache[OriginalIndex];

			if (CachedIndex == INDEX_NONE)
			{
				// No new index has been allocated for this existing index, assign a new one
				ChunkIndices[Index] = NextAvailableIndex;
				// Mark what this index has been assigned to
				IndexCache[OriginalIndex] = NextAvailableIndex;
				NextAvailableIndex++;
			}
			else
			{
				// Reuse an existing index assignment
				ChunkIndices[Index] = CachedIndex;
			}
			// Reorder the vertices based on the new index assignment
			ChunkVertices[ChunkIndices[Index]] = OriginalVertices[OriginalIndex];
		}
	}

	// Keep track of index mapping to chunk vertex offsets
	TArray< TArray<uint32> > VertexIndexRemap;
	VertexIndexRemap.Empty(LODModel.Sections.Num());
	//int32 NewVertexCount = 0;

	// Build the arrays of rigid and soft vertices on the model's chunks.
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
	{
		FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
		Section.BaseVertexIndex = (SectionIndex > 0 ? LODModel.Sections[SectionIndex - 1].BaseVertexIndex + LODModel.Sections[SectionIndex - 1].NumVertices : 0);

		TArray<uint32>& ChunkVertexIndexRemap = *new(VertexIndexRemap)TArray<uint32>();
		if (!SectionToChunkIndexMap.Contains(&Section))
		{
			continue;
		}
		Section.NumTriangles += Chunks[(*SectionToChunkIndexMap.Find(&Section))]->Indices.Num() / 3;
		TArray<FSoftSkinBuildVertex>& ChunkVertices = Chunks[(*SectionToChunkIndexMap.Find(&Section))]->Vertices;

		if (IsInGameThread())
		{
			// Only update status if in the game thread.  When importing morph targets, this function can run in another thread
			GWarn->StatusUpdate(SectionIndex, LODModel.Sections.Num(), NSLOCTEXT("UnrealEd", "ProcessingChunks", "Processing Chunks"));
		}

		CurrentVertexIndex = Section.NumVertices;

		// Update the size of the vertex buffer.
		LODModel.NumVertices += ChunkVertices.Num();

		// Separate the section's vertices into rigid and soft vertices.
		ChunkVertexIndexRemap.AddUninitialized(ChunkVertices.Num());

		int32 OldBoneNum = ChunksOldBoneNum[(*SectionToChunkIndexMap.Find(&Section))];
		for (int32 VertexIndex = 0; VertexIndex < ChunkVertices.Num(); VertexIndex++)
		{
			const FSoftSkinBuildVertex& SoftVertex = ChunkVertices[VertexIndex];

			FSoftSkinVertex NewVertex;
			NewVertex.Position = SoftVertex.Position;
			NewVertex.TangentX = SoftVertex.TangentX;
			NewVertex.TangentY = SoftVertex.TangentY;
			NewVertex.TangentZ = SoftVertex.TangentZ;
			FMemory::Memcpy(NewVertex.UVs, SoftVertex.UVs, sizeof(FVector2f) * MAX_TEXCOORDS);
			NewVertex.Color = SoftVertex.Color;

			for (int32 i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
			{
				NewVertex.InfluenceBones[i] = 0;
				NewVertex.InfluenceWeights[i] = 0;
			}

			// it only adds to the bone map if it has weight on it
			// BoneMap contains only the bones that has influence with weight of >0.f
			// so here, just make sure it is included before setting the data
			if (Section.BoneMap.IsValidIndex(OldBoneNum + SoftVertex.InfluenceBones[0]))
			{
				NewVertex.InfluenceBones[0] = OldBoneNum + SoftVertex.InfluenceBones[0];
				NewVertex.InfluenceWeights[0] = SoftVertex.InfluenceWeights[0];
			}


			Section.SoftVertices.Add(NewVertex);
			ChunkVertexIndexRemap[VertexIndex] = (uint32)(Section.BaseVertexIndex + CurrentVertexIndex);
			CurrentVertexIndex++;
			// add the index to the original wedge point source of this vertex
			//RawPointIndices.Add(SoftVertex.PointWedgeIdx);
			// Also remember import index
			//const int32 RawVertIndex = RawMeshIndexOffset + PointToOriginalMap[SoftVertex.PointWedgeIdx];
			//TODO this is looks wrong
			//LODModel.MeshToImportVertexMap.Add(RawVertIndex);
			//LODModel.MaxImportVertex = FMath::Max<float>(LODModel.MaxImportVertex, RawVertIndex);
		}

		// update NumVertices
		Section.NumVertices = Section.SoftVertices.Num();

		// update max bone influences
		Section.CalcMaxBoneInfluences();

		// Log info about the chunk.
		UE_LOG(LogSkeletalMesh, Log, TEXT("Section %u: %u vertices, %u active bones"),
			SectionIndex,
			Section.GetNumVertices(),
			Section.BoneMap.Num()
		);
	}

	LODModel.IndexBuffer.Reset();

	// Finish building the sections.
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
	{
		FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
		Section.BaseIndex = LODModel.IndexBuffer.Num();
		for (int32 Index = 0; Index < Section.NumVertices; Index++)
		{
			LODModel.IndexBuffer.Add(Section.BaseIndex + Index);
		}
	}

	// Free the skinned mesh chunks which are no longer needed.
	for (int32 i = 0; i < Chunks.Num(); ++i)
	{
		delete Chunks[i];
		Chunks[i] = NULL;
	}
	Chunks.Empty();

	// Compute the required bones for this model.
	USkeletalMesh::CalculateRequiredBones(LODModel, RefSkeleton, NULL);
#endif // #if WITH_EDITORONLY_DATA
}
#endif // USE_FRACTURE_UPDATE

Nv::Blast::Mesh* CreateAuthoringMeshFromRawMesh(const FRawMesh& RawMesh, const FTransform3f& UE4ToBlastTransform)
{
	//Raw meshes are unwelded by default so weld them together and generate a real index buffer
	TArray<FStaticMeshBuildVertex> WeldedVerts;
	TArray<TArray<uint32>> PerSectionIndices;
	TArray<int32> WedgeMap;

	//Map them all to section 0
	PerSectionIndices.SetNum(1);
	TMap<uint32, uint32> MaterialToSectionMapping;
	for (int32 Face = 0; Face < RawMesh.FaceMaterialIndices.Num(); Face++)
	{
		MaterialToSectionMapping.Add(RawMesh.FaceMaterialIndices[Face], 0);
	}

	FOverlappingCorners OverlappingCorners;
	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	MeshUtilities.FindOverlappingCorners(OverlappingCorners, RawMesh.VertexPositions, RawMesh.WedgeIndices, THRESH_POINTS_ARE_SAME);
	MeshUtilities.BuildStaticMeshVertexAndIndexBuffers(WeldedVerts, PerSectionIndices, WedgeMap, RawMesh, OverlappingCorners, MaterialToSectionMapping, THRESH_POINTS_ARE_SAME, FVector3f(1.0f), EImportStaticMeshVersion::LastVersion);

	const TArray<int32>* FaceMaterialIndices = &RawMesh.FaceMaterialIndices;
	const TArray<uint32>* FaceSmoothingMasks = &RawMesh.FaceSmoothingMasks;

	TArray<int32> FilteredFaceMaterialIndices;
	TArray<uint32> FilteredFaceSmoothingMasks;

	//If the size doesn't match it removed some degenerate  triangles so we need to update our arrays
	if (PerSectionIndices[0].Num() != (FaceMaterialIndices->Num() * 3))
	{
		check((FaceMaterialIndices->Num() * 3) == WedgeMap.Num());
		FilteredFaceMaterialIndices.Reserve(FaceMaterialIndices->Num());
		FilteredFaceSmoothingMasks.Reserve(FaceSmoothingMasks->Num());

		for (int32 FaceIdx = 0; FaceIdx < FaceMaterialIndices->Num(); FaceIdx++)
		{
			int32 WedgeNewIdxs[3] = {
				WedgeMap[FaceIdx * 3 + 0],
				WedgeMap[FaceIdx * 3 + 1],
				WedgeMap[FaceIdx * 3 + 2]
			};

			if (WedgeNewIdxs[0] != INDEX_NONE && WedgeNewIdxs[1] != INDEX_NONE && WedgeNewIdxs[2] != INDEX_NONE)
			{
				//we kept this face, make sure the ordering matches
				//checkSlow(WedgeNewIdxs[0] == PerSectionIndices[0][FaceIdx * 3 + 0]
				//	&& WedgeNewIdxs[1] == PerSectionIndices[0][FaceIdx * 3 + 1]
				//	&& WedgeNewIdxs[2] == PerSectionIndices[0][FaceIdx * 3 + 2]);
				FilteredFaceMaterialIndices.Add((*FaceMaterialIndices)[FaceIdx]);
				if (FaceSmoothingMasks->IsValidIndex(FaceIdx))
				{
					FilteredFaceSmoothingMasks.Add((*FaceSmoothingMasks)[FaceIdx]);
				}
			}
			else
			{
				//This should be impossible but if the entire triangle was not removed
				checkSlow(WedgeNewIdxs[0] == INDEX_NONE
					&& WedgeNewIdxs[1] == INDEX_NONE
					&& WedgeNewIdxs[2] == INDEX_NONE);
			}
		}

		FaceMaterialIndices = &FilteredFaceMaterialIndices;
		FaceSmoothingMasks = &FilteredFaceSmoothingMasks;
	}

	TArray<FVector3f> Positions;
	TArray<FVector3f> Normals;
	TArray<FVector2f> UVs;

	Positions.SetNumUninitialized(WeldedVerts.Num());
	Normals.SetNumUninitialized(WeldedVerts.Num());
	UVs.SetNumUninitialized(WeldedVerts.Num());

	//FMeshMergeHelpers::TransformRawMeshVertexData flips it if the determinant is < 0 which we don't wnat
	for (int32 V = 0; V < WeldedVerts.Num(); V++)
	{
		const FStaticMeshBuildVertex& SMBV = WeldedVerts[V];
		Positions[V] = UE4ToBlastTransform.TransformPosition(SMBV.Position);
		Normals[V] = UE4ToBlastTransform.TransformVectorNoScale(SMBV.TangentZ);
		UVs[V] = FVector2f(SMBV.UVs[0].X, 1.0f - SMBV.UVs[0].Y);
	}

	Nv::Blast::Mesh* mesh = NvBlastExtAuthoringCreateMesh((NvcVec3*)Positions.GetData(), (NvcVec3*)Normals.GetData(), (NvcVec2*)UVs.GetData(), WeldedVerts.Num(),
		PerSectionIndices[0].GetData(), PerSectionIndices[0].Num());
	mesh->setMaterialId((int32*)FaceMaterialIndices->GetData());
	if (FaceMaterialIndices->Num() == FaceSmoothingMasks->Num())
	{
		mesh->setSmoothingGroup((int32*)FaceSmoothingMasks->GetData());
	}
	return mesh;
}

void PrepareLODData(TSharedPtr<FFractureSession> FractureSession,
	const TArray<FSkeletalMaterial>& ExistingMaterials, TMap<int32, int32>& InteriorMaterialsToSlots,
	TArray<FVector3f>& LODPoints, TArray<SkeletalMeshImportData::FMeshWedge>& LODWedges, TArray<SkeletalMeshImportData::FMeshFace>& LODFaces, TArray<SkeletalMeshImportData::FVertInfluence>& LODInfluences, TArray<int32>& LODPointToRawMap,
	int32 ChunkIndex = INDEX_NONE)
{
	USkeletalMesh* SkeletalMesh = FractureSession->BlastMesh->Mesh;
	TSharedPtr<Nv::Blast::AuthoringResult> FractureData = FractureSession->FractureData;
	check(ChunkIndex < (int32)FractureData->chunkCount);
	UFbxSkeletalMeshImportData* skelMeshImportData = Cast<UFbxSkeletalMeshImportData>(SkeletalMesh->GetAssetImportData());
	FTransform3f Converter = UBlastMeshFactory::GetTransformBlastToUE4CoordinateSystem(skelMeshImportData);

	TArray<FSkeletalMaterial>& NewMaterials = SkeletalMesh->GetMaterials();

	uint32 FirstChunk = ChunkIndex < 0 ? 0 : ChunkIndex;
	uint32 LastChunk = ChunkIndex < 0 ? FractureData->chunkCount : ChunkIndex + 1;

	uint32 TriangleCount = FractureData->geometryOffset[LastChunk] - FractureData->geometryOffset[FirstChunk];
	LODPoints.AddUninitialized(TriangleCount * 3);
	LODWedges.AddUninitialized(TriangleCount * 3);
	LODFaces.SetNum(TriangleCount);
	LODInfluences.AddUninitialized(TriangleCount * 3);
	LODPointToRawMap.AddUninitialized(TriangleCount * 3);
	uint32 VertexIndex = 0;
	uint32 FaceIndex = 0;

	for (uint32 ci = FirstChunk; ci < LastChunk; ci++)
	{
		for (uint32 fi = FractureData->geometryOffset[ci]; fi < FractureData->geometryOffset[ci + 1]; fi++, FaceIndex++)
		{
			Nv::Blast::Triangle& tr = FractureData->geometry[fi];
			//No need to pass normals, it is computed in mesh builder anyway
			for (uint32 vi = 0; vi < 3; vi++, VertexIndex++)
			{
				Nv::Blast::Vertex& v = (&tr.a)[vi];
				LODPoints[VertexIndex] = Converter.TransformPosition(FVector3f(v.p.x, v.p.y, v.p.z));
				LODPointToRawMap[VertexIndex] = VertexIndex;
				LODWedges[VertexIndex].Color = FColor::White;
				for (uint32 uvi = 0; uvi < MAX_TEXCOORDS; uvi++)
				{
					LODWedges[VertexIndex].UVs[uvi] = FVector2f(0.f, 0.f);
					if (uvi == 0)
					{
						LODWedges[VertexIndex].UVs[uvi] = FVector2f(v.uv[uvi].x, -v.uv[uvi].y + 1.f);
					}
				}
				LODWedges[VertexIndex].iVertex = VertexIndex;
				LODFaces[FaceIndex].iWedge[vi] = VertexIndex;
				LODInfluences[VertexIndex].BoneIndex = FractureSession->ChunkToBoneIndex[ci];
				LODInfluences[VertexIndex].VertIndex = VertexIndex;
				LODInfluences[VertexIndex].Weight = 1.f;
			}

			//the interior material ids don't follow after the previously existing materials, there could be a gap, while UE4 needs them tightly packed
			int32 FinalMatSlot = tr.materialId;

			if (!ExistingMaterials.IsValidIndex(tr.materialId))
			{
				const int32* MatSlot = InteriorMaterialsToSlots.Find(tr.materialId);
				if (MatSlot)
				{
					FinalMatSlot = *MatSlot;
				}
				else
				{
					// Try find material by name.
					FName matname = FName(FBlastFracture::InteriorMaterialID, tr.materialId);
					int32 rslot = -1;
					for (int32 mid = 0; mid < ExistingMaterials.Num(); ++mid)
					{
						if (ExistingMaterials[mid].ImportedMaterialSlotName == matname)
						{
							rslot = mid;
							break;
						}
					}


					if (rslot == -1)
					{
						FinalMatSlot = NewMaterials.Num();
						InteriorMaterialsToSlots.Add(tr.materialId, FinalMatSlot);
						//Update the internal representation with what our final materialID is
						FractureSession->FractureTool->replaceMaterialId(tr.materialId, FinalMatSlot);
						FSkeletalMaterial NewMat(UMaterial::GetDefaultMaterial(MD_Surface));
						NewMat.ImportedMaterialSlotName = matname;
						NewMaterials.Add(NewMat);
					}
					else
					{
						FinalMatSlot = rslot;
						InteriorMaterialsToSlots.Add(tr.materialId, FinalMatSlot);
						FractureSession->FractureTool->replaceMaterialId(tr.materialId, FinalMatSlot);
					}
				}
			}
			LODFaces[FaceIndex].MeshMaterialIndex = FinalMatSlot;
			tr.materialId = FinalMatSlot;
			//tr.smoothingGroup >= 0  is only valid if non-negative
			LODFaces[FaceIndex].SmoothingGroups = FMath::Max(tr.smoothingGroup, 0);
		}
	}

	FBox3f BoundingBox(LODPoints.GetData(), LODPoints.Num());
	BoundingBox += FBox3f(SkeletalMesh->GetImportedBounds().GetBox());
	SkeletalMesh->SetImportedBounds(FBoxSphereBounds(FBoxSphereBounds3f(BoundingBox)));
	{
		FVector3f MidMesh = 0.5f * (BoundingBox.Min + BoundingBox.Max);
		SkeletalMesh->SetNegativeBoundsExtension(FVector(1.0f * (BoundingBox.Min - MidMesh)));// -FVector(0, 0, 0.1f*(BoundingBox.Min[2] - MidMesh[2])));
		SkeletalMesh->SetPositiveBoundsExtension(FVector(1.0f * (BoundingBox.Max - MidMesh)));
	}

	SkeletalMesh->SetHasVertexColors(false);
}

void ProcessImportMeshSkeleton(USkeletalMesh* SkeletalMesh, TSharedPtr<FFractureSession> FractureSession)
{
	FTransform RootTransform(FTransform::Identity);
	if (SkeletalMesh->GetRefSkeleton().GetRefBonePose().Num())
	{
		RootTransform = SkeletalMesh->GetRefSkeleton().GetRefBonePose()[0];
	}

	SkeletalMesh->GetRefSkeleton().Empty();

	FReferenceSkeletonModifier RefSkelModifier(SkeletalMesh->GetRefSkeleton(), SkeletalMesh->GetSkeleton());

	RefSkelModifier.Add(FMeshBoneInfo(FName(TEXT("root"), FNAME_Add), TEXT("root"), INDEX_NONE), RootTransform);

	for (auto& IndexPair : FractureSession->ChunkToBoneIndex)
	{
		if (IndexPair.Key < 0)
		{
			continue;
		}
		FName BoneName = UBlastMesh::GetDefaultChunkBoneNameFromIndex(IndexPair.Key);
		const FMeshBoneInfo BoneInfo(BoneName, BoneName.ToString(), FractureSession->ChunkToBoneIndex[FractureSession->FractureData->chunkDescs[IndexPair.Key].parentChunkDescIndex]);
		RefSkelModifier.Add(BoneInfo, FTransform::Identity);
	}
}

void FinalizeMeshCreation(USkeletalMesh* SkeletalMesh)
{
	FScopedSkeletalMeshPostEditChange ScopedPostEditChange(SkeletalMesh);
	SkeletalMesh->InvalidateDeriveDataCacheGUID();

	SkeletalMesh->ResetLODInfo();
	FSkeletalMeshLODInfo& NewLODInfo = SkeletalMesh->AddLODInfo();
	NewLODInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
	NewLODInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
	NewLODInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
	NewLODInfo.LODHysteresis = 0.02f;

	SkeletalMesh->CalculateInvRefMatrices();
	if ((!SkeletalMesh->GetResourceForRendering() || !SkeletalMesh->GetResourceForRendering()->LODRenderData.IsValidIndex(0)))
	{
		SkeletalMesh->Build();
	}
	SkeletalMesh->MarkPackageDirty();

	SkeletalMesh->GetSkeleton()->RecreateBoneTree(SkeletalMesh);
}

void CreateSkeletalMeshFromAuthoring(TSharedPtr<FFractureSession> FractureSession, UStaticMesh* InSourceStaticMesh)
{
	UBlastMesh* BlastMesh = FractureSession->BlastMesh;
	BlastMesh->Mesh = nullptr;

	BlastMesh->PhysicsAsset = NewObject<UPhysicsAsset>(BlastMesh, *InSourceStaticMesh->GetName().Append(TEXT("_PhysicsAsset")), RF_NoFlags);
	if (BlastMesh->AssetImportData == nullptr)
	{
		BlastMesh->AssetImportData = NewObject<UBlastAssetImportData>(BlastMesh);
	}

	BlastMesh->Skeleton = NewObject<USkeleton>(BlastMesh, *InSourceStaticMesh->GetName().Append(TEXT("_Skeleton")));

	USkeletalMesh* SkeletalMesh = NewObject<USkeletalMesh>(BlastMesh, FName(*InSourceStaticMesh->GetName().Append(TEXT("_SkelMesh"))), RF_Public);
	SkeletalMesh->SetSkeleton(BlastMesh->Skeleton);
	BlastMesh->Mesh = SkeletalMesh;

	SkeletalMesh->PreEditChange(NULL);

	TArray<FSkeletalMaterial> ExistingMaterials;
	TMap<int32, int32> InteriorMaterialsToSlots;

	for (auto& mat : InSourceStaticMesh->GetStaticMaterials())
	{
		FSkeletalMaterial NewMat(mat.MaterialInterface);
		NewMat.MaterialSlotName = mat.MaterialSlotName;
		NewMat.ImportedMaterialSlotName = mat.ImportedMaterialSlotName;
		ExistingMaterials.Add(NewMat);
		SkeletalMesh->GetMaterials().Add(NewMat);
	}

	TArray<FVector3f> LODPoints;
	TArray<SkeletalMeshImportData::FMeshWedge> LODWedges;
	TArray<SkeletalMeshImportData::FMeshFace> LODFaces;
	TArray<SkeletalMeshImportData::FVertInfluence> LODInfluences;
	TArray<int32> LODPointToRawMap;

	PrepareLODData(FractureSession, ExistingMaterials, InteriorMaterialsToSlots, LODPoints, LODWedges, LODFaces, LODInfluences, LODPointToRawMap);

	ProcessImportMeshSkeleton(SkeletalMesh, FractureSession);


	FSkeletalMeshModel& ImportedResource = *SkeletalMesh->GetImportedModel();
	ImportedResource.LODModels.Empty();
	ImportedResource.LODModels.Add(new FSkeletalMeshLODModel());

	FSkeletalMeshLODModel& LODModel = ImportedResource.LODModels[0];
	LODModel.NumTexCoords = 1;// FMath::Max<uint32>(1, skelMeshImportData->NumTexCoords);

	IMeshUtilities::MeshBuildOptions BuildOptions;
	BuildOptions.bComputeNormals = true;
	BuildOptions.bComputeTangents = true;
	BuildOptions.bUseMikkTSpace = true;
	BuildOptions.bComputeWeightedNormals = true;
	BuildOptions.bRemoveDegenerateTriangles = false;

	TArray<FText> WarningMessages;
	TArray<FName> WarningNames;

	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	const bool bBuildSuccess = MeshUtilities.BuildSkeletalMesh(LODModel, SkeletalMesh->GetName(), SkeletalMesh->GetRefSkeleton(), LODInfluences, LODWedges, LODFaces, LODPoints, LODPointToRawMap, BuildOptions, &WarningMessages, &WarningNames);

	if (!WarningMessages.IsEmpty() && WarningNames.Num() == WarningMessages.Num())
	{
		for (int32 MessageIdx = 0; MessageIdx < WarningMessages.Num(); ++MessageIdx)
		{
			GLog->Log(LogSkeletalMesh.GetCategoryName(), bBuildSuccess ? ELogVerbosity::Warning : ELogVerbosity::Error, FString::Format(TEXT("[BLAST MESH] [{0}] {1}"), { WarningNames[MessageIdx].ToString(), WarningMessages[MessageIdx].ToString() }));
		}
	}

	if (!bBuildSuccess)
	{
		SkeletalMesh->MarkAsGarbage();
		return;
	}

	FinalizeMeshCreation(SkeletalMesh);
}


void CreateSkeletalMeshFromAuthoring(TSharedPtr<FFractureSession> FractureSession, bool isFinal, UMaterialInterface* InteriorMaterial)
{
	TSharedPtr<Nv::Blast::AuthoringResult> FractureData = FractureSession->FractureData;
	USkeletalMesh* SkeletalMesh = FractureSession->BlastMesh->Mesh;
	check(SkeletalMesh);

	TArray<FVector3f> LODPoints;
	TArray<SkeletalMeshImportData::FMeshWedge> LODWedges;
	TArray<SkeletalMeshImportData::FMeshFace> LODFaces;
	TArray<SkeletalMeshImportData::FVertInfluence> LODInfluences;
	TArray<int32> LODPointToRawMap;

	SkeletalMesh->PreEditChange(NULL);
	TArray<FSkeletalMaterial> ExistingMaterials = SkeletalMesh->GetMaterials();
	TMap<int32, int32> InteriorMaterialsToSlots;

	PrepareLODData(FractureSession, ExistingMaterials, InteriorMaterialsToSlots, LODPoints, LODWedges, LODFaces, LODInfluences, LODPointToRawMap);

	//New slots must be interior materials, they couldn't have come from anywhere else
	for (int32 NewSlot = ExistingMaterials.Num(); NewSlot < SkeletalMesh->GetMaterials().Num(); NewSlot++)
	{
		FSkeletalMaterial& MatSlot = SkeletalMesh->GetMaterials()[NewSlot];
		MatSlot.MaterialInterface = InteriorMaterial;
		if (MatSlot.MaterialInterface)
		{
			MatSlot.MaterialInterface->CheckMaterialUsage(MATUSAGE_SkeletalMesh);
		}
	}

	ProcessImportMeshSkeleton(SkeletalMesh, FractureSession);

	FSkeletalMeshModel& ImportedResource = *SkeletalMesh->GetImportedModel();
	ImportedResource.LODModels.Empty();
	ImportedResource.LODModels.Add(new FSkeletalMeshLODModel());

	FSkeletalMeshLODModel& LODModel = ImportedResource.LODModels[0];
	LODModel.NumTexCoords = 1;// FMath::Max<uint32>(1, skelMeshImportData->NumTexCoords);

	IMeshUtilities::MeshBuildOptions BuildOptions;
	BuildOptions.bComputeNormals = true;
	BuildOptions.bComputeTangents = true;
	BuildOptions.bUseMikkTSpace = true;
	BuildOptions.bComputeWeightedNormals = true;
	BuildOptions.bRemoveDegenerateTriangles = false;

	TArray<FText> WarningMessages;
	TArray<FName> WarningNames;

	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	const bool bBuildSuccess = MeshUtilities.BuildSkeletalMesh(LODModel, SkeletalMesh->GetName(), SkeletalMesh->GetRefSkeleton(), LODInfluences, LODWedges, LODFaces, LODPoints, LODPointToRawMap, BuildOptions, &WarningMessages, &WarningNames);

	if (!WarningMessages.IsEmpty() && WarningNames.Num() == WarningMessages.Num())
	{
		for (int32 MessageIdx = 0; MessageIdx < WarningMessages.Num(); ++MessageIdx)
		{
			GLog->Log(LogSkeletalMesh.GetCategoryName(), bBuildSuccess ? ELogVerbosity::Warning : ELogVerbosity::Error, FString::Format(TEXT("[BLAST MESH EDIT] [{0}] {1}"), { WarningNames[MessageIdx].ToString(), WarningMessages[MessageIdx].ToString() }));
		}
	}

	if (!bBuildSuccess)
	{
		SkeletalMesh->MarkAsGarbage();
		return;
	}

	FinalizeMeshCreation(SkeletalMesh);

	FractureSession->IsMeshCreatedFromFractureData = true;
}

void LoadFracturedChunk(TSharedPtr<FFractureSession> FractureSession, UMaterialInterface* InteriorMaterial,
	const TArray<FSkeletalMaterial>& ExistingMaterials, TMap<int32, int32>& InteriorMaterialsToSlots,
	TArray <FSkinnedMeshChunk*>& Chunks, TArray<int32>& OutLODPointToRawMap, int32 ChunkIndex, int32 MaxBonesPerChunk)
{
	USkeletalMesh* SkeletalMesh = FractureSession->BlastMesh->Mesh;
	check(SkeletalMesh);

	TArray<FVector3f> LODPoints;
	TArray<SkeletalMeshImportData::FMeshWedge> LODWedges;
	TArray<SkeletalMeshImportData::FMeshFace> LODFaces;
	TArray<SkeletalMeshImportData::FVertInfluence> LODInfluences;
	TArray<int32> LODPointToRawMap;

	TArray<uint32> TangentsIndices;
	TArray<FVector2f> TangentsUVs;
	TArray<uint32> TangentsSmothingGroups;
	TArray<FVector3f> TangentsX, TangentsY, TangentsZ;

	uint32 VertexIndex = 0;

	PrepareLODData(FractureSession, ExistingMaterials, InteriorMaterialsToSlots, LODPoints, LODWedges, LODFaces, LODInfluences, LODPointToRawMap, ChunkIndex);

	//New slots must be interior materials, they couldn't have come from anywhere else
	for (int32 NewSlot = ExistingMaterials.Num(); NewSlot < SkeletalMesh->GetMaterials().Num(); NewSlot++)
	{
		FSkeletalMaterial& MatSlot = SkeletalMesh->GetMaterials()[NewSlot];
		MatSlot.MaterialInterface = InteriorMaterial;
		if (MatSlot.MaterialInterface)
		{
			MatSlot.MaterialInterface->CheckMaterialUsage(MATUSAGE_SkeletalMesh);
		}
	}

	for (uint32 fi = FractureSession->FractureData->geometryOffset[ChunkIndex]; fi < FractureSession->FractureData->geometryOffset[ChunkIndex + 1]; fi++)
	{
		const Nv::Blast::Triangle& tr = FractureSession->FractureData->geometry[fi];
		for (uint32 vi = 0; vi < 3; vi++, VertexIndex++)
		{
			auto& v = (&tr.a)[vi];
			for (uint32 uvi = 0; uvi < MAX_TEXCOORDS; uvi++)
			{
				if (uvi == 0)
				{
					TangentsUVs.Add(FVector2f(v.uv[uvi].x, -v.uv[uvi].y + 1.f));
				}
			}
			TangentsIndices.Add(VertexIndex);
			TangentsSmothingGroups.Add(tr.smoothingGroup >= 0 ? tr.smoothingGroup : 0);
		}
	}

	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	MeshUtilities.CalculateTangents(LODPoints, TangentsIndices, TangentsUVs, TangentsSmothingGroups,
		ETangentOptions::BlendOverlappingNormals | ETangentOptions::UseMikkTSpace,
		TangentsX, TangentsY, TangentsZ);

	for (int32 FaceIndex = 0; FaceIndex < LODFaces.Num(); FaceIndex++)
	{
		const SkeletalMeshImportData::FMeshFace& Face = LODFaces[FaceIndex];
		//TMP clamp to single material

		// Find a chunk which matches this triangle.
		FSkinnedMeshChunk* Chunk = NULL;
		for (int32 i = 0; i < Chunks.Num(); ++i)
		{
			if (Chunks[i]->MaterialIndex == Face.MeshMaterialIndex && Chunks[i]->BoneMap.Num() < MaxBonesPerChunk)
			{
				Chunk = Chunks[i];
				break;
			}
		}
		if (Chunk == NULL)
		{
			Chunk = new FSkinnedMeshChunk();
			Chunk->MaterialIndex = Face.MeshMaterialIndex;
			Chunk->OriginalSectionIndex = Chunks.Num();
			Chunks.Add(Chunk);
		}
		for (int32 VI = 0; VI < 3; ++VI)
		{
			int32 WedgeIndex = FaceIndex * 3 + VI;
			const SkeletalMeshImportData::FMeshWedge& Wedge = LODWedges[WedgeIndex];
			FSoftSkinBuildVertex Vertex;
			Vertex.Position = LODPoints[WedgeIndex];
			Vertex.TangentX = TangentsX[WedgeIndex];
			Vertex.TangentY = TangentsY[WedgeIndex];
			Vertex.TangentZ = TangentsZ[WedgeIndex];
			Vertex.Color = Wedge.Color;
			FMemory::Memcpy(Vertex.UVs, Wedge.UVs, sizeof(FVector2f) * MAX_TEXCOORDS);

			Chunk->BoneMap.AddUnique(LODInfluences[WedgeIndex].BoneIndex);

			Vertex.InfluenceBones[0] = Chunk->BoneMap.Find(LODInfluences[WedgeIndex].BoneIndex);;
			Vertex.InfluenceWeights[0] = 255;//(uint8)(LODInfluences[WedgeIndex].Weight * 255.0f);;
			for (uint32 i = 1; i < MAX_TOTAL_INFLUENCES; i++)
			{
				Vertex.InfluenceBones[i] = 0;
				Vertex.InfluenceWeights[i] = 0;
			}

			Vertex.PointWedgeIdx = Wedge.iVertex;

			int32 FinalVertIndex = Chunk->Vertices.Add(Vertex);
			Chunk->Indices.Add(FinalVertIndex);
			OutLODPointToRawMap.Add(OutLODPointToRawMap.Num());
		}
	}
}

void UpdateSkeletalMeshFromAuthoring(TSharedPtr<FFractureSession> FractureSession, UMaterialInterface* InteriorMaterial)
{
#if !USE_FRACTURE_UPDATE
	CreateSkeletalMeshFromAuthoring(FractureSession, false, InteriorMaterial);
#else
	TMap<int32, int32> OldToNewBoneMap;
	TArray<int32> NewChunks;

	TMap<uint32, uint32> FractureToAssetMap;
	for (uint32 i = 0; i < FractureSession->FractureData->chunkCount; i++)
	{
		int32 val = FractureSession->FractureData->assetToFractureChunkIdMap[i];
		FractureToAssetMap.Add(val, i);
		if (FractureSession->FractureIdMap.Find(val) == INDEX_NONE)
		{
			NewChunks.Add(i);
		}
	}
	for (int32 i = 0; i < FractureSession->FractureIdMap.Num(); i++)
	{
		uint32* val = FractureToAssetMap.Find(FractureSession->FractureIdMap[i]);
		OldToNewBoneMap.Add(FractureSession->ChunkToBoneIndexPrev[i], val == nullptr ? INDEX_NONE : FractureSession->ChunkToBoneIndex[*val]);
	}

	if (!FractureSession->IsRootFractured || !FractureSession->IsMeshCreatedFromFractureData || NewChunks.Num() >= (int32)FractureSession->FractureData->chunkCount - 1)
	{
		CreateSkeletalMeshFromAuthoring(FractureSession, false, InteriorMaterial);
		return;
	}

	UBlastMesh* BlastMesh = FractureSession->BlastMesh;
	TSharedPtr<Nv::Blast::AuthoringResult> FractureData = FractureSession->FractureData;

	USkeletalMesh* SkeletalMesh = BlastMesh->Mesh;
	check(SkeletalMesh);

	SkeletalMesh->PreEditChange(NULL);
	TArray<FSkeletalMaterial> ExistingMaterials = SkeletalMesh->Materials;
	TMap<int32, int32> InteriorMaterialsToSlots;

	ProcessImportMeshSkeleton(SkeletalMesh, FractureSession);

	FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();

	FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[0];
	LODRenderData.ReleaseResources();
	//LODModel.NumTexCoords = 1;// FMath::Max<uint32>(1, skelMeshImportData->NumTexCoords);

	TIndirectArray<FComponentReregisterContext> ComponentContexts;

	// Chunk vertices to satisfy the requested limit.
	const uint32 MaxGPUSkinBones = FGPUBaseSkinVertexFactory::GetMaxGPUSkinBones();
	check(MaxGPUSkinBones <= FGPUBaseSkinVertexFactory::GHardwareMaxGPUSkinBones);
	//ChunkSkinnedVertices(Chunks, MaxGPUSkinBones);

	// Build the skeletal model from Chunks.

	TArray <FSkinnedMeshChunk*> Chunks;
	TArray<int32> LODPointToRawMap;

	for (int32 i = 0; i < NewChunks.Num(); i++)
	{
		LoadFracturedChunk(FractureSession, InteriorMaterial, ExistingMaterials, InteriorMaterialsToSlots, Chunks, LODPointToRawMap, NewChunks[i], MaxGPUSkinBones);
	}

	FSkeletalMeshLODModel& LODModel = SkeletalMesh->GetImportedModel()->LODModels[0];
	BuildSkeletalModelFromChunks(LODModel, SkeletalMesh->RefSkeleton, MaxGPUSkinBones, Chunks, LODPointToRawMap, OldToNewBoneMap);

	FinallizeMeshCreation(SkeletalMesh, LODModel, ComponentContexts);
#endif
}


#undef LOCTEXT_NAMESPACE
