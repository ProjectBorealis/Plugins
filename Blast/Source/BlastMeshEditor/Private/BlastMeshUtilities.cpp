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
#include "NvBlastExtAuthoringMeshCleaner.h"

#include "Modules/ModuleManager.h"
#include "RawIndexBuffer.h"
#include "Materials/Material.h"
#include "SkeletalMeshTypes.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include "Animation/Skeleton.h"
#include "MeshUtilities.h"
#include "MaterialDomain.h"
#include "MeshDescription.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Runtime/Core/Public/Misc/FeedbackContext.h"
#include "RawMesh.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshModel.h"
#include "OverlappingCorners.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshOperations.h"
#include "Engine/SkinnedAssetCommon.h"

#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Misc/CoreMisc.h"

#define LOCTEXT_NAMESPACE "BlastMeshEditor"

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
					IsHardEdge |= !FVector3f::PointsAreNear(RawMesh.WedgeTangentZ[3 * i + Matches[k].Key],
					                                        RawMesh.WedgeTangentZ[3 * j + Matches[k].Value], 1e-3);
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
	MeshUtilities.FindOverlappingCorners(OverlappingCorners, RawMesh.VertexPositions, RawMesh.WedgeIndices,
	                                     THRESH_POINTS_ARE_SAME);
	MeshUtilities.BuildStaticMeshVertexAndIndexBuffers(WeldedVerts, PerSectionIndices, WedgeMap, RawMesh,
	                                                   OverlappingCorners, MaterialToSectionMapping,
	                                                   THRESH_POINTS_ARE_SAME, FVector3f(1.0f),
	                                                   EImportStaticMeshVersion::LastVersion);

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

	Nv::Blast::Mesh* mesh = NvBlastExtAuthoringCreateMesh((NvcVec3*)Positions.GetData(), (NvcVec3*)Normals.GetData(),
	                                                      (NvcVec2*)UVs.GetData(), WeldedVerts.Num(),
	                                                      PerSectionIndices[0].GetData(), PerSectionIndices[0].Num());
	mesh->setMaterialId(FaceMaterialIndices->GetData());
	if (FaceMaterialIndices->Num() == FaceSmoothingMasks->Num())
	{
		mesh->setSmoothingGroup((int32*)FaceSmoothingMasks->GetData());
	}

	Nv::Blast::MeshCleaner* cleaner = NvBlastExtAuthoringCreateMeshCleaner();
	Nv::Blast::Mesh* cleanedMesh = cleaner->cleanMesh(mesh);
	cleaner->release();
	if (cleanedMesh)
	{
		mesh->release();
		return cleanedMesh;
	}

	return mesh;
}

