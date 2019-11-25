//! @file callbacks.h
//! @brief Substance Source Database Object
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Substance Source Callbacks Object
#ifndef _SUBSTANCE_SOURCE_CALLBACKS_H_
#define _SUBSTANCE_SOURCE_CALLBACKS_H_
#pragma once

#include "ihttprequest.h"
#include "ijsonobject.h"

namespace Alg
{
namespace Source
{

/**/
class Callbacks
{
public:
	virtual ~Callbacks() = default;

	/** Set global callbacks instance */
	static void setInstance(Callbacks* callbacks);

	/** Retrieve global callbacks instance (can be nullptr) */
	static Callbacks* getInstance();

	/** Retrieve User Agent String */
	String getUserAgentString() const;

	/** Allocate memory at a given alignment */
	virtual void* memoryAlloc(size_t bytesCount, size_t alignment) = 0;

	/** Free memory at a given address */
	virtual void memoryFree(void* ptr) = 0;

	/** Create a new IHTTPRequest instance */
	virtual IHTTPRequestPtr createHTTPRequest() = 0;

	/** Create a new JSON Object from string */
	virtual IJSONObjectPtr createJSONObject(const String& json = "{}") = 0;

	/** Get platform string */
	virtual String getPlatformName() const = 0;

private:
	static Callbacks*			sInstance;
};

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_CALLBACKS_H_
