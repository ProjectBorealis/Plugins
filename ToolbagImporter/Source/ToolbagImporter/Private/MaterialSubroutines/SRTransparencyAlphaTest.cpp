/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRTransparencyAlphaTest.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include <Materials/MaterialExpressionFunctionOutput.h>


USRTransparencyAlphaTest::USRTransparencyAlphaTest( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRTransparency"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRTransparencyAlphaTest::GetName ()
{
	return "SRTransparencyAlphaTest";
}

void USRTransparencyAlphaTest::SetupMaterial ( UMaterial* Material )
{
	USRTransparency::SetupMaterial( Material );
	Material->BlendMode = EBlendMode::BLEND_Masked;
}

void USRTransparencyAlphaTest::LateSetup ( UMaterial* Material )
{
	USRTransparency::LateSetup( Material );
	for (int i = 0; i < FunctionCallExpression->Outputs.Num(); ++i)
	{
		FFunctionExpressionOutput* Output = &(FunctionCallExpression->FunctionOutputs[i]);
		if(Output->ExpressionOutput->OutputName.ToString().Equals(TEXT("AlphaOut")))
		{
			Material->OpacityMask.Connect(i, FunctionCallExpression);
		}
	}
}