Nv::Blast::Mesh* CreateAuthoringMeshFromMeshDescription(const FMeshDescription& SourceMeshDescription,
                                                        const TMap<FName, int32>& MaterialMap,
                                                        const FTransform3f& UE4ToBlastTransform)
{
	TArray<Nv::Blast::Vertex> Verts;
	TArray<Nv::Blast::Edge> Edges;
	TArray<Nv::Blast::Facet> Facets;

	//Gather all array data
	FStaticMeshConstAttributes Attributes(SourceMeshDescription);
	TVertexAttributesConstRef<FVector3f> VertexPositions = Attributes.GetVertexPositions();
	TVertexInstanceAttributesConstRef<FVector3f> VertexInstanceNormals = Attributes.GetVertexInstanceNormals();
	TVertexInstanceAttributesConstRef<FVector2f> VertexInstanceUVs = Attributes.GetVertexInstanceUVs();
	TPolygonGroupAttributesConstRef<FName> PolygonGroupMaterialSlotName = Attributes.GetPolygonGroupMaterialSlotNames();

	Verts.AddZeroed(SourceMeshDescription.Vertices().Num());
	TArray<int32> RemapVerts;
	RemapVerts.AddZeroed(SourceMeshDescription.Vertices().GetArraySize());
	int32 VertexIndex = 0;
	for (const FVertexID VertexID : SourceMeshDescription.Vertices().GetElementIDs())
	{
		Verts[VertexIndex].p = ToNvVector(UE4ToBlastTransform.TransformPosition(VertexPositions[VertexID]));
		const int32 NumChannels = FMath::Min(1, VertexInstanceUVs.GetNumChannels());
		for (int32 UVChannel = 0; UVChannel < NumChannels; UVChannel++)
		{
			Verts[VertexIndex].uv[UVChannel] = ToNvVector(VertexInstanceUVs.Get(VertexID, UVChannel));
			Verts[VertexIndex].uv[UVChannel].y = 1.f - Verts[VertexIndex].uv[UVChannel].y; // blast thing
		}
		Verts[VertexIndex].p = ToNvVector(VertexPositions[VertexID]);
		Verts[VertexIndex].n = ToNvVector(UE4ToBlastTransform.TransformVectorNoScale(VertexInstanceNormals[VertexID]));
		RemapVerts[VertexID.GetValue()] = VertexIndex;
		VertexIndex++;
	}

	const int32 TriangleNumber = SourceMeshDescription.Triangles().Num();
	Facets.AddZeroed(TriangleNumber);
	Edges.Reserve(Facets.Num() * 3);

	TArray<uint32> FaceSmoothingMasks;
	FaceSmoothingMasks.AddZeroed(TriangleNumber);
	//Convert the smoothgroup
	FStaticMeshOperations::ConvertHardEdgesToSmoothGroup(SourceMeshDescription, FaceSmoothingMasks);

	int32 TriangleIdx = 0;
	for (const FTriangleID TriID : SourceMeshDescription.Triangles().GetElementIDs())
	{
		Facets[TriangleIdx].firstEdgeNumber = Edges.Num();
		Facets[TriangleIdx].smoothingGroup = FaceSmoothingMasks[TriangleIdx];

		const FPolygonGroupID PolygonGroupID = SourceMeshDescription.GetTrianglePolygonGroup(TriID);
		const int32* MaterialForFace = MaterialMap.Find(PolygonGroupMaterialSlotName[PolygonGroupID]);
		Facets[TriangleIdx].materialId = MaterialForFace ? *MaterialForFace : PolygonGroupID.GetValue();

		const auto EdgesOfTriangle = SourceMeshDescription.GetTriangleEdges(TriID);
		Facets[TriangleIdx].edgesCount = EdgesOfTriangle.Num();

		const auto VertsOfTriangle = SourceMeshDescription.GetTriangleVertices(TriID);
		check(VertsOfTriangle.Num() == 3);

		const FVector3f DeducedNormal = FVector3f::CrossProduct(
			VertexPositions[VertsOfTriangle[1]] - VertexPositions[VertsOfTriangle[0]],
			VertexPositions[VertsOfTriangle[2]] - VertexPositions[VertsOfTriangle[0]]);

		const FVector3f AverageNormal =
			VertexInstanceNormals[VertsOfTriangle[0]] + VertexInstanceNormals[VertsOfTriangle[1]] +
			VertexInstanceNormals[VertsOfTriangle[2]];
		if (FVector3f::DotProduct(AverageNormal, DeducedNormal) < 0.f)
		{
			Swap(VertsOfTriangle[0], VertsOfTriangle[1]);
		}

		Edges.Add({
			(uint32)RemapVerts[VertsOfTriangle[0]],
			(uint32)RemapVerts[VertsOfTriangle[1]]
		});
		Edges.Add({
			(uint32)RemapVerts[VertsOfTriangle[1]],
			(uint32)RemapVerts[VertsOfTriangle[2]]
		});
		Edges.Add({
			(uint32)RemapVerts[VertsOfTriangle[2]],
			(uint32)RemapVerts[VertsOfTriangle[0]]
		});

		TriangleIdx++;
	}

	Nv::Blast::Mesh* mesh = NvBlastExtAuthoringCreateMeshFromFacets(Verts.GetData(), Edges.GetData(),
	                                                                Facets.GetData(), Verts.Num(),
	                                                                Edges.Num(), Facets.Num());

	/*Nv::Blast::MeshCleaner* cleaner = NvBlastExtAuthoringCreateMeshCleaner();
	Nv::Blast::Mesh* cleanedMesh = cleaner->cleanMesh(mesh);
	cleaner->release();
	if (cleanedMesh)
	{
		mesh->release();
		return cleanedMesh;
	}*/

	return mesh;
}

