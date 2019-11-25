/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "Factory/MultiPackerLayerDatabaseFactory.h"
#include "MultiPackerLayerDatabase.h"
#include "PackageTools.h"
#include <Uobject/UObjectGlobals.h>


#define LOCTEXT_NAMESPACE "MultiPacker"

UMultiPackerLayerDatabaseFactory::UMultiPackerLayerDatabaseFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMultiPackerLayerDatabase::StaticClass();
}

UObject* UMultiPackerLayerDatabaseFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UMultiPackerLayerDatabase* NewDataBase = NewObject<UMultiPackerLayerDatabase>(InParent, Class, Name, Flags | RF_Transactional);
	return NewDataBase;
}

FText UMultiPackerLayerDatabaseFactory::GetDisplayName() const
{
	return LOCTEXT("UMultiPackerLayerDatabaseFactory", "LayerDatabase");
}

#undef LOCTEXT_NAMESPACE