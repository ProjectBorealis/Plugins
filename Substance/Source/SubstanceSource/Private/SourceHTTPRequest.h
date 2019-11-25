// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SourceHTTPRequest.h
#pragma once

#include <substance/source/ihttprequest.h>

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

/**/
class SourceHTTPRequest : public Alg::Source::IHTTPRequest
{
public:
	SourceHTTPRequest();
	virtual ~SourceHTTPRequest() override;

	//from Alg::Source::IHTTPRequest
	virtual void setCallback(Alg::Source::IHTTPRequest::RequestCallback callback) override;
	virtual void setContent(const Alg::Source::String& content) override;
	virtual void setVerb(const Alg::Source::String& verb) override;
	virtual void setURL(const Alg::Source::String& url) override;
	virtual void addHeader(const Alg::Source::String& name, const Alg::Source::String& value) override;
	virtual Alg::Source::IHTTPRequest::ProgressHandlePtr processRequest() override;

private:
	TSharedPtr<IHttpRequest>										mHTTPRequest;
	Alg::Source::IHTTPRequest::RequestCallback						mCallback;
};
