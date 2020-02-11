/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionVertexNormalWS2.generated.h"

UCLASS(collapsecategories, hidecategories=Object)
class UMaterialExpressionVertexNormalWS2 : public UMaterialExpression
{
	GENERATED_BODY()
public:

	UMaterialExpressionVertexNormalWS2( const FObjectInitializer& ObjectInitializer );

	//~ Begin UMaterialExpression Interface
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	//~ End UMaterialExpression Interface
};



