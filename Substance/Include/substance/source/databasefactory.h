//! @file databasefactory.h
//! @brief Substance Source Database Factory
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Allocate a Database Object via latent factory
#ifndef _SUBSTANCE_SOURCE_DATABASEFACTORY_H_
#define _SUBSTANCE_SOURCE_DATABASEFACTORY_H_
#pragma once

#include <substance/source/database.h>

namespace Alg
{
namespace Source
{

class IHTTPRequest;

typedef std::function<void (DatabasePtr)> DatabaseFactoryCreateCallback;

/**/
class DatabaseFactory
{
public:
	void createInstance(DatabaseFactoryCreateCallback callback);

private:
	static DatabasePtr createDatabaseFromResponse(const String& response);
};

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_DATABASEFACTORY_H_
