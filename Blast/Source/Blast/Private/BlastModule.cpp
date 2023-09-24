#include "BlastModule.h"

#include "NvBlastGlobals.h"
#include "BlastGlobals.h"
#include "Logging/LogVerbosity.h"
#include "Modules/ModuleManager.h"

class BlastAllocatorCallback final : public nvidia::NvAllocatorCallback
{
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* ptr = FMemory::Malloc(size, 0x10);
		check((reinterpret_cast<size_t>(ptr) & 0x0F) == 0);
		return ptr;

	}

	virtual void deallocate(void* ptr)
	{
		FMemory::Free(ptr);
	}
};

static BlastAllocatorCallback g_blastAllocatorCallback;

class BlastErrorCallback final : public nvidia::NvErrorCallback
{
	virtual void reportError(nvidia::NvErrorCode::Enum code, const char* message, const char* file, int line)
	{
#if !NO_LOGGING
		ELogVerbosity::Type verbosity;
		switch (code)
		{
		case nvidia::NvErrorCode::eABORT:
			verbosity = ELogVerbosity::Fatal;
			break;
		case nvidia::NvErrorCode::eOUT_OF_MEMORY:
		case nvidia::NvErrorCode::eINTERNAL_ERROR:
			verbosity = ELogVerbosity::Error;
			break;
		case nvidia::NvErrorCode::eINVALID_PARAMETER:
		case nvidia::NvErrorCode::eINVALID_OPERATION:
			verbosity = ELogVerbosity::Error;
			break;
		case nvidia::NvErrorCode::ePERF_WARNING:
		case nvidia::NvErrorCode::eDEBUG_WARNING:
			verbosity = ELogVerbosity::Warning;
			break;
		case nvidia::NvErrorCode::eDEBUG_INFO:
			verbosity = ELogVerbosity::Log;
			break;
		case nvidia::NvErrorCode::eNO_ERROR:
			verbosity = ELogVerbosity::Log;
			break;
		default:
			verbosity = ELogVerbosity::Log;
			break;
		}

		if ((verbosity & ELogVerbosity::VerbosityMask) <= FLogCategoryLogBlast::CompileTimeVerbosity)
		{
			if (!LogBlast.IsSuppressed(verbosity))
			{
				FMsg::Logf(file, line, LogBlast.GetCategoryName(), verbosity,
					TEXT("Blast Log : %s"), ANSI_TO_TCHAR(message));
			}
		}
#endif
	}
};

static BlastErrorCallback g_blastErrorCallback;
IMPLEMENT_MODULE(FBlastModule, Blast);

FBlastModule::FBlastModule()
{
}

void FBlastModule::StartupModule()
{
	// Set the allocator and log for serialization
	NvBlastGlobalSetAllocatorCallback(&g_blastAllocatorCallback);
	NvBlastGlobalSetErrorCallback(&g_blastErrorCallback);
}

void FBlastModule::ShutdownModule()
{
}
