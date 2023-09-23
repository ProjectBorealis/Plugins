#pragma once

#include "CoreMinimal.h"

BLASTLOADER_API FString GetBlastDLLPath(FString ConfigFolderName);

BLASTLOADER_API void* LoadBlastDLL(const FString& DLLPath, const TCHAR* BaseName);