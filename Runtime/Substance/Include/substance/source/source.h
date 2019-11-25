//! @file source.h
//! @brief Substance Source Entry Header
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Base include file for Substance Source API
#ifndef _SUBSTANCE_SOURCE_SOURCE_H_
#define _SUBSTANCE_SOURCE_SOURCE_H_
#pragma once

//C++ includes
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iterator>

//macros
#define SUBSTANCE_SOURCE_URL_GRAPHQL				"https://source-api.allegorithmic.com/graphql"
#define SUBSTANCE_SOURCE_URL_USER_INFO				"https://source-api.allegorithmic.com/user"
#define SUBSTANCE_SOURCE_URL_USER_LOGIN				"https://source-api.allegorithmic.com/login"

#define SUBSTANCE_SOURCE_MIMETYPE_SBSAR				"application/vnd.allegorithmic.sbsar"

#define SUBSTANCE_SOURCE_FRAMEWORK_VERSION_MAJOR	1
#define SUBSTANCE_SOURCE_FRAMEWORK_VERSION_MINOR	0

#define SUBSTANCE_SOURCE_TO_STRING(x)				SUBSTANCE_SOURCE_TO_STRING_INNER(x)
#define SUBSTANCE_SOURCE_TO_STRING_INNER(x)			#x

//memory
#include <substance/source/memory.h>
#include <substance/source/typedefs.h>

//common types and functions
namespace Alg
{
namespace Source
{

/** Binary Data */
typedef Vector<std::uint8_t> BinaryData;

/** Tokenize a string into a vector */
static inline Vector<String> tokenizeString(const String& str)
{
	Vector<String> tokens;
	Istringstream iss(str.c_str());
	std::copy(std::istream_iterator<String>(iss), std::istream_iterator<String>(), std::back_inserter(tokens));
	return tokens;
}

/** Convert string to lower case */
static inline void toLowerCase(String& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

class Platform
{
public:
	/** Singleton Access */
	static Platform& GetInstance();

	/** Used for platform specific functionality */
	enum class SourcePlatformID
	{
		Standard,
		UE4_Studio,
		UE4_Default,
		Unity,
	};

	/* Pass platform specific and expected string ID to set ID */
	void SetSourceIntegrationID(Alg::Source::String);

	/* Pass platform specific and expected string ID to set ID */
	SourcePlatformID GetPlatformID() const;

private:

	/** Which platform is set */
	SourcePlatformID mPlatformID;

	/** Default Constructor */
	Platform();
};

} //Source
} //Alg

//source API includes
#include <substance/source/database.h>

#endif //_SUBSTANCE_SOURCE_SOURCE_H_
