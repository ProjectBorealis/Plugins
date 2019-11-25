/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "Factory/MultiPackerDatabaseFactory.h"
#include "MultiPackerDataBase.h"
#include "PackageTools.h"

#define LOCTEXT_NAMESPACE "MultiPacker"

UMultiPackerDatabaseFactory::UMultiPackerDatabaseFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UMultiPackerDataBase::StaticClass();
}

UObject* UMultiPackerDatabaseFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UMultiPackerDataBase* NewDataBase = NewObject<UMultiPackerDataBase>(InParent, Class, Name, Flags | RF_Transactional);
	return NewDataBase;
}

#undef LOCTEXT_NAMESPACE