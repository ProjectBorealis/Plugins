// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkModule.h
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FSubstanceLiveLinkTextureLoader;

namespace SubstanceLiveLinkModule
{
extern const FName SubstanceLiveLinkAppIdentifier;
}

/**/
class ISubstanceLiveLinkModule : public IModuleInterface
{
public:
	/**
	 * Generic Accessor to be able to access to module from other classes
	 * @return Returns the Substance Live Link Module
	 */
	static ISubstanceLiveLinkModule* Get();

	/**
	 * Verify if this UClass is a mesh type
	 * @param Class the object class to check
	 * @return Returns true if the Class is a valid mesh type
	 */
	static bool IsMeshClass(UClass* Class);

	/**
	 * Grab the module's Texture Loader object
	 * @return Returns a pointer to the Texture Loader
	 */
	virtual TSharedRef<FSubstanceLiveLinkTextureLoader> GetTextureLoader() = 0;
};