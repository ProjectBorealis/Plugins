//! @file renderopt.h
//! @brief Substance Air definition of render options used by renderer
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_RENDEROPT_H
#define _SUBSTANCE_AIR_FRAMEWORK_RENDEROPT_H

#include "typedefs.h"


namespace SubstanceAir
{


//! @brief Substance output options
//! Used at renderer creation and by Renderer::setOptions method.
//! Allows to set memory consumption budget and CPU usage.
struct RenderOptions
{
	//! @brief Memory budget allowed to renderer, in bytes.
	//! The default value is set to 512MB.
	size_t mMemoryBudget;
	
	//! @brief Maximum number of CPU Cores allowed to renderer.
	//! All cores used by default.
	//! @note Only relevant w/ CPU based Engines.
	size_t mCoresCount;

	//! @brief Default constructor
	RenderOptions() :
		mMemoryBudget(512*1024*1024),       // 512MB
		mCoresCount(32)                     // All
	{
	}
};


} // namespace SubstanceAir


#endif //_SUBSTANCE_AIR_FRAMEWORK_RENDEROPT_H
