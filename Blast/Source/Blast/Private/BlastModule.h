#pragma once
#include "CoreMinimal.h"

#include "Stats/Stats.h"
#include "Stats/Stats2.h"

#include "Modules/ModuleInterface.h"

class FBlastModule : public IModuleInterface
{
public:
	FBlastModule();
	virtual ~FBlastModule() = default;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

#if STATS
DECLARE_STATS_GROUP(TEXT("Blast"), STATGROUP_Blast, STATCAT_Advanced);
#endif

