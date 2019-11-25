//! @file ihttpresponse.h
//! @brief Substance Source HTTP Response
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Interface for HTTP Response
#ifndef _SUBSTANCE_SOURCE_IHTTPRESPONSE_H_
#define _SUBSTANCE_SOURCE_IHTTPRESPONSE_H_
#pragma once

#include <substance/source/source.h>

namespace Alg
{
namespace Source
{

/** HTTP Status Codes */
enum class HTTPStatus
{
	Unknown						= 0,

	OK							= 200,
	Created						= 201,
	MovedPermanently			= 301,
	URLRedirect					= 302,
	BadRequest					= 400,
	Unauthorized				= 401,
	Forbidden					= 403,
	NotFound					= 404,
	InternalServerError			= 500,
	NotImplemented				= 501,
	BadGateway					= 502,
};

/**/
class HTTPResponse
{
public:
	/** Create HTTP Response using response code and payload */
	inline HTTPResponse(HTTPStatus responseCode, BinaryData&& payload, int payloadLength);

	/** Return HTTP Response Code */
	inline HTTPStatus getResponseCode() const;

	/** Return reference to payload data */
	inline const BinaryData& getPayload() const;

	/** Return payload as string */
	inline String getPayloadAsString() const;

	/** Return payload length - This may be different than the payload itself */
	inline int getPayloadLength() const;

private:
	HTTPStatus					mResponseCode;
	BinaryData					mPayload;
	int							mPayloadLength;
};

/**/
inline HTTPResponse::HTTPResponse(HTTPStatus responseCode, BinaryData&& payload, int payloadLength)
	: mResponseCode(responseCode)
	, mPayload(std::move(payload))
	, mPayloadLength(payloadLength)
{
}

inline HTTPStatus HTTPResponse::getResponseCode() const
{
	return mResponseCode;
}

inline const BinaryData& HTTPResponse::getPayload() const
{
	return mPayload;
}

inline String HTTPResponse::getPayloadAsString() const
{
	return String(mPayload.begin(), mPayload.end());
}

inline int HTTPResponse::getPayloadLength() const
{
	return mPayloadLength;
}

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_IHTTPREQUEST_H_
