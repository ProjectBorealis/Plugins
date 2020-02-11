/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SRReflectivityMetalnessMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"

USRReflectivityMetalnessMap::USRReflectivityMetalnessMap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SRReflectivityMetalnessMap"));

	if (Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USRReflectivityMetalnessMap::GetName()
{
	return "SRReflectivityMetalnessMap";
}

void USRReflectivityMetalnessMap::SetupFields(UToolbagImportUI* ImportUI, UMaterial* Material)
{
	MatFields.Add(new mset::MatFieldTexture("Metalness Map", "Metalness Map", Material, ImportUI, false));
	MatFields.Add(new mset::MatFieldEnum("Channel", "Metalness Channel", Material));
	MatFields.Add(new mset::MatFieldFloat("Metalness", "Metalness", Material));
	MatFields.Add(new mset::MatFieldBool("Invert", "Invert Metalness", Material));
}

bool USRReflectivityMetalnessMap::TryDirectImport(UMaterial* Material)
{
	if (((mset::MatFieldBool*)GetField("Invert"))->getValue() == false)
	{
		if (((mset::MatFieldTexture*)GetField("Metalness Map"))->getTexture() != NULL && ((mset::MatFieldFloat*)GetField("Metalness"))->getValue() == 1 )
		{
			int32 Channel = ((mset::MatFieldEnum*)GetField("Channel"))->getValue();
			Material->Metallic.Connect(Channel < 4 ? Channel + 1 : 0, ((mset::MatFieldTexture*)GetField("Metalness Map"))->generateExpression());
			return true;
		}
		if(((mset::MatFieldTexture*)GetField("Metalness Map"))->getTexture() == NULL)
		{
			Material->Metallic.Expression = ((mset::MatFieldFloat*)GetField("Metalness"))->generateExpression();
			return true;
		}
	}
	return false;
}
