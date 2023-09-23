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

#include "BlastMeshExporterJsonWriter.h"
#include "NvBlastExtAuthoringTypes.h"
#include "NvBlastExtAuthoringMesh.h"

#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

#include <sstream>
#include <ostream>
#include <iomanip>
#include "Containers/StringConv.h"


using namespace physx;
using namespace Nv::Blast;


#define JS_NAME(name) "\"" << name << "\": "

using namespace Nv::Blast;


static void SerializaHullPolygon(std::ostringstream& stream, const Nv::Blast::HullPolygon& p, uint32_t indent)
{
	std::string sindent(indent, '\t');
	std::string bindent(indent + 1, '\t');
	stream << sindent << "{\n" <<
		bindent << JS_NAME("indexBase") << p.indexBase << ",\n" <<
		bindent << JS_NAME("plane") << "[" << p.plane[0] << ", " << p.plane[1] << ", " << p.plane[2] << ", " << p.plane[3] << "],\n" <<
		bindent << JS_NAME("vertexCount") << p.vertexCount << "\n" <<
		sindent << "}";
}

static void SerializeCollisionHull(std::ostringstream& stream, const CollisionHull& hl, uint32_t indent)
{
	std::string sindent(indent, '\t');
	std::string bindent(indent + 1, '\t');

	stream << sindent << "{\n" << bindent << JS_NAME("indices") << "[";
	for (uint32_t i = 0; i < hl.indicesCount; ++i)
	{
		stream << hl.indices[i];
		if (i < hl.indicesCount - 1) stream << ", ";
	}
	stream << "],\n";
	stream << bindent << JS_NAME("points") << "[";
	for (uint32_t i = 0; i < hl.pointsCount; ++i)
	{
		auto& p = hl.points[i];
		stream << p.x << ", " << p.y << ", " << p.z;
		if (i < hl.pointsCount - 1) stream << ", ";
	}
	stream << "],\n";
	stream << bindent << JS_NAME("polygonData") << "[\n";
	for (uint32_t i = 0; i < hl.polygonDataCount; ++i)
	{
		SerializaHullPolygon(stream, hl.polygonData[i], indent + 1);
		if (i < hl.polygonDataCount - 1) stream << ", ";
		stream << "\n";
	}
	stream << bindent << "]\n";
	stream << sindent << "}";
}


void
JsonCollisionExporter::release()
{
	delete this;
}


bool
JsonCollisionExporter::JsonCollisionExporter::writeCollision(const char* path, uint32_t chunkCount, const uint32_t* hullOffsets, const CollisionHull* const * hulls)
{
	std::ostringstream OSStream;
	OSStream << std::fixed << std::setprecision(8);

	OSStream << "{\n" << "\t" << JS_NAME("CollisionData") << "[\n";
	for (uint32_t i = 0; i < chunkCount; ++i)
	{
		OSStream << "\t\t" << "[\n";
		for (uint32_t j = hullOffsets[i]; j < hullOffsets[i + 1]; ++j)
		{
			SerializeCollisionHull(OSStream, *hulls[j], 3);
			OSStream << ((j < hullOffsets[i + 1] - 1) ? ",\n" : "\n");
		}
		OSStream << "\t\t" << ((i < chunkCount - 1) ? "], \n" : "]\n");
	}
	OSStream << "\t]\n}";

	FString OutputString(OSStream.str().c_str());
	FString CollisionFileSavePath(path);
	return FFileHelper::SaveStringToFile(OutputString, *CollisionFileSavePath);
};
