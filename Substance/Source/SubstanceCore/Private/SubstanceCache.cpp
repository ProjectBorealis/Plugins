// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCache.cpp

#include "SubstanceCache.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceCoreClasses.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceTexture2D.h"

#include "substance/framework/channel.h"
#include "substance/framework/output.h"
#include "substance/framework/graph.h"
#include "substance/framework/typedefs.h"
#include "substance/texture.h"
#include "substance/pixelformat.h"

#include "RenderUtils.h"

#include "Misc/Paths.h"
#include "HAL/FileManager.h"

#define SUBSTANCECACHE_VERSION 1


TSharedPtr<Substance::SubstanceCache> Substance::SubstanceCache::SbsCache;

bool Substance::SubstanceCache::CanReadFromCache(SubstanceAir::weak_ptr<SubstanceAir::GraphInstance> graph)
{
#if PLATFORM_PS4
	// disable caching for PS4
	return false;
#else
	bool canReadFromCache = false;
	auto CurrentInstance = graph.lock();

	if (!CurrentInstance)
	{
		return false;
	}

	//make sure all enabled outputs have a cached file
	{
		auto iter = CurrentInstance->getOutputs().begin();
		while (iter != CurrentInstance->getOutputs().end())
		{
			if ((*iter)->mEnabled && (*iter)->mUserData != 0)
			{
				FGuid& CurrentGuid = reinterpret_cast<OutputInstanceData*>((*iter)->mUserData)->CacheGuid;
				FString path = Substance::SubstanceCache::GetPathForGuid(CurrentGuid);

				if (IFileManager::Get().FileSize(*path) == INDEX_NONE)
				{
					return false;
				}
				else
				{
					canReadFromCache = true;
				}
			}

			iter++;
		}
	}

	return canReadFromCache;
#endif
}

bool Substance::SubstanceCache::ReadFromCache(SubstanceAir::weak_ptr<SubstanceAir::GraphInstance> graph)
{
#if PLATFORM_PS4
	// disable caching for PS4
	return false;
#else
	auto CurrentInstance = graph.lock();

	if (!CurrentInstance)
	{
		return false;
	}

	if (!reinterpret_cast<GraphInstanceData*>(CurrentInstance->mUserData)->ParentGraph->ParentFactory->ShouldCacheOutput())
	{
		return false;
	}

	if (!CanReadFromCache(CurrentInstance))
	{
		return false;
	}
	//read in cached data and upload
	auto Oiter = CurrentInstance->getOutputs().begin();
	while (Oiter != CurrentInstance->getOutputs().end())
	{
		if ((*Oiter)->mEnabled)
		{
			SubstanceAir::OutputInstance& outputInstance = *const_cast<SubstanceAir::OutputInstance*>(*Oiter);
			FGuid& CurrentGuid = reinterpret_cast<OutputInstanceData*>(outputInstance.mUserData)->CacheGuid;
			FString path = Substance::SubstanceCache::GetPathForGuid(CurrentGuid);
			FArchive* Ar = IFileManager::Get().CreateFileReader(*path);

			if (Ar)
			{
				SubstanceTexture output;
				FMemory::Memzero(output);

				if (Substance::SubstanceCache::SerializeTexture(*Ar, output))
				{
					delete Ar;

					USubstanceTexture2D* texture = reinterpret_cast<OutputInstanceData*>(outputInstance.mUserData)->Texture.Get();
					Substance::Helpers::UpdateTexture(output, &outputInstance, false);

#if SUBSTANCE_MEMORY_STAT
					DEC_MEMORY_STAT_BY(STAT_SubstanceCacheMemory, FMemory::GetAllocSize(output.buffer));
#endif
					FMemory::Free(output.buffer);
				}
				else
				{
					delete Ar;
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		Oiter++;
	}

	return true;
#endif
}

void Substance::SubstanceCache::CacheOutput(SubstanceAir::OutputInstance* output, const SubstanceTexture& result)
{
#if PLATFORM_PS4
	return;
#endif //!PLATFORM_PS4

	FGuid& CurrentGuid = reinterpret_cast<OutputInstanceData*>(output->mUserData)->CacheGuid;
	FString filename = Substance::SubstanceCache::GetPathForGuid(CurrentGuid);
	FArchive* Ar = IFileManager::Get().CreateFileWriter(*filename, 0);

	if (Ar)
	{
		SerializeTexture(*Ar, const_cast<SubstanceTexture&>(result));
		delete Ar;
	}
}

FString Substance::SubstanceCache::GetPathForGuid(const FGuid& guid) const
{
	return FString::Printf(TEXT("%s/Substance/%s.cache"), *FPaths::ProjectSavedDir(), *(guid.ToString()));
}

bool Substance::SubstanceCache::SerializeTexture(FArchive& Ar, SubstanceTexture& result) const
{
	uint32 version = SUBSTANCECACHE_VERSION;

	Ar << version;

	if (Ar.IsLoading())
	{
		if (version != SUBSTANCECACHE_VERSION)
		{
			UE_LOG(LogSubstanceCore, Warning, TEXT("Out of date Substance cache entry, will regenerate"));
			return false;
		}
	}

	Ar << result.level0Width;
	Ar << result.level0Height;
	Ar << result.pixelFormat;
	Ar << result.channelsOrder;
	Ar << result.mipmapCount;

	EPixelFormat pixelFormat = Substance::Helpers::SubstanceToUe4Format((SubstancePixelFormat)result.pixelFormat, (SubstanceChannelsOrder)result.channelsOrder);
	SIZE_T bufferSize = CalcTextureSize(result.level0Width, result.level0Height, pixelFormat, result.mipmapCount);

	if (Ar.IsLoading())
	{
		check(result.buffer == NULL);
		result.buffer = FMemory::Malloc(bufferSize);
#if SUBSTANCE_MEMORY_STAT
		INC_MEMORY_STAT_BY(STAT_SubstanceCacheMemory, bufferSize);
#endif
	}

	Ar.Serialize(result.buffer, bufferSize);

	return true;
}

void Substance::SubstanceCache::Shutdown()
{

}
