/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "Structs.h"
#include <cstring>

namespace ToolbagUtils
{
inline MatField* GetMatField(const MaterialSubroutine* subroutine, const char* name)
{
	for(unsigned i = 0; i < subroutine->MatFieldsCount; ++i)
	{
		if(strcmp(subroutine->MatFields[i]->name, name) == 0)
		{
			return subroutine->MatFields[i];
		}
	}
	return 0;
}

inline MatFieldBool* GetMatFieldBool(const MaterialSubroutine* subroutine, const char* name)
{
	MatField* matField = GetMatField(subroutine, name);
	if(matField == 0 ||matField->type != MATFIELD_BOOL)
		return 0;
	return (MatFieldBool*)matField;
}

inline MatFieldFloat* GetMatFieldFloat(const MaterialSubroutine* subroutine, const char* name)
{
	MatField* matField = GetMatField(subroutine, name);
	if(matField == 0 ||matField->type != MATFIELD_FLOAT)
		return 0;
	return (MatFieldFloat*)matField;
}

inline MatFieldColor* GetMatFieldColor(const MaterialSubroutine* subroutine, const char* name)
{
	MatField* matField = GetMatField(subroutine, name);
	if(matField == 0 ||matField->type != MATFIELD_COLOR)
		return 0;
	return (MatFieldColor*)matField;
}

inline MatFieldEnum* GetMatFieldEnum(const MaterialSubroutine* subroutine, const char* name)
{
	MatField* matField = GetMatField(subroutine, name);
	if(matField == 0 ||matField->type != MATFIELD_ENUM)
		return 0;
	return (MatFieldEnum*)matField;
}

inline MatFieldInt* GetMatFieldInt(const MaterialSubroutine* subroutine, const char* name)
{
	MatField* matField = GetMatField(subroutine, name);
	if(matField == 0 ||matField->type != MATFIELD_INT)
		return 0;
	return (MatFieldInt*)matField;
}

inline MatFieldTexture* GetMatFieldTexture(const MaterialSubroutine* subroutine, const char* name)
{
	MatField* matField = GetMatField(subroutine, name);
	if(matField == 0 ||matField->type != MATFIELD_TEXTURE)
		return 0;
	return (MatFieldTexture*)matField;
}
}