/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRSubdivisionPN.h"
#include "ToolbagImporterPrivatePCH.h"


USRSubdivisionPN::USRSubdivisionPN( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRSubdivision"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRSubdivisionPN::GetName ()
{
	return "SRSubdivisionPN";
}

void USRSubdivisionPN::SetupMaterial( UMaterial* Material )
{
	Material->D3D11TessellationMode = EMaterialTessellationMode::MTM_PNTriangles;
	Material->bEnableAdaptiveTessellation = false;
}