//! @file ijsonobject.h
//! @brief Substance Source JSON Object
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Interface for JSON Objects
#ifndef _SUBSTANCE_SOURCE_IJSONOBJECT_H_
#define _SUBSTANCE_SOURCE_IJSONOBJECT_H_
#pragma once

#include <substance/source/source.h>

namespace Alg
{
namespace Source
{

typedef SharedPtr<class IJSONObject> IJSONObjectPtr;
typedef SharedPtr<class IJSONArray> IJSONArrayPtr;

enum class JSONType
{
	Array,
	Boolean,
	Null,
	Number,
	Object,
	String,
	Unknown,
};

/**/
class IJSONArray
{
public:
	virtual ~IJSONArray() = default;

	/** How many elements are in the array */
	virtual int getElementCount() const = 0;

	/** Retrieve property type of field by name */
	virtual bool hasElementOfType(int index, JSONType type) const = 0;

	/** Get the element as an array */
	virtual IJSONArrayPtr getArrayElementAt(int index) const = 0;

	/** Get the element as a number */
	virtual double getNumberElementAt(int index) const = 0;

	/** Get the element as an Object */
	virtual IJSONObjectPtr getObjectElementAt(int index) const = 0;

	/** Get the element as a string */
	virtual String getStringElementAt(int index) const = 0;
};

/**/
class IJSONObject
{
public:
	virtual ~IJSONObject() {}

	/** Retrieve property type of field by name */
	virtual bool hasFieldOfType(const String& name, JSONType type) const = 0;

	/** Retrieve array value */
	virtual IJSONArrayPtr getArrayValue(const String& name) const = 0;

	/** Retrieve bool value */
	virtual bool getBoolValue(const String& name) const = 0;

	/** Retrieve Number value */
	virtual double getNumberValue(const String& name) const = 0;

	/** Retrieve Object value */
	virtual IJSONObjectPtr getObjectValue(const String& name) const = 0;

	/** Retrieve string value */
	virtual String getStringValue(const String& name) const = 0;

	/** Set bool value */
	virtual void setBoolValue(const String& name, bool value) = 0;

	/** Set Number value */
	virtual void setNumberValue(const String& name, double value) = 0;

	/** Set string value */
	virtual void setStringValue(const String& name, const String& value) = 0;

	/** Create JSON string from object */
	virtual String asString() const = 0;
};

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_IJSONOBJECT_H_
