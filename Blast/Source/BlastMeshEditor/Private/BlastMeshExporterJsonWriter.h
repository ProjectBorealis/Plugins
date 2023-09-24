// Copyright (c) 2017 NVIDIA Corporation. All rights reserved.


#pragma once

#include "BlastMeshExporter.h"

namespace Nv
{
namespace Blast
{

struct CollisionHull;

/**
	Interface to object which serializes collision geometry to JSON format. 
*/
class JsonCollisionExporter final : public ICollisionExporter
{
public: 
	JsonCollisionExporter() {}
	~JsonCollisionExporter() = default;

	/**
		Delete this object
	*/
	virtual void	release() override;

	/**
		Method creates file with given path and serializes given array of arrays of convex hulls to it in JSON format.
		\param[in] path			Output file path.
		\param[in] chunkCount	The number of chunks, may be less than the number of collision hulls.
		\param[in] hullOffsets	Collision hull offsets. Contains chunkCount + 1 element. First collision hull for i-th chunk: hull[hullOffsets[i]]. hullOffsets[chunkCount+1] is total number of hulls.
		\param[in] hulls		Array of pointers to convex hull descriptors, contiguously grouped for chunk[0], chunk[1], etc.
	*/
	virtual bool	writeCollision(const char* path, uint32_t chunkCount, const uint32_t* hullOffsets, const CollisionHull* const * hulls) override;
};

} // namespace Blast
} // namespace Nv

