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

class	MatFieldFloat : public MatField
{
public:
	MatFieldFloat(const char* name, const char* parameterName, UMaterial* Material) : MatField(name, parameterName, Material), mValue(0)
	{}
	void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) override
	{
		ToolbagUtils::MatFieldFloat* m = ToolbagUtils::GetMatFieldFloat(subroutine, mName.c_str());
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

	float	getValue( void ) const
	{
		return mValue;
	}

protected:
	float	mValue;
};

}
