/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include <Factories/Factory.h>
#include "MultiPackerLayerDatabaseFactory.generated.h"

UCLASS()
class MULTIPACKEREDITOR_API UMultiPackerLayerDatabaseFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
};