Nv::Blast::Mesh* CreateAuthoringMeshFromRenderData(const FStaticMeshRenderData& RenderData,
                                                   const TMap<FName, int32>& MaterialMap,
                                                   const FTransform3f& UE4ToBlastTransform)
{
	if (RenderData.LODResources.IsEmpty())
	{
		return nullptr;
	}

	TArray<Nv::Blast::Vertex> Verts;
	Verts.AddUninitialized(RenderData.LODResources[0].GetNumVertices());

	for (int32 VertIdx = 0; VertIdx < Verts.Num(); VertIdx++)
	{
		Verts[VertIdx].p = ToNvVector(UE4ToBlastTransform.TransformPosition(
			RenderData.LODResources[0].VertexBuffers.PositionVertexBuffer.VertexPosition(VertIdx)));
		Verts[VertIdx].n = ToNvVector(UE4ToBlastTransform.TransformVectorNoScale(
			RenderData.LODResources[0].VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertIdx)));
		Verts[VertIdx].uv[0] = ToNvVector(
			RenderData.LODResources[0].VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertIdx, 0));
		Verts[VertIdx].uv[0].y = 1.f - Verts[VertIdx].uv[0].y; // blast thing
	}

	TArray<uint32> Indices;
	RenderData.LODResources[0].IndexBuffer.GetCopy(Indices);

	Nv::Blast::Mesh* mesh = NvBlastExtAuthoringCreateMeshOnlyTriangles(Verts.GetData(), Verts.Num(),
	                                                                   Indices.GetData(), Indices.Num(),
	                                                                   nullptr, 0);

	Nv::Blast::MeshCleaner* cleaner = NvBlastExtAuthoringCreateMeshCleaner();
	Nv::Blast::Mesh* cleanedMesh = cleaner->cleanMesh(mesh);
	cleaner->release();
	if (cleanedMesh)
	{
		mesh->release();
		return cleanedMesh;
	}

	return mesh;
}

