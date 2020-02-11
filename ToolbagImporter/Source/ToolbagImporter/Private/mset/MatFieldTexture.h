/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include "MatField.h"
#include "ToolbagUtils/Helpers.h"
#include "../TextureLoader.h"
#include <Materials/MaterialExpressionTextureSampleParameter2D.h>
#include "../UI/ToolbagImportUI.h"

namespace mset
{

class	MatFieldTexture : public MatField
{
public:
	MatFieldTexture(const char* name, const char* parameterName, UMaterial* Material, UToolbagImportUI* ImportUI, bool sampleLinear = true, bool isNormal = false) : MatField(name, parameterName, Material)
	{
		mImportUI = ImportUI;
		mTexture = NULL;
		mSampleLinear = sampleLinear;
		mIsNormal = isNormal;
	}

	enum
	{
		GENERATE_MIPMAPS =	(1 << 0),
		SUBSTANCE_OUTPUT =	(1 << 1),
		AUTO_RELOAD =		(1 << 2),
		VERTICAL_FLIP =		(1 << 3),
		PRECONVERT_SRGB =	(1 << 4)
	};

	bool readTexture ( ToolbagUtils::Texture* t )
	{
		mTexture = TextureLoader::CreateTexture( t, GENERATE_MIPMAPS, mImportUI->AbsoluteTexturesPath, mImportUI, mIsNormal );
		return mTexture != NULL;
	}

	void readValue( ToolbagUtils::MaterialSubroutine* subroutine ) override
	{
		ToolbagUtils::MatFieldTexture* m = ToolbagUtils::GetMatFieldTexture(subroutine, mName.c_str());
		if(m != 0)
			readTexture(&m->texture);
	}

	UMaterialExpression* generateExpression() override
	{
		if(mExpression != NULL)
			return mExpression;
		if (mTexture == NULL) return NULL;
		UMaterialExpressionTextureSampleParameter2D* Expression = NewObject<UMaterialExpressionTextureSampleParameter2D>(mMaterial);
		Expression->Texture = mTexture;
		Expression->ParameterName = FName(mParameterName.c_str());
		Expression->Texture = mTexture;
		if(mTexture->Source.GetFormat() == TSF_G8)
		{
			Expression->SamplerType = (mIsNormal ? EMaterialSamplerType::SAMPLERTYPE_Normal : (mTexture->SRGB == 0 ? EMaterialSamplerType::SAMPLERTYPE_LinearGrayscale : EMaterialSamplerType::SAMPLERTYPE_Grayscale));
		}
		else
		{
			Expression->SamplerType = (mIsNormal ? EMaterialSamplerType::SAMPLERTYPE_Normal : (mTexture->SRGB == 0 ? EMaterialSamplerType::SAMPLERTYPE_LinearColor : EMaterialSamplerType::SAMPLERTYPE_Color));
		}
		mMaterial->Expressions.Add( Expression );
		mExpression = Expression;
		return mExpression;
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


	void setTexture( UTexture* t )
	{
		mTexture =  t;
	}

	UTexture*	getTexture( void ) const
	{
		return mTexture;
	}

protected:
	UToolbagImportUI* mImportUI;
	UTexture* 	mTexture;
	bool mIsNormal;
	bool mSampleLinear;
};

}
