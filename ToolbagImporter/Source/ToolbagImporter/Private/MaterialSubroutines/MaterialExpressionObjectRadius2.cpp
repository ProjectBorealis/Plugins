/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "MaterialExpressionObjectRadius2.h"
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialCompiler.h"

UMaterialExpressionObjectRadius2::UMaterialExpressionObjectRadius2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShaderInputData = true;
}

int32 UMaterialExpressionObjectRadius2::Compile(class FMaterialCompiler* Compiler, int32 OutputIndex)
{
	if (Material && Material->MaterialDomain == MD_DeferredDecal)
	{
		return CompilerError(Compiler, TEXT("Expression not available in the deferred decal material domain."));
	}

	return Compiler->ObjectRadius();
}

void UMaterialExpressionObjectRadius2::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("Object Radius"));
}