void PrepareLODData(TSharedPtr<FFractureSession> FractureSession,
                    const TArray<FSkeletalMaterial>& ExistingMaterials, TMap<int32, int32>& InteriorMaterialsToSlots,
                    TArray<FVector3f>& LODPoints, TArray<SkeletalMeshImportData::FMeshWedge>& LODWedges,
                    TArray<SkeletalMeshImportData::FMeshFace>& LODFaces,
                    TArray<SkeletalMeshImportData::FVertInfluence>& LODInfluences, TArray<int32>& LODPointToRawMap,
                    int32 ChunkIndex = INDEX_NONE)
{
	USkeletalMesh* SkeletalMesh = FractureSession->BlastMesh->Mesh;
	TSharedPtr<Nv::Blast::AuthoringResult> FractureData = FractureSession->FractureData;
	check(ChunkIndex < (int32)FractureData->chunkCount);
	UFbxSkeletalMeshImportData* skelMeshImportData = Cast<UFbxSkeletalMeshImportData>(
		SkeletalMesh->GetAssetImportData());
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
		SkeletalMesh->SetNegativeBoundsExtension(FVector(1.0f * (BoundingBox.Min - MidMesh)));
		// -FVector(0, 0, 0.1f*(BoundingBox.Min[2] - MidMesh[2])));
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

	for (const auto& IndexPair : FractureSession->ChunkToBoneIndex)
	{
		if (IndexPair.Key < 0)
		{
			continue;
		}

		const int32 chunkID = FractureSession->FractureData->assetToFractureChunkIdMap[IndexPair.Key];
		const int32 chunkInfoIdx = FractureSession->FractureTool->getChunkInfoIndex(chunkID);

		const Nv::Blast::ChunkInfo& ChunkInfo = FractureSession->FractureTool->getChunkInfo(chunkInfoIdx);
		int32 ParentBoneIdx = 0;
		for (uint32 Idx = 0; Idx < FractureSession->FractureData->chunkCount; Idx++)
		{
			if (ChunkInfo.parentChunkId == FractureSession->FractureData->assetToFractureChunkIdMap[Idx])
			{
				ParentBoneIdx = FractureSession->ChunkToBoneIndex[Idx];
				break;
			}
		}

		const FName BoneName = UBlastMesh::GetDefaultChunkBoneNameFromIndex(IndexPair.Key);
		const FMeshBoneInfo BoneInfo(BoneName, BoneName.ToString(), ParentBoneIdx);
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
	if ((!SkeletalMesh->GetResourceForRendering() || !SkeletalMesh->GetResourceForRendering()->LODRenderData.
	                                                                IsValidIndex(0)))
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

	BlastMesh->PhysicsAsset = NewObject<UPhysicsAsset>(
		BlastMesh, *InSourceStaticMesh->GetName().Append(TEXT("_PhysicsAsset")), RF_NoFlags);
	if (BlastMesh->AssetImportData == nullptr)
	{
		BlastMesh->AssetImportData = NewObject<UBlastAssetImportData>(BlastMesh);
	}

	BlastMesh->Skeleton = NewObject<USkeleton>(BlastMesh, *InSourceStaticMesh->GetName().Append(TEXT("_Skeleton")));

	USkeletalMesh* SkeletalMesh = NewObject<USkeletalMesh>(
		BlastMesh, FName(*InSourceStaticMesh->GetName().Append(TEXT("_SkelMesh"))), RF_Public);
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

	PrepareLODData(FractureSession, ExistingMaterials, InteriorMaterialsToSlots, LODPoints, LODWedges, LODFaces,
	               LODInfluences, LODPointToRawMap);

	ProcessImportMeshSkeleton(SkeletalMesh, FractureSession);


	FSkeletalMeshModel& ImportedResource = *SkeletalMesh->GetImportedModel();
	ImportedResource.LODModels.Empty();
	ImportedResource.LODModels.Add(new FSkeletalMeshLODModel());

	FSkeletalMeshLODModel& LODModel = ImportedResource.LODModels[0];
	LODModel.NumTexCoords = 1; // FMath::Max<uint32>(1, skelMeshImportData->NumTexCoords);

	IMeshUtilities::MeshBuildOptions BuildOptions;
	BuildOptions.bComputeNormals = true;
	BuildOptions.bComputeTangents = true;
	BuildOptions.bUseMikkTSpace = true;
	BuildOptions.bComputeWeightedNormals = true;
	BuildOptions.bRemoveDegenerateTriangles = false;

	TArray<FText> WarningMessages;
	TArray<FName> WarningNames;

	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	const bool bBuildSuccess = MeshUtilities.BuildSkeletalMesh(LODModel, SkeletalMesh->GetName(),
	                                                           SkeletalMesh->GetRefSkeleton(), LODInfluences, LODWedges,
	                                                           LODFaces, LODPoints, LODPointToRawMap, BuildOptions,
	                                                           &WarningMessages, &WarningNames);

	if (!WarningMessages.IsEmpty() && WarningNames.Num() == WarningMessages.Num())
	{
		for (int32 MessageIdx = 0; MessageIdx < WarningMessages.Num(); ++MessageIdx)
		{
			GLog->Log(LogSkeletalMesh.GetCategoryName(), bBuildSuccess ? ELogVerbosity::Warning : ELogVerbosity::Error,
			          FString::Format(TEXT("[BLAST MESH] [{0}] {1}"), {
				                          WarningNames[MessageIdx].ToString(), WarningMessages[MessageIdx].ToString()
			                          }));
		}
	}

	if (!bBuildSuccess)
	{
		SkeletalMesh->MarkAsGarbage();
		return;
	}

	FinalizeMeshCreation(SkeletalMesh);
}


void CreateSkeletalMeshFromAuthoring(TSharedPtr<FFractureSession> FractureSession, bool isFinal,
                                     UMaterialInterface* InteriorMaterial)
{
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

	PrepareLODData(FractureSession, ExistingMaterials, InteriorMaterialsToSlots, LODPoints, LODWedges, LODFaces,
	               LODInfluences, LODPointToRawMap);

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
	LODModel.NumTexCoords = 1; // FMath::Max<uint32>(1, skelMeshImportData->NumTexCoords);

	IMeshUtilities::MeshBuildOptions BuildOptions;
	BuildOptions.bComputeNormals = true;
	BuildOptions.bComputeTangents = true;
	BuildOptions.bUseMikkTSpace = true;
	BuildOptions.bComputeWeightedNormals = true;
	BuildOptions.bRemoveDegenerateTriangles = false;

	TArray<FText> WarningMessages;
	TArray<FName> WarningNames;

	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	const bool bBuildSuccess = MeshUtilities.BuildSkeletalMesh(LODModel, SkeletalMesh->GetName(),
	                                                           SkeletalMesh->GetRefSkeleton(), LODInfluences, LODWedges,
	                                                           LODFaces, LODPoints, LODPointToRawMap, BuildOptions,
	                                                           &WarningMessages, &WarningNames);

	if (!WarningMessages.IsEmpty() && WarningNames.Num() == WarningMessages.Num())
	{
		for (int32 MessageIdx = 0; MessageIdx < WarningMessages.Num(); ++MessageIdx)
		{
			GLog->Log(LogSkeletalMesh.GetCategoryName(), bBuildSuccess ? ELogVerbosity::Warning : ELogVerbosity::Error,
			          FString::Format(TEXT("[BLAST MESH EDIT] [{0}] {1}"), {
				                          WarningNames[MessageIdx].ToString(), WarningMessages[MessageIdx].ToString()
			                          }));
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

void UpdateSkeletalMeshFromAuthoring(TSharedPtr<FFractureSession> FractureSession, UMaterialInterface* InteriorMaterial)
{
	CreateSkeletalMeshFromAuthoring(FractureSession, false, InteriorMaterial);
}

#undef LOCTEXT_NAMESPACE
