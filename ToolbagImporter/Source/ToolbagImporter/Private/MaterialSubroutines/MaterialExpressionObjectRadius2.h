/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionObjectRadius2.generated.h"

UCLASS(collapsecategories, hidecategories=Object)
class UMaterialExpressionObjectRadius2 : public UMaterialExpression
{
	GENERATED_BODY()
public:
	UMaterialExpressionObjectRadius2( const FObjectInitializer& ObjectInitializer );


	//~ Begin UMaterialExpression Interface
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex ) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	//~ End UMaterialExpression Interface
};



