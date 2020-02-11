/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagMaterialImportFactory.generated.h"


class UToolbagImportUI;

namespace ToolbagUtils
{
	struct SceneObject;
	struct Scene;
}

UCLASS(hidecategories=Object)
class UToolbagMaterialImportFactory : public UFactory
{
	GENERATED_BODY()
public:

	UToolbagMaterialImportFactory( const FObjectInitializer& ObjectInitializer );
	virtual FText GetDisplayName() const override;
	virtual void PostInitProperties() override;
	UObject* FactoryCreateBinary( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled ) override;
protected:
	void ShowImportOptions( UToolbagImportUI* ImportUI, const FString& SourcePath, const FString& TargetPath);
private:
	UToolbagImportUI* ImportUI;
	FToolbagImporterModule* ToolbagImporterModule;
	bool bCanceled;
	bool bShowDialog;
};