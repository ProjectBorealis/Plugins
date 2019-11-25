/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "Factory/MultiPackerRuntimeGraphFactory.h"
#include "UnrealEd.h"
#include "IAssetTools.h"
#include "PackageTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "ImageUtils.h"
#include "MultiPackerRuntimeGraph.h"

#define LOCTEXT_NAMESPACE "MultiPacker"

UMultiPackerRuntimeGraphFactory::UMultiPackerRuntimeGraphFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMultiPackerRuntimeGraph::StaticClass();
}

UMultiPackerRuntimeGraphFactory::~UMultiPackerRuntimeGraphFactory()
{
}

UObject* UMultiPackerRuntimeGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UMultiPackerRuntimeGraph* Result = NewObject<UMultiPackerRuntimeGraph>(InParent, Class, Name, Flags | RF_Transactional);//Flags
	//Result->TargetName = *Name.ToString();
	//Result->Flags = Flags;
	return Result;
}

FText UMultiPackerRuntimeGraphFactory::GetDisplayName() const
{
	return LOCTEXT("UMultiPackerRuntimeGraphFactory", "MultiPacker Runtime Graph");
}

#undef LOCTEXT_NAMESPACE