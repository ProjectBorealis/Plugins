// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SourceCallbacks.h
#pragma once

#include <substance/source/callbacks.h>

class SourceHTTPRequest;

/**/
class SourceCallbacks : public Alg::Source::Callbacks
{
public:

	//from Alg::Source::Callbacks
	virtual void* memoryAlloc(size_t bytesCount, size_t alignment) override;
	virtual void memoryFree(void* ptr) override;
	virtual Alg::Source::SharedPtr<Alg::Source::IHTTPRequest> createHTTPRequest() override;
	virtual Alg::Source::SharedPtr<Alg::Source::IJSONObject> createJSONObject(const Alg::Source::String& json) override;
	virtual Alg::Source::String getPlatformName() const override;
};
