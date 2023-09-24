// Copyright (c) 2017 NVIDIA Corporation. All rights reserved.


#pragma once

#include "CoreMinimal.h"

#include "fbxsdk.h"
#include "NvCTypes.h"

namespace Nv
{
	namespace Blast
	{
		struct Vertex;
	}
}

class FbxUtils
{
public:
	static void VertexToFbx(const Nv::Blast::Vertex& vert, FbxVector4& outVertex, FbxVector4& outNormal, FbxVector2& outUV);

	static void NvcVec3ToFbx(const NvcVec3& inVector, FbxVector4& outVector);
	static void NvcVec2ToFbx(const NvcVec2& inVector, FbxVector2& outVector);

	static FbxAxisSystem getBlastFBXAxisSystem();
	static FbxSystemUnit getBlastFBXUnit();

	static FString FbxAxisSystemToString(const FbxAxisSystem& axisSystem);
	static FString FbxSystemUnitToString(const FbxSystemUnit& systemUnit);

	//returns UINT32_MAX if not a chunk
	static uint32 getChunkIndexForNode(FbxNode* node, uint32* outParentChunkIndex = nullptr);
	//Search using the old naming 
	static uint32 getChunkIndexForNodeBackwardsCompatible(FbxNode* node, uint32* outParentChunkIndex = nullptr);
	static FString getChunkNodeName(uint32 chunkIndex);

	static FString getCollisionGeometryLayerName();
	static FString getRenderGeometryLayerName();
};
