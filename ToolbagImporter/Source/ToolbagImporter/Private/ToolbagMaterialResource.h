/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"

class UToolbagImportUI;

namespace mset{
	class StringParse;
	class Serializer;
}
namespace ToolbagUtils{
	struct Material;
}

class ToolbagMaterialResource
{
public:
	static UMaterial* Read( ToolbagUtils::Material* ToolbagMaterial, FString TargetPath, UToolbagImportUI* ImportUI, FString& OutKey );
protected:
	static bool ParseString(ToolbagUtils::Material* ToolbagMaterial, UMaterial* Material, UToolbagImportUI* ImportUI);

	static void FormatMaterial(UMaterial* Material);
	static int32 GetRequiredHeightRecursive(UMaterialExpression* Expression, TArray<UMaterialExpression*>& AlreadyTouchedExpressions);
	static void FormatRecursive(UMaterialExpression* Expression, int32 PosX, int32 PosY, TArray<UMaterialExpression*>& AlreadyTouchedExpressions2);
	static void StartFormatRecursive(TArray<FExpressionInput*>& MaterialInputs, UMaterial* Material);
};
