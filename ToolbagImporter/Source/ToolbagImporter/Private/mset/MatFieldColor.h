/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MatField.h"
#include "ToolbagUtils/Helpers.h"
#include <Materials/MaterialExpressionVectorParameter.h>

namespace mset
{

class	MatFieldColor : public MatField
{
public:
	MatFieldColor(const char* name, const char* parameterName, UMaterial* Material) : MatField(name, parameterName, Material), mValue(FLinearColor::White)
	{}
	void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) override
	{
		ToolbagUtils::MatFieldColor* m = ToolbagUtils::GetMatFieldColor(subroutine, mName.c_str());
		if(m != 0)
			mValue = FLinearColor(m->value[0], m->value[1], m->value[2]);
	}

	UMaterialExpression* generateExpression() override
	{
		if(mExpression != NULL)
			return mExpression;

		UMaterialExpressionVectorParameter* Expression = NewObject<UMaterialExpressionVectorParameter>(mMaterial);
		Expression->DefaultValue = mValue;
		Expression->ParameterName = FName(mParameterName.c_str());
		mMaterial->Expressions.Add( Expression );
		mExpression = Expression;
		return mExpression;
	}

	const FLinearColor	getValue( void ) const
	{
		return mValue;
	}

	void connectTo(const FString& inputName, FExpressionInput* input) override
	{
		if (generateExpression() != NULL)
		{
			if (inputName.Equals( FString( getName() ) ) )
				input->Expression = mExpression;
			else if (inputName.Equals( FString( getName() ) + " RGB" ) ) 
				input->Connect( 0, mExpression );
			else if (inputName.Equals( FString( getName() ) + " A" ) )
				input->Connect( 4, mExpression );
		}
	}
protected:
	FLinearColor mValue;
};

}
