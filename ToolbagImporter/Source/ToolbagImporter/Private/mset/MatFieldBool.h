/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MatField.h"
#include "ToolbagUtils/Helpers.h"
#include <Materials/MaterialExpressionStaticBoolParameter.h>

namespace mset
{

class	MatFieldBool : public MatField
{
public:
	MatFieldBool(const char* name, const char* parameterName, UMaterial* Material) : MatField(name, parameterName, Material), mValue(false)
	{}
	void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) override
	{
		ToolbagUtils::MatFieldBool* m = ToolbagUtils::GetMatFieldBool(subroutine, mName.c_str());
		if(m != 0)
			mValue = m->value;
	}

	UMaterialExpression* generateExpression() override
	{
		if(mExpression != NULL)
			return mExpression;

		UMaterialExpressionStaticBoolParameter* Expression = NewObject<UMaterialExpressionStaticBoolParameter>(mMaterial);
		Expression->DefaultValue = mValue;
		Expression->ParameterName = FName(mParameterName.c_str());
		mMaterial->Expressions.Add( Expression );
		mExpression = Expression;
		return mExpression;
	}

	bool			getValue( void ) const
	{
		return mValue;
	}

protected:
	bool	mValue;
};

}
