// Copyright (c) 2017 NVIDIA Corporation. All rights reserved.


#pragma once

#include "BlastMeshExporter.h"
#include "Templates/SharedPointer.h"

struct NvBlastAsset;

namespace Nv
{
	namespace Blast
	{

		class ObjFileWriter : public IMeshFileWriter
		{
		public:

			ObjFileWriter() : mIntSurfaceMatIndex(-1), interiorNameStr("INTERIOR_MATERIAL") {  };
			~ObjFileWriter() {};

			virtual void release() override;

			virtual bool appendMesh(const AuthoringResult& aResult, const char* assetName, bool nonSkinned) override;

			/**
			Append rendermesh to scene. Meshes constructed from arrays of vertices and indices
			*/
			virtual bool appendMesh(const ExporterMeshData& meshData, const char* assetName, bool nonSkinned) override;

			/**
			Save scene to file.
			*/
			virtual bool saveToFile(const char* assetName, const char* outputPath) override;

			/**
				Set interior material index. Not supported in OBJ since AuthoringTool doesn't created OBJ with materials currently.
			*/
			virtual void setInteriorIndex(int32 index) override;

		private:
			TSharedPtr<ExporterMeshData> mMeshData;
			int32 mIntSurfaceMatIndex;
			FString	interiorNameStr;
		};

	}
}
