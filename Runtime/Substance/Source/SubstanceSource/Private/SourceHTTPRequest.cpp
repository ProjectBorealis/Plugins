// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SourceHTTPRequest.cpp
#include "SourceHTTPRequest.h"
#include "SubstanceSourcePrivatePCH.h"
#include "SubstanceSourceModule.h"

#include "Interfaces/IHttpResponse.h"

#include <substance/source/callbacks.h>
#include <substance/source/ihttpresponse.h>

//==================================================
class SourceHTTPRequestProgressHandle : public Alg::Source::IHTTPRequest::ProgressHandle
{
public:
	SourceHTTPRequestProgressHandle()
		: Alg::Source::IHTTPRequest::ProgressHandle()
		, mBytesSent(0)
		, mBytesReceived(0)
	{
	}

	/** Get number of bytes sent so far */
	virtual int getBytesSent() const override
	{
		return mBytesSent;
	}

	/** Get number of bytes received so far */
	virtual int getBytesReceived() const override
	{
		return mBytesReceived;
	}

	/** Update the progress handle */
	void update(int bytesSent, int bytesReceived)
	{
		mBytesSent = bytesSent;
		mBytesReceived = bytesReceived;
	}

private:
	int mBytesSent;
	int mBytesReceived;
};

//==================================================
SourceHTTPRequest::SourceHTTPRequest()
	: Alg::Source::IHTTPRequest()
	, mHTTPRequest(FHttpModule::Get().CreateRequest())
{
	//add default headers
	addHeader("User-Agent", Alg::Source::Callbacks::getInstance()->getUserAgentString());
}

SourceHTTPRequest::~SourceHTTPRequest()
{
	if (mHTTPRequest.IsValid())
	{
		mHTTPRequest->CancelRequest();
	}
}

void SourceHTTPRequest::setCallback(Alg::Source::IHTTPRequest::RequestCallback callback)
{
	mCallback = callback;
}

void SourceHTTPRequest::setContent(const Alg::Source::String& content)
{
	check(mHTTPRequest.IsValid());
	mHTTPRequest->SetContentAsString(content.c_str());
}

void SourceHTTPRequest::setVerb(const Alg::Source::String& verb)
{
	check(mHTTPRequest.IsValid());
	mHTTPRequest->SetVerb(verb.c_str());
}

void SourceHTTPRequest::setURL(const Alg::Source::String& url)
{
	check(mHTTPRequest.IsValid());
	mHTTPRequest->SetURL(url.c_str());
}

void SourceHTTPRequest::addHeader(const Alg::Source::String& name, const Alg::Source::String& value)
{
	check(mHTTPRequest.IsValid());
	mHTTPRequest->SetHeader(name.c_str(), value.c_str());
}

Alg::Source::IHTTPRequest::ProgressHandlePtr SourceHTTPRequest::processRequest()
{
	check(mHTTPRequest.IsValid());

	Alg::Source::IHTTPRequest::RequestCallback capCallback = mCallback;
	TSharedPtr<IHttpRequest> httpRequestPtr = mHTTPRequest;

	mCallback = nullptr;

	//create a listener for module shutdown so we appropriately clean up
	TSharedPtr<IHttpRequest> shutdownHttpRequest = mHTTPRequest;
	FDelegateHandle shutdownDelegateHandle = ISubstanceSourceModule::Get()->OnModuleShutdown().AddLambda([shutdownHttpRequest]()
	{
		shutdownHttpRequest->OnRequestProgress().Unbind();
		shutdownHttpRequest->OnProcessRequestComplete().Unbind();
		shutdownHttpRequest->CancelRequest();
	});

	//create progress handle and update it frequently
	Alg::Source::SharedPtr<SourceHTTPRequestProgressHandle> progressHandle = Alg::Source::MakeShared<SourceHTTPRequestProgressHandle>();
	mHTTPRequest->OnRequestProgress().BindLambda([progressHandle](FHttpRequestPtr httpRequest, int32 bytesSent, int32 bytesReceived)
	{
		progressHandle->update(bytesSent, bytesReceived);
	});

	//bind request complete delegate to handle request completion logic
	mHTTPRequest->OnProcessRequestComplete().BindLambda([capCallback, shutdownDelegateHandle](FHttpRequestPtr httpRequest, FHttpResponsePtr httpResponse, bool bSucceeded)
	{
		if (bSucceeded != true)
		{
			const TArray<uint8>& unrealPayload = httpRequest->GetContent();
			Alg::Source::HTTPStatus responseCode = (Alg::Source::HTTPStatus)httpRequest->GetStatus();
			Alg::Source::BinaryData payload(unrealPayload.GetData(), unrealPayload.GetData() + unrealPayload.Num());
			Alg::Source::HTTPResponse algResponse(responseCode, std::move(payload), unrealPayload.Num()* sizeof(uint8));
			capCallback(algResponse);
		}
		else
		{
			const TArray<uint8>& unrealPayload = httpResponse->GetContent();
			Alg::Source::HTTPStatus responseCode = (Alg::Source::HTTPStatus)httpResponse->GetResponseCode();
			Alg::Source::BinaryData payload(unrealPayload.GetData(), unrealPayload.GetData() + unrealPayload.Num());
			Alg::Source::HTTPResponse algResponse(responseCode, std::move(payload), httpResponse->GetContentLength());
			capCallback(algResponse);
		}

		//unbind requests here because the HTTP Manager can take a long time to 'clean up', potentially leading to a shutdown crash
		httpRequest->OnProcessRequestComplete().Unbind();
		httpRequest->OnRequestProgress().Unbind();

		//remove shutdown listener we bound earlier
		ISubstanceSourceModule::Get()->OnModuleShutdown().Remove(shutdownDelegateHandle);
	});

	mHTTPRequest->ProcessRequest();
	mHTTPRequest.Reset();

	return progressHandle;
}
