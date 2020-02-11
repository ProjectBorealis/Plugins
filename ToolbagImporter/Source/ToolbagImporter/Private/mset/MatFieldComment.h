/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MatField.h"
#include "ToolbagUtils/Helpers.h"

namespace mset
{

class	MatFieldComment : public MatField
{
public:
	MatFieldComment(const char* name, const char* parameterName, UMaterial* Material) : MatField(name, parameterName, Material)
	{}
	void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) override
	{
	}

protected:
};

}
