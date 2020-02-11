/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SREmissiveHeat.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USREmissiveHeat::USREmissiveHeat( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SREmissiveHeat"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USREmissiveHeat::GetName ()
{
	return "SREmissiveHeat";
}

void USREmissiveHeat::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture("Heat Map", "Heat Map", Material, ImportUI, false));
	MatFields.Add(new mset::MatFieldFloat("Intensity", "Emissive Intensity", Material));
	MatFields.Add(new mset::MatFieldFloat("Temperature (K)", "Temperature (K)", Material));
	MatFields.Add(new mset::MatFieldFloat("Minimum Temperature (K)", "Minimum Temperature (K)", Material));
}
