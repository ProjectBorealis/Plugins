/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagSceneObjectMesh.h"
#include "ToolbagImporterPrivatePCH.h"
#include "RawMesh.h"
#include "UI/ToolbagImportUI.h"
#include <string>
#include <limits>
#include <Developer/AssetTools/Public/AssetToolsModule.h>
#include <PackageTools.h>
#include <ObjectTools.h>
#include "ToolbagUtils.h"
#include "ToolbagUtils/Structs.h"

UStaticMesh* ToolbagSceneObjectMesh::ReadMesh( ToolbagUtils::MeshSceneObject* ToolbagMeshSceneObject, FString Name, UToolbagImportUI* ImportUI, TMap<FString, UMaterialInterface*>& Materials )
{
	ToolbagUtils::Mesh* ToolbagMesh = &ToolbagMeshSceneObject->mesh;
	//desc
	FString MeshName = Name;
	MeshName = ObjectTools::SanitizeObjectName(MeshName);

	UPackage* MeshPackage = ToolbagUtils::CreatePackageForAsset<UStaticMesh>( ImportUI->AbsoluteMeshesPath, MeshName );
	UStaticMesh* m = FindObject<UStaticMesh>(MeshPackage, *MeshName);
	if(m == NULL)
	{
		m = NewObject<UStaticMesh>( MeshPackage, UStaticMesh::StaticClass(), FName( *MeshName ), RF_Standalone | RF_Public );
		FAssetRegistryModule::AssetCreated( m );
	}
	// clear out any old data
	m->SourceModels.Empty();
	m->SectionInfoMap.Clear();

	FStaticMeshSourceModel* sourceModel = &m->AddSourceModel(); //new FStaticMeshSourceModel();
	sourceModel->BuildSettings.bRecomputeNormals = false;
	sourceModel->BuildSettings.bRecomputeTangents = false;
	sourceModel->BuildSettings.bUseMikkTSpace = false;
	sourceModel->BuildSettings.bBuildAdjacencyBuffer = false;
	
	FRawMesh rawMesh;
	sourceModel->RawMeshBulkData->LoadRawMesh( rawMesh );
	rawMesh.VertexPositions.Empty();
	rawMesh.VertexPositions.AddZeroed( ToolbagMesh->vertexCount );
	TArray<FVector>	WedgeTangentX;
	WedgeTangentX.AddZeroed( ToolbagMesh->vertexCount );
	rawMesh.WedgeTangentX.Empty();
	rawMesh.WedgeTangentX.AddZeroed( ToolbagMesh->indexCount );
	TArray<FVector>	WedgeTangentY;
	WedgeTangentY.AddZeroed( ToolbagMesh->vertexCount );
	rawMesh.WedgeTangentY.Empty();
	rawMesh.WedgeTangentY.AddZeroed( ToolbagMesh->indexCount );
	TArray<FVector>	WedgeTangentZ;
	WedgeTangentZ.AddZeroed( ToolbagMesh->vertexCount );
	rawMesh.WedgeTangentZ.Empty();
	rawMesh.WedgeTangentZ.AddZeroed( ToolbagMesh->indexCount );
	TArray<FVector2D> Texcoords0;
	Texcoords0.AddZeroed( ToolbagMesh->vertexCount );
	rawMesh.WedgeTexCoords[0].Empty();
	rawMesh.WedgeTexCoords[0].AddZeroed( ToolbagMesh->indexCount );
	TArray<FVector2D> Texcoords1;
	Texcoords1.AddZeroed( ToolbagMesh->vertexCount );
	rawMesh.WedgeTexCoords[1].Empty();
	rawMesh.WedgeTexCoords[1].AddZeroed( ToolbagMesh->indexCount );
	TArray<FColor> Colors;
	Colors.AddZeroed( ToolbagMesh->vertexCount );
	rawMesh.WedgeColors.Empty();
	rawMesh.WedgeColors.AddZeroed( ToolbagMesh->indexCount );
	rawMesh.WedgeIndices.Empty();
	rawMesh.WedgeIndices.AddZeroed( ToolbagMesh->indexCount );
	rawMesh.FaceMaterialIndices.Empty();
	rawMesh.FaceMaterialIndices.AddZeroed( ToolbagMesh->indexCount/3 );
	rawMesh.FaceSmoothingMasks.Empty();
	rawMesh.FaceSmoothingMasks.AddZeroed( ToolbagMesh->indexCount/3 );
	//attribs
	for(unsigned i = 0; i < ToolbagMesh->vertexCount ; ++i)
	{
		if(ToolbagMesh->verticesComps >= 3)
		{
			FVector Vertex;
			Vertex.Set( ToolbagMesh->vertices[i*ToolbagMesh->verticesComps+0] * -ImportUI->SceneScaleFactor, ToolbagMesh->vertices[i*ToolbagMesh->verticesComps+1] * -ImportUI->SceneScaleFactor, ToolbagMesh->vertices[i*ToolbagMesh->verticesComps+2] * -ImportUI->SceneScaleFactor );
			rawMesh.VertexPositions[i] = Vertex;
		}
		if (ToolbagMesh->colorsComps >= 3)
		{
			Colors[i].R = ToolbagMesh->colors[i*ToolbagMesh->colorsComps];
			Colors[i].G = ToolbagMesh->colors[i*ToolbagMesh->colorsComps+1];
			Colors[i].B = ToolbagMesh->colors[i*ToolbagMesh->colorsComps+2];
			Colors[i].A = 1;
		}
		if(ToolbagMesh->tangentsComps >= 3)
			WedgeTangentX[i].Set( -ToolbagMesh->tangents[i*ToolbagMesh->tangentsComps+0], -ToolbagMesh->tangents[i*ToolbagMesh->tangentsComps+1], -ToolbagMesh->tangents[i*ToolbagMesh->tangentsComps+2]);
		if(ToolbagMesh->bitangentsComps >= 3)
			WedgeTangentY[i].Set( -ToolbagMesh->bitangents[i*ToolbagMesh->bitangentsComps+0], -ToolbagMesh->bitangents[i*ToolbagMesh->bitangentsComps+1], -ToolbagMesh->bitangents[i*ToolbagMesh->bitangentsComps+2]);
		if(ToolbagMesh->normalsComps >= 3)
			WedgeTangentZ[i].Set( -ToolbagMesh->normals[i*ToolbagMesh->normalsComps+0], -ToolbagMesh->normals[i*ToolbagMesh->normalsComps+1], -ToolbagMesh->normals[i*ToolbagMesh->normalsComps+2]);
		if(ToolbagMesh->texcoords0Comps >= 2)
			Texcoords0[i].Set( ToolbagMesh->texcoords0[i*ToolbagMesh->texcoords0Comps+0], 1-ToolbagMesh->texcoords0[i*ToolbagMesh->texcoords0Comps+1]);
		if(ToolbagMesh->texcoords1Comps >= 2)
			Texcoords1[i].Set( ToolbagMesh->texcoords1[i*ToolbagMesh->texcoords1Comps+0], 1-ToolbagMesh->texcoords1[i*ToolbagMesh->texcoords1Comps+1]);
	}

	TArray<FStaticMaterial> MeshMaterials;
	TArray<int32> MaterialIndicices;
	MaterialIndicices.AddZeroed( ToolbagMeshSceneObject->subMeshCount );
	for(uint32 j = 0; j < ToolbagMeshSceneObject->subMeshCount; ++j)
	{
		FString MaterialName = FString(ToolbagMeshSceneObject->subMeshes[j].materialName);
		UMaterialInterface* Material = Materials[MaterialName];
		int32 index = -1;
		for(int32 i = 0; i < MeshMaterials.Num(); ++i)
		{
			if(MeshMaterials[i].MaterialInterface == Material)
			{
				index = i;
				break;
			}
		}
		if(index == -1)
		{
			index = MeshMaterials.Add( FStaticMaterial(Material) );
			//We need the AdjacencyBuffer for Tesselation
			if(Materials[MaterialName]->GetMaterial()->D3D11TessellationMode == EMaterialTessellationMode::MTM_PNTriangles)
				sourceModel->BuildSettings.bBuildAdjacencyBuffer = true;
		}
		MaterialIndicices[j] = index;
	}

	//indices
	if( ToolbagMesh->indexCount > 0 )
	{
		unsigned* data = ToolbagMesh->indices;
		for(unsigned i = 0; i < ToolbagMesh->indexCount ; i+=3)
		{
			for(unsigned j = 0; j < 3; ++j)
			{
				rawMesh.WedgeIndices[i+j] = data[i+j];
				rawMesh.WedgeTangentX[i+j] = WedgeTangentX[data[i+j]];
				rawMesh.WedgeTangentY[i+j] = WedgeTangentY[data[i+j]];
				rawMesh.WedgeTangentZ[i+j] = WedgeTangentZ[data[i+j]];
				rawMesh.WedgeTexCoords[0][i+j] = Texcoords0[data[i+j]];
				rawMesh.WedgeTexCoords[1][i+j] = Texcoords1[data[i+j]];
				rawMesh.WedgeColors[i+j] = Colors[data[i+j]];
			}
			rawMesh.FaceSmoothingMasks[i/3] = 1;
			for(uint32 j = 0; j < ToolbagMeshSceneObject->subMeshCount; ++j)
			{
				if(ToolbagMeshSceneObject->subMeshes[j].startIndex < i && ToolbagMeshSceneObject->subMeshes[j].startIndex + ToolbagMeshSceneObject->subMeshes[j].indexCount >= i)
				{
					rawMesh.FaceMaterialIndices[i/3] = MaterialIndicices[j];
					break;
				}
			}
		}
	}

	sourceModel->RawMeshBulkData->SaveRawMesh( rawMesh );
	m->StaticMaterials.Empty();
	m->StaticMaterials.Append( MeshMaterials );
	/*if(m->SourceModels.Num() > 0)
	{
		m->SourceModels[0] = *sourceModel;
	}
	else
	{
		m->SourceModels.Add( *sourceModel );
	}*/
	TArray<FText> BuildErrors;
	m->Build(false);
	for( FText& Error : BuildErrors )
	{
		UE_LOG(LogToolbag, Warning, TEXT("%s"), *(Error.ToString()));
	}
	if (m->GetOuter())
	{
		m->GetOuter()->MarkPackageDirty();
	}
	else
	{
		m->MarkPackageDirty();
	}
	return m;
}



void ToolbagSceneObjectMesh::ParseComponent( UToolbagImportUI* ImportUI, USceneComponent* Component, FString Name )
{
	ToolbagUtils::MeshSceneObject* TMOL = (ToolbagUtils::MeshSceneObject*) TSO;

	if(name.Len() == 0)
	{
		name = Name;
	}
	UStaticMeshComponent* StaticMeshComponent = CastChecked<UStaticMeshComponent>( Component );
	UStaticMesh* mesh = ReadMesh( TMOL, name, ImportUI, Materials );
	StaticMeshComponent->SetStaticMesh( mesh );
	//set the materials explicitly just in case we use the same mesh with different materials multiple times
	for(int32 i = 0; i < mesh->StaticMaterials.Num(); ++i)
	{
		StaticMeshComponent->SetMaterial(i, mesh->StaticMaterials[i].MaterialInterface);
	}
}