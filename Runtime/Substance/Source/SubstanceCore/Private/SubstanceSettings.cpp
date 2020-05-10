// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceSettings.cpp

#include "SubstanceSettings.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceInstanceFactory.h"

USubstanceSettings::USubstanceSettings(const FObjectInitializer& PCIP)
	: Super(PCIP)
	, MemoryBudgetMb(512)
	, CPUCores(32)
	, AsyncLoadMipClip(3)
	, MaxAsyncSubstancesRenderedPerFrame(10)
	, SubstanceEngine(SET_CPU)
	, DefaultSubstanceOutputSizeX(EDefaultSubstanceTextureSize::SIZE_1024)
	, DefaultSubstanceOutputSizeY(EDefaultSubstanceTextureSize::SIZE_1024)
	, DefaultTemplateMaterial((UMaterialInterface*)TAssetPtr<UObject>(
		FSoftObjectPath(FString(TEXT("/Substance/Templates/parent_materials/Substance_Basic_Template.Substance_Basic_Template")))).LoadSynchronous())
{
}
