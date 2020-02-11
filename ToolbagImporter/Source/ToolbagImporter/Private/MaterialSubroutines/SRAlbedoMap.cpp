/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRAlbedoMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USRAlbedoMap::USRAlbedoMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRAlbedoMap"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRAlbedoMap::GetName ()
{
	return "SRAlbedoMap";
}

void USRAlbedoMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture("Albedo Map", "Albedo Map", Material, ImportUI, false));
	MatFields.Add(new mset::MatFieldColor("Color", "Albedo Color", Material));
}

bool USRAlbedoMap::TryDirectImport ( UMaterial* Material )
{
	if(((mset::MatFieldTexture*)GetField("Albedo Map"))->getTexture() != NULL && ((mset::MatFieldColor*)GetField("Color"))->getValue() == FLinearColor::White)
	{
		Material->BaseColor.Expression = ((mset::MatFieldTexture*)GetField("Albedo Map"))->generateExpression();
		return true;
	}
	if(((mset::MatFieldTexture*)GetField("Albedo Map"))->getTexture() == NULL )
	{
		Material->BaseColor.Expression = ((mset::MatFieldColor*)GetField("Color"))->generateExpression();
		return true;
	}
	return false;
}
