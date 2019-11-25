// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SourceCallbacks.cpp
#include "SourceCallbacks.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SourceHTTPRequest.h"
#include "SourceJSONObject.h"

//==================================================
void* SourceCallbacks::memoryAlloc(size_t bytesCount, size_t alignment)
{
	return FMemory::Malloc(bytesCount, alignment);
}

void SourceCallbacks::memoryFree(void* ptr)
{
	return FMemory::Free(ptr);
}

Alg::Source::SharedPtr<Alg::Source::IHTTPRequest> SourceCallbacks::createHTTPRequest()
{
	return Alg::Source::MakeShared<SourceHTTPRequest>();
}

Alg::Source::SharedPtr<Alg::Source::IJSONObject> SourceCallbacks::createJSONObject(const Alg::Source::String& json)
{
	return Alg::Source::MakeShared<SourceJSONObject>(json);
}

Alg::Source::String SourceCallbacks::getPlatformName() const
{
	Alg::Source::Stringstream platformStr;
	platformStr << "UnrealEngine4 "
	            << PREPROCESSOR_TO_STRING(UBT_COMPILED_TARGET)
	            << " "
	            << PREPROCESSOR_TO_STRING(UBT_COMPILED_PLATFORM);

	return platformStr.str();
}
