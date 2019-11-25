//! @file userfactory.h
//! @brief Substance Source User Factory
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Allocate a User Object via latent factory
#ifndef _SUBSTANCE_SOURCE_USERFACTORY_H_
#define _SUBSTANCE_SOURCE_USERFACTORY_H_
#pragma once

#include <substance/source/user.h>

namespace Alg
{
namespace Source
{

class Database;
class IHTTPRequest;

typedef std::function<void(UserPtr, const HTTPResponse&)> UserFactoryCreateCallback;

/**/
class UserFactory
{
public:
	/** Acquire user using username and password */
	void createInstance(const String& username, const String& password, DatabasePtr database, UserFactoryCreateCallback callback);

	/** Acquire user using refresh token of existing User object */
	void createInstance(const String& refreshToken, DatabasePtr database, UserFactoryCreateCallback callback);

private:
	/** Create a user with a given JSON request */
	void createInstance(IJSONObjectPtr json, DatabasePtr database, UserFactoryCreateCallback callback);

	/** Once a response is received, create a user from it*/
	static void createUserFromResponse(const HTTPResponse& response, DatabasePtr database, UserFactoryCreateCallback callback);
};

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_USERFACTORY_H_
