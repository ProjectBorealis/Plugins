// Copyright (c) 2017 NVIDIA Corporation. All rights reserved.


#pragma once

#include "BlastMeshExporter.h"
#include <memory>
#include <vector>
#include <map>

namespace fbxsdk
{
	class FbxScene;
	class FbxNode;
	class FbxMesh;
	class FbxSkin;
	class FbxManager;
	class FbxSurfaceMaterial;
	class FbxDisplayLayer;
}

struct NvBlastAsset;

namespace Nv
{
namespace Blast
{
class Mesh;
struct Triangle;
struct CollisionHull;

class FbxFileWriter : public IMeshFileWriter
{
public:

	/**
		Initialize FBX sdk and create scene.
	*/
	FbxFileWriter();
	~FbxFileWriter() {};

	virtual void release() override;

	/**
		Get current scene;
	*/
	fbxsdk::FbxScene* getScene();

	/**
		Append rendermesh to scene. Meshes constructed from arrays of triangles.
	*/
	virtual bool appendMesh(const AuthoringResult& aResult, const char* assetName, bool nonSkinned) override;

	/**
		Append rendermesh to scene. Meshes constructed from arrays of vertex data (position, normal, uvs) and indices.
		Position, normal and uv has separate index arrays.
	*/
	virtual bool appendMesh(const ExporterMeshData& meshData, const char* assetName, bool nonSkinned) override;

	/**
		Save scene to file.
	*/
	virtual bool saveToFile(const char* assetName, const char* outputPath) override;

	/**
		Set interior material index.
	*/
	virtual void setInteriorIndex(int32_t index) override;

	/**
		Set true if FBX should be saved in ASCII mode.
	*/
	bool bOutputFBXAscii;

private:
	std::vector<fbxsdk::FbxSurfaceMaterial*> mMaterials;
	fbxsdk::FbxScene* mScene;
	fbxsdk::FbxDisplayLayer* mRenderLayer;

	//TODO we should track for every memory allocation and deallocate it not only for sdkManager
	std::shared_ptr<fbxsdk::FbxManager> sdkManager;
	std::map<uint32, fbxsdk::FbxNode*> chunkNodes;
	std::map<uint32, NvcVec3> worldChunkPivots;

	bool appendNonSkinnedMesh(const AuthoringResult& aResult, const char* assetName);
	bool appendNonSkinnedMesh(const ExporterMeshData& meshData, const char* assetName);
	void createMaterials(const ExporterMeshData& meshData);
	void createMaterials(const AuthoringResult& aResult);

	/**
	Append collision geometry to scene. Each node with collision geometry has "ParentalChunkIndex" property, which contain index of chunk
	which this collision geometry belongs to.
	*/
	bool appendCollisionMesh(uint32 meshCount, uint32* offsets, CollisionHull** hulls, const char* assetName);

	uint32 addCollisionHulls(uint32 chunkIndex, fbxsdk::FbxDisplayLayer* displayLayer, fbxsdk::FbxNode* parentNode, uint32 hullsCount, CollisionHull** hulls);
	uint32 createChunkRecursive(uint32 currentCpIdx, uint32 chunkIndex, fbxsdk::FbxNode *meshNode, fbxsdk::FbxNode* parentNode, fbxsdk::FbxSkin* skin, const AuthoringResult& aResult);
	uint32 createChunkRecursive(uint32 currentCpIdx, uint32 chunkIndex, fbxsdk::FbxNode *meshNode, fbxsdk::FbxNode* parentNode, fbxsdk::FbxSkin* skin, const ExporterMeshData& meshData);

	void createChunkRecursiveNonSkinned(const std::string& meshName, uint32 chunkIndex, fbxsdk::FbxNode* parentNode,
		const std::vector<fbxsdk::FbxSurfaceMaterial*>& materials, const AuthoringResult& aResult);

	void createChunkRecursiveNonSkinned(const std::string& meshName, uint32 chunkIndex, fbxsdk::FbxNode* parentNode,
		const std::vector<fbxsdk::FbxSurfaceMaterial*>& materials, const ExporterMeshData& meshData);

	void addControlPoints(fbxsdk::FbxMesh* mesh, const ExporterMeshData& meshData);
	void addBindPose();

	void generateSmoothingGroups(fbxsdk::FbxMesh* mesh, fbxsdk::FbxSkin* skin);
	void removeDuplicateControlPoints(fbxsdk::FbxMesh* mesh, fbxsdk::FbxSkin* skin);

	int32_t mInteriorIndex;
};

}
}
