/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "IMultiPackerRuntime.h"

DEFINE_LOG_CATEGORY(MultiPackerRuntime)

class FMultiPackerRuntime : public IMultiPackerRuntime
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FMultiPackerRuntime, MultiPackerRuntime )

void FMultiPackerRuntime::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}

void FMultiPackerRuntime::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
