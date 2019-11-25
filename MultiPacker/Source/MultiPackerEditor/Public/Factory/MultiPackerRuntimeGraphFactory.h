/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "MultiPackerRuntimeGraphFactory.generated.h"

UCLASS()
class MULTIPACKEREDITOR_API UMultiPackerRuntimeGraphFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMultiPackerRuntimeGraphFactory();
	virtual ~UMultiPackerRuntimeGraphFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	FText GetDisplayName() const override;
};
