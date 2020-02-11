/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "MaterialExpressionVertexNormalWS2.h"
#include "ToolbagImporterPrivatePCH.h"
#include "MaterialCompiler.h"



UMaterialExpressionVertexNormalWS2::UMaterialExpressionVertexNormalWS2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShaderInputData = true;
}

int32 UMaterialExpressionVertexNormalWS2::Compile(class FMaterialCompiler* Compiler, int32 OutputIndex)
{
	return Compiler->VertexNormal();
}

void UMaterialExpressionVertexNormalWS2::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("VertexNormalWS"));
}