/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialFunctionSubroutine.generated.h"

class UToolbagImportUI;

namespace mset {
	class MatField;
	class StringParse;
}

namespace ToolbagUtils
{
	struct MaterialSubroutine;
}

UCLASS()
class UMaterialFunctionSubroutine : public UObject
{
	GENERATED_BODY()
public:

	UMaterialFunctionSubroutine(const FObjectInitializer& ObjectInitializer);
	static FString CreateNiceName(FString Name);

	virtual void Cleanup ();
	class UMaterialFunction* MaterialFunction;

	virtual void SetupMaterial(UMaterial* Material)
	{};
	virtual void SetupFields( UToolbagImportUI* ImportUI, UMaterial* Material )
	{};
	virtual void ParseFields( ToolbagUtils::MaterialSubroutine* toolbagSubroutine, UMaterial* Material );
	virtual void GenerateExpressions(UToolbagImportUI* ImportUI, UMaterial* Material);
	virtual void ConnectMatFields(UMaterial* Material);
	virtual void LateSetup(UMaterial* Material)
	{};

	virtual const int GetPrio() const
	{ return 0; };
	static bool SortCompare(const UMaterialFunctionSubroutine *lhs, const UMaterialFunctionSubroutine *rhs)
	{ return lhs->GetPrio() < rhs->GetPrio(); };

	mset::MatField* GetField( const char* name );

protected:
	virtual void ConnectOutput( FString OutputName, UMaterialExpression* Expression, int32 Index, UMaterial* Material );
	virtual bool TryDirectImport(UMaterial* Material)
	{ return false; };
	bool WasImportedDirectly;
	TArray<mset::MatField*> MatFields;
	UMaterialExpressionMaterialFunctionCall* FunctionCallExpression;
};