/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRDisplacementHeight.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USRDisplacementHeight::USRDisplacementHeight( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRDisplacementHeight"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}};

const char* USRDisplacementHeight::GetName ()
{
	return "SRDisplacementHeight";
}

void USRDisplacementHeight::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture("Displacement Map", "Displacement Map", Material, ImportUI));
	MatFields.Add(new mset::MatFieldFloat("Scale", "Displacement Scale", Material));
	MatFields.Add(new mset::MatFieldFloat("Scale Center", "Displacement Scale Center", Material));
	MatFields.Add(new mset::MatFieldBool("Relative Scale", "Relative Scale Displacement", Material));
}
void USRDisplacementHeight::GenerateExpressions( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	UMaterialFunctionSubroutine::GenerateExpressions( ImportUI, Material );
	if(Material->D3D11TessellationMode == EMaterialTessellationMode::MTM_NoTessellation)
	{
		Material->WorldPositionOffset.Expression = FunctionCallExpression;
	}
	else
	{
		Material->WorldDisplacement.Expression = FunctionCallExpression;
	}
}