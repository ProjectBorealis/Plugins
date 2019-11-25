/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "Factory/MultiPackerFactory.h"
#include "UnrealEd.h"
#include "IAssetTools.h"
#include "PackageTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "ImageUtils.h"
#include "Graph/MultiPacker.h"

#define LOCTEXT_NAMESPACE "MultiPacker"

UMultiPackerFactory::UMultiPackerFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMultiPacker::StaticClass();
}

UMultiPackerFactory::~UMultiPackerFactory()
{
}

UObject* UMultiPackerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UMultiPacker* Result = NewObject<UMultiPacker>(InParent, Class, Name, Flags | RF_Transactional);//Flags
	Result->TargetName = *Name.ToString();
	Result->Flags = Flags;
	Result->GetOutermost()->SetPackageFlags(PKG_EditorOnly);
	return Result;
}

FText UMultiPackerFactory::GetDisplayName() const
{
	return LOCTEXT("UMultiPackerFactory", "MultiPacker Graph");
}

#undef LOCTEXT_NAMESPACE