// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: ReimportSubstanceImageInputFactory.h

#pragma once
#include "SubstanceImageInputFactory.h"
#include "Factories/Factory.h"
#include "ReimportSubstanceImageInputFactory.generated.h"

class USubstanceImageInput;

UCLASS(hideCategories = Object)
class UReimportSubstanceImageInputFactory : public USubstanceImageInputFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	TArray<FString> ReimportPaths;

	/** FReimportHandler interface - checks to see if this asset can be reimported */
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;

	/** FReimportHandler interface - Sets the path to the file to reimport from */
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;

	/** FReimportHandler interface - Reimports the Obj */
	virtual EReimportResult::Type Reimport(UObject* Obj) override;

	/** Suppresses the dialog box that, when importing over an existing texture, asks if the users wishes to overwrite its settings. */
	static void SuppressImportOverwriteDialog();

private:
	/** This variable is static because in StaticImportObject() the type of the factory is not known. */
	static bool bSuppressImportOverwriteDialog;
};
