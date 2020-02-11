/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "ToolbagUtils/Structs.h"
#include <string>

namespace mset
{
	class MatField
	{
	public:
		MatField(const char* name, const char* parameterName, UMaterial* Material)
		{
			mName = name;
			mParameterName = parameterName;
			mExpression = NULL;
			mMaterial = Material;
		}

		virtual ~MatField() {}

		virtual void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) = 0;

		const char* getName()
		{
			return mName.c_str();
		}

		virtual UMaterialExpression* generateExpression()
		{
			return mExpression;
		}

		virtual void connectTo(const FString& inputName, FExpressionInput* input)
		{
			if(generateExpression() != NULL && inputName.Equals(FString(getName())) )
				input->Expression = mExpression;
		}

	protected:
		std::string mName;
		std::string mParameterName;
		UMaterial* mMaterial;
		UMaterialExpression* mExpression;
	};
}
