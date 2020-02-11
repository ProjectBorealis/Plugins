/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"
#include <string>
#include <vector>
#include "mset/MatField.h"

class MaterialSubroutine
{
public:
	static const char* getName()
	{
		return "MaterialSubroutine";
	}

	void parseFields( mset::StringParse& p )
	{
		std::string token;
		//read the subroutine fields
		while( !p.eof() )
		{
			p.readLine( token, '=' );
			if( 0 == strcmp(token.c_str(), "@End") )
			{
				break;
			}
			else 
			{
				for(auto it = mFields.begin(); it != mFields.end(); ++it)
				{
					if( 0 == strcmp(token.c_str(), (*it)->getName()) )
					{
						//(*it)->readValue( p );
					}
				}
			}
		}
	}

	virtual void preNodeCreation(UMaterial* Material, UPackage* Package)
	{}
	virtual void createMaterialNodes(UMaterial* Material, UPackage* Package)
	{}
	virtual void postNodeCreation(UMaterial* Material, UPackage* Package)
	{}

protected:
	void addField( mset::MatField* matField)
	{
		mFields.push_back( matField );
	}
	std::vector<mset::MatField*> mFields;
};
