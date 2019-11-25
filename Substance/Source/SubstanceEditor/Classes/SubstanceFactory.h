// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceFactory.h

#pragma once
#include "SubstanceImportOptionsUi.h"
#include "Factories/Factory.h"
#include "SubstanceFactory.generated.h"

class USubstanceGraphInstance;

UCLASS(hideCategories = Object, customConstructor)
class USubstanceFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	USubstanceFactory(const class FObjectInitializer& PCIP);

	/** Called when an sbsar is loaded for the first time. Creates package */
	virtual UObject* FactoryCreateBinary(UClass*, UObject*, FName, EObjectFlags, UObject*, const TCHAR*,
	                                     const uint8*&, const uint8*, FFeedbackContext*) override;

	/** Suppresses the dialog box that, when importing over an existing texture, asks if the users wishes to overwrite its settings. */
	static void SUBSTANCEEDITOR_API SuppressImportOverwriteDialog();

private:
	/** This variable is static because in StaticImportObject() the type of the factory is not known. */
	static bool bSuppressImportOverwriteDialog;

	/** Handles recreation of the substances graphs after the user reimport */
	void RecreateGraphsPostReimport(class USubstanceInstanceFactory* ParentFactory, UObject* InParent);
};

namespace Substance
{
/** Options we will ask the user when importing a substance factory */
struct FSubstanceImportOptions
{
	bool bCreateMaterial;
	bool bCreateInstance;
	bool bForceCreateInstance;

	FString InstanceName;
	FString MaterialName;

	FString InstanceDestinationPath;
	FString MaterialDestinationPath;
};

/** Get the options from the user for the naming / settings to use for the asset being created */
void GetImportOptions(FString Name, FString ParentName, FSubstanceImportOptions& InOutImportOptions, bool& OutOperationCanceled);

/** Converts Import UI to Import options */
void ApplyImportUIToImportOptions(class USubstanceImportOptionsUi* ImportUI, FSubstanceImportOptions& InOutImportOptions);
}
