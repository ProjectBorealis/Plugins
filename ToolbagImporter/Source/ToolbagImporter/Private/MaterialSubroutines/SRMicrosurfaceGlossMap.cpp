/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRMicrosurfaceGlossMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldTexture.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldBool.h"
#include "../UI/ToolbagImportUI.h"


USRMicrosurfaceGlossMap::USRMicrosurfaceGlossMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRMicrosurfaceGloss"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRMicrosurfaceGlossMap::GetName ()
{
	return "SRMicrosurfaceGlossMap";
}

void USRMicrosurfaceGlossMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture("Gloss Map", "Gloss Map", Material, ImportUI));
	MatFields.Add(new mset::MatFieldEnum("Channel", "Gloss Channel", Material));
	MatFields.Add(new mset::MatFieldFloat("Gloss", "Glossiness", Material));
	MatFields.Add(new mset::MatFieldBool("Invert", "Invert Gloss", Material));
}

bool USRMicrosurfaceGlossMap::TryDirectImport ( UMaterial* Material )
{
	if (((mset::MatFieldBool*)GetField("Invert"))->getValue() == true)
	{
		if (((mset::MatFieldTexture*)GetField("Gloss Map"))->getTexture() != NULL && ((mset::MatFieldFloat*)GetField("Gloss"))->getValue() == 1)
		{
			Material->Roughness.Connect(((mset::MatFieldEnum*)GetField("Channel"))->getValue()+1, ((mset::MatFieldTexture*)GetField("Gloss Map"))->generateExpression());
			return true;
		}
		if (((mset::MatFieldTexture*)GetField("Gloss Map"))->getTexture() == NULL)
		{
			Material->Roughness.Expression = ((mset::MatFieldFloat*)GetField("Gloss"))->generateExpression();
			return true;
		}
	}
	return false;
}