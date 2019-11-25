//! @file ihttprequest.h
//! @brief Substance Source HTTP Request
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Interface for HTTP Requests
#ifndef _SUBSTANCE_SOURCE_IHTTPREQUEST_H_
#define _SUBSTANCE_SOURCE_IHTTPREQUEST_H_
#pragma once

#include <substance/source/source.h>

namespace Alg
{
namespace Source
{

typedef SharedPtr<class IHTTPRequest> IHTTPRequestPtr;

class HTTPResponse;

/**/
class IHTTPRequest
{
public:
	/**/
	class ProgressHandle
	{
	public:
		/** Destructor */
		virtual ~ProgressHandle() = default;

		/** Get number of bytes sent so far */
		virtual int getBytesSent() const = 0;

		/** Get number of bytes received so far */
		virtual int getBytesReceived() const = 0;
	};

	/** ProgressHandle Shared Ptr Type */
	typedef SharedPtr<ProgressHandle> ProgressHandlePtr;

	/** Callback function */
	typedef std::function<void (const HTTPResponse&)> RequestCallback;

public:
	/** Destructor */
	virtual ~IHTTPRequest() = default;

	/** Set ProcessRequest Callback */
	virtual void setCallback(RequestCallback callback) = 0;

	/** Set Content for POST requests */
	virtual void setContent(const String& content) = 0;

	/** Set the Verb of the request (POST/GET) */
	virtual void setVerb(const String& verb) = 0;

	/** Set the URL of the request */
	virtual void setURL(const String& url) = 0;

	/** Add a header to the request */
	virtual void addHeader(const String& name, const String& value) = 0;

	/** Begin processing the request. The callback is invoked when the process completes */
	virtual ProgressHandlePtr processRequest() = 0;
};

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_IHTTPREQUEST_H_
