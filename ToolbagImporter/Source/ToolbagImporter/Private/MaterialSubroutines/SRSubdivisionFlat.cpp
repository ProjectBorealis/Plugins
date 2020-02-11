/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRSubdivisionFlat.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldTexture.h"
#include "mset/MatFieldFloat.h"
#include "../UI/ToolbagImportUI.h"


USRSubdivisionFlat::USRSubdivisionFlat( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRSubdivision"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRSubdivisionFlat::GetName ()
{
	return "SRSubdivisionFlat";
}

void USRSubdivisionFlat::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldFloat("Tessellation", "Tessellation", Material));
}

void USRSubdivisionFlat::SetupMaterial( UMaterial* Material )
{
	Material->D3D11TessellationMode = EMaterialTessellationMode::MTM_FlatTessellation;
	Material->bEnableAdaptiveTessellation = false;
}

bool USRSubdivisionFlat::TryDirectImport ( UMaterial* Material )
{
	Material->TessellationMultiplier.Expression = ((mset::MatFieldFloat*)GetField("Tessellation"))->generateExpression();
	return true;
}