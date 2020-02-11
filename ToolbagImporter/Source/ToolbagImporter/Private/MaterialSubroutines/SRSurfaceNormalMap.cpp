/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRSurfaceNormalMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USRSurfaceNormalMap::USRSurfaceNormalMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRSurfaceNormalMap"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}};

const char* USRSurfaceNormalMap::GetName ()
{
	return "SRSurfaceNormalMap";
}

void USRSurfaceNormalMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture("Normal Map", "Normal Map", Material, ImportUI, false, true));
	MatFields.Add(new mset::MatFieldBool("Flip X", "Normal Flip R", Material));
	MatFields.Add(new mset::MatFieldBool("Flip Y", "Normal Flip G", Material));
	MatFields.Add(new mset::MatFieldBool("Flip Z", "Normal Flip B", Material));
}

bool USRSurfaceNormalMap::TryDirectImport ( UMaterial* Material )
{
	if (((mset::MatFieldTexture*)GetField( "Normal Map" ))->getTexture() == NULL)
		return true;
	if (((mset::MatFieldBool*)GetField("Flip X"))->getValue() == false &&
		((mset::MatFieldBool*)GetField("Flip Y"))->getValue() == true &&
		((mset::MatFieldBool*)GetField("Flip Z"))->getValue() == false)
	{
		if (((mset::MatFieldTexture*)GetField("Normal Map"))->getTexture() != NULL)
		{
			Material->Normal.Expression = ((mset::MatFieldTexture*)GetField("Normal Map"))->generateExpression();
		}
		return true;
	}
	return false;
}