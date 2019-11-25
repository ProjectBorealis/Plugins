/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "MultiPackerFactory.generated.h"

UCLASS()
class MULTIPACKEREDITOR_API UMultiPackerFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMultiPackerFactory();
	virtual ~UMultiPackerFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	FText GetDisplayName() const override;
};
