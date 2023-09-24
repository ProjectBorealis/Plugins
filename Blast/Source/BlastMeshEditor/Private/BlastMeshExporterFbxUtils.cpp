// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2017 NVIDIA Corporation. All rights reserved.

#include "BlastMeshExporterFbxUtils.h"

#include "NvBlastExtAuthoringTypes.h"
#include <sstream>


void FbxUtils::VertexToFbx(const Nv::Blast::Vertex& vert, FbxVector4& outVertex, FbxVector4& outNormal, FbxVector2& outUV)
{
	NvcVec3ToFbx(vert.p, outVertex);
	NvcVec3ToFbx(vert.n, outNormal);
	NvcVec2ToFbx(vert.uv[0], outUV);
}

void FbxUtils::NvcVec3ToFbx(const NvcVec3& inVector, FbxVector4& outVector)
{
	outVector[0] = inVector.x;
	outVector[1] = inVector.y;
	outVector[2] = inVector.z;
	outVector[3] = 0;
}

void FbxUtils::NvcVec2ToFbx(const NvcVec2& inVector, FbxVector2& outVector)
{
	outVector[0] = inVector.x;
	outVector[1] = inVector.y;
}

FbxAxisSystem FbxUtils::getBlastFBXAxisSystem()
{
	const FbxAxisSystem::EUpVector upVector = FbxAxisSystem::eZAxis;
	//From the documentation: If the up axis is Z, the remain two axes will X And Y, so the ParityEven is X, and the ParityOdd is Y
	const FbxAxisSystem::EFrontVector frontVector = FbxAxisSystem::eParityOdd;
	const FbxAxisSystem::ECoordSystem rightVector = FbxAxisSystem::eRightHanded;
	return FbxAxisSystem(upVector, frontVector, rightVector);
}

FbxSystemUnit FbxUtils::getBlastFBXUnit()
{
	return FbxSystemUnit::cm;
}

FString FbxUtils::FbxAxisSystemToString(const FbxAxisSystem& axisSystem)
{
	FString ss;
	int upSign, frontSign;
	FbxAxisSystem::EUpVector upVector = axisSystem.GetUpVector(upSign);
	FbxAxisSystem::EFrontVector frontVector = axisSystem.GetFrontVector(frontSign);
	FbxAxisSystem::ECoordSystem  coordSystem = axisSystem.GetCoorSystem();
	ss.Append(TEXT("Predefined Type: "));
	if (axisSystem == FbxAxisSystem::MayaZUp)
	{
		ss.Append(TEXT("MayaZUP"));
	}
	else if (axisSystem == FbxAxisSystem::MayaYUp)
	{
		ss.Append(TEXT("MayaYUp"));
	}
	else if (axisSystem == FbxAxisSystem::Max)
	{
		ss.Append(TEXT("Max"));
	}
	else if (axisSystem == FbxAxisSystem::Motionbuilder)
	{
		ss.Append(TEXT("Motionbuilder"));
	}
	else if (axisSystem == FbxAxisSystem::OpenGL)
	{
		ss.Append(TEXT("OpenGL"));
	}
	else if (axisSystem == FbxAxisSystem::DirectX)
	{
		ss.Append(TEXT("OpenGL"));
	}
	else if (axisSystem == FbxAxisSystem::Lightwave)
	{
		ss.Append(TEXT("OpenGL"));
	}
	else
	{
		ss.Append(TEXT("<Other>"));
	}

	ss.Append(TEXT(" UpVector: "));
	ss.Append(upSign > 0 ? TEXT("+") : TEXT("-"));

	switch (upVector)
	{
	case FbxAxisSystem::eXAxis: ss.Append(TEXT("eXAxis")); break;
	case FbxAxisSystem::eYAxis: ss.Append(TEXT("eYAxis")); break;
	case FbxAxisSystem::eZAxis: ss.Append(TEXT("eZAxis")); break;
	default: ss.Append(TEXT("<unknown>")); break;
	}

	ss.Append(TEXT(" FrontVector: "));
	ss.Append(frontSign > 0 ? TEXT("+") : TEXT("-"));
	switch (frontVector)
	{
	case FbxAxisSystem::eParityEven:ss.Append(TEXT("eParityEven")); break;
	case FbxAxisSystem::eParityOdd: ss.Append(TEXT("eParityOdd")); break;
	default:ss.Append(TEXT("<unknown>")); break;
	}

	ss.Append(TEXT(" CoordSystem: "));
	switch (coordSystem)
	{
	case FbxAxisSystem::eLeftHanded: ss.Append(TEXT("eLeftHanded")); break;
	case FbxAxisSystem::eRightHanded: ss.Append(TEXT("eRightHanded")); break;
	default: ss.Append(TEXT("<unknown>")); break;
	}

	return ss;
}

FString FbxUtils::FbxSystemUnitToString(const FbxSystemUnit& systemUnit)
{
	return FString(systemUnit.GetScaleFactorAsString());
}

const FString currentChunkPrefix = TEXT("chunk_");
const FString oldChunkPrefix = TEXT("bone_");

static uint32 getChunkIndexForNodeInternal(const FString& chunkPrefix, FbxNode* node, uint32* outParentChunkIndex /*=nullptr*/)
{
	if (!node)
	{
		//Found nothing
		return UINT32_MAX;
	}

	FString nodeName(node->GetNameOnly().Buffer());
	nodeName.ToLowerInline();

	if (nodeName.StartsWith(chunkPrefix))
	{
		int32 ChunkIdx = FCString::Atoi(*nodeName.Mid(chunkPrefix.Len()));
		if (ChunkIdx >= 0)
		{
			if (outParentChunkIndex)
			{
				*outParentChunkIndex = getChunkIndexForNodeInternal(chunkPrefix, node->GetParent(), nullptr);
			}
			return ChunkIdx;
		}
	}

	return getChunkIndexForNodeInternal(chunkPrefix, node->GetParent(), outParentChunkIndex);
}

uint32 FbxUtils::getChunkIndexForNode(FbxNode* node, uint32* outParentChunkIndex /*=nullptr*/)
{
	return getChunkIndexForNodeInternal(currentChunkPrefix, node, outParentChunkIndex);
}

uint32 FbxUtils::getChunkIndexForNodeBackwardsCompatible(FbxNode* node, uint32* outParentChunkIndex /*= nullptr*/)
{
	return getChunkIndexForNodeInternal(oldChunkPrefix, node, outParentChunkIndex);
}

FString FbxUtils::getChunkNodeName(uint32 chunkIndex)
{
	//This naming is required for the UE4 plugin to find them
	return currentChunkPrefix + FString::FromInt(chunkIndex);
}

FString FbxUtils::getCollisionGeometryLayerName()
{
	static const FString CollisionStr(TEXT("Collision"));
	return CollisionStr;
}

FString FbxUtils::getRenderGeometryLayerName()
{
	static const FString RenderStr(TEXT("Render"));
	return RenderStr;
}
