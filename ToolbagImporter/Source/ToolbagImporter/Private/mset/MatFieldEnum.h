/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MatField.h"
#include "ToolbagUtils/Helpers.h"
#include <Materials/MaterialExpressionScalarParameter.h>

namespace mset
{

class	MatFieldEnum : public MatField
{
public:
	MatFieldEnum(const char* name, const char* parameterName, UMaterial* Material) : MatField(name, parameterName, Material), mValue(0)
	{}
	void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) override
	{
		ToolbagUtils::MatFieldEnum* m = ToolbagUtils::GetMatFieldEnum(subroutine, mName.c_str());
		if(m != 0)
			mValue = m->value;
	}

	UMaterialExpression* generateExpression() override
	{
		if(mExpression != NULL)
			return mExpression;
		UMaterialExpressionScalarParameter* Expression = NewObject<UMaterialExpressionScalarParameter>(mMaterial);
		Expression->DefaultValue = mValue;
		Expression->ParameterName = FName(mParameterName.c_str());
		mMaterial->Expressions.Add( Expression );
		mExpression = Expression;
		return mExpression;
	}

	int		getValue( void ) const
	{
		return mValue;
	}

protected:
	int	mValue;
};

}
