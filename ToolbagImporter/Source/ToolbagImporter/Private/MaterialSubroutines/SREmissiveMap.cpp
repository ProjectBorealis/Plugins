/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "SREmissiveMap.h"
#include "ToolbagImporterPrivatePCH.h"
#include "mset/MatFieldColor.h"
#include "mset/MatFieldFloat.h"
#include "mset/MatFieldEnum.h"
#include "mset/MatFieldBool.h"
#include "mset/MatFieldTexture.h"
#include "../UI/ToolbagImportUI.h"


USREmissiveMap::USREmissiveMap( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	static ConstructorHelpers::FObjectFinder<UMaterialFunction> Material(TEXT("/ToolbagImporter/MaterialSubroutines/SREmissiveMap"));

	if(Material.Object != NULL)
	{
		MaterialFunction = (UMaterialFunction*)Material.Object;
	}
};

const char* USREmissiveMap::GetName ()
{
	return "SREmissiveMap";
}

void USREmissiveMap::SetupFields ( UToolbagImportUI* ImportUI, UMaterial* Material )
{
	MatFields.Add(new mset::MatFieldTexture( "Emissive Map", "Emissive Map", Material, ImportUI, false ));
	MatFields.Add(new mset::MatFieldColor( "Color", "Emissive Color", Material ));
	MatFields.Add(new mset::MatFieldFloat( "Intensity", "Emissive Intensity", Material ));
	MatFields.Add(new mset::MatFieldEnum( "UV Set", "Emissive UV Set", Material ));
	MatFields.Add(new mset::MatFieldColor( "Glow", "Emissive Glow", Material ));
}

bool USREmissiveMap::TryDirectImport ( UMaterial* Material )
{
	if (((mset::MatFieldFloat*)GetField("Intensity"))->getValue() == 1 &&
		((mset::MatFieldEnum*)GetField("UV Set"))->getValue() == 0 &&
		((mset::MatFieldColor*)GetField("Glow"))->getValue() == FLinearColor::Black)
	{
		if (((mset::MatFieldTexture*)GetField("Emissive Map"))->getTexture() != NULL && ((mset::MatFieldColor*)GetField("Color"))->getValue() == FLinearColor::White)
		{
			Material->EmissiveColor.Expression = ((mset::MatFieldTexture*)GetField("Emissive Map"))->generateExpression();
			return true;
		}
		if (((mset::MatFieldTexture*)GetField( "Emissive Map" ))->getTexture() == NULL && ((mset::MatFieldColor*)GetField( "Color" ))->getValue() != FLinearColor::Black)
		{
			Material->EmissiveColor.Expression = ((mset::MatFieldColor*)GetField( "Color" ))->generateExpression();
			return true;
		}
	}
	return false;
}