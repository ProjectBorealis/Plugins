/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagUtils/Structs.h"
#include "ToolbagImportFactory.generated.h"


class UToolbagImportUI;

namespace ToolbagUtils
{
	struct SceneObject;
	struct Scene;
}

UCLASS(hidecategories=Object)
class UToolbagImportFactory : public UFactory
{
	GENERATED_BODY()
public:

	UToolbagImportFactory( const FObjectInitializer& ObjectInitializer );
	virtual FText GetDisplayName() const override;
	virtual void PostInitProperties() override;
	UObject* FactoryCreateBinary( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled ) override;
protected:
	bool ReadScene( ToolbagUtils::Scene* ToolbagScene, UWorld* World);
	bool ReadBlueprint( ToolbagUtils::Scene* ToolbagScene, USCS_Node* Root);
	bool ReadObject( ToolbagUtils::SceneObject* ToolbagObject, USCS_Node* Parent, TMap<FString, UMaterialInterface*>& Materials );
	bool ReadObject( ToolbagUtils::SceneObject* ToolbagObject, AActor* Parent, TMap<FString, UMaterialInterface*>& Materials );
	void ShowImportOptions( UToolbagImportUI* ImportUI, const FString& SourcePath, const FString& TargetPath);
	void ReadMaterials( ToolbagUtils::Scene* ToolbagScene, TMap<FString, UMaterialInterface*>* MaterialMap ) const;
private:
	UToolbagImportUI* ImportUI;
	FToolbagImporterModule* ToolbagImporterModule;
	bool bCanceled;
	bool bShowDialog;
};