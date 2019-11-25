// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SourceJSONObject.h
#pragma once

#include <substance/source/ijsonobject.h>
#include "Templates/SharedPointer.h"
#include "Dom/JsonValue.h"

#include "Serialization/JsonReader.h"

/**/
class SourceJSONArray : public Alg::Source::IJSONArray
{
public:
	SourceJSONArray(const TArray<TSharedPtr<FJsonValue> >& arr);

	//From Alg::Source::IJSONArray
	virtual int getElementCount() const override;
	virtual bool hasElementOfType(int index, Alg::Source::JSONType type) const override;
	virtual Alg::Source::IJSONArrayPtr getArrayElementAt(int index) const override;
	virtual double getNumberElementAt(int index) const override;
	virtual Alg::Source::IJSONObjectPtr getObjectElementAt(int index) const override;
	virtual Alg::Source::String getStringElementAt(int index) const override;

private:
	TArray<TSharedPtr<FJsonValue> >				mArray;
};

/**/
class SourceJSONObject : public Alg::Source::IJSONObject
{
public:
	SourceJSONObject(const Alg::Source::String& json);
	SourceJSONObject(TSharedPtr<FJsonObject> object);

	//From Alg::Source::IJSONObject
	virtual bool hasFieldOfType(const Alg::Source::String& name, Alg::Source::JSONType type) const override;
	virtual Alg::Source::IJSONArrayPtr getArrayValue(const Alg::Source::String& name) const override;
	virtual bool getBoolValue(const Alg::Source::String& name) const override;
	virtual double getNumberValue(const Alg::Source::String& name) const override;
	virtual Alg::Source::IJSONObjectPtr getObjectValue(const Alg::Source::String& name) const override;
	virtual Alg::Source::String getStringValue(const Alg::Source::String& name) const override;
	virtual void setBoolValue(const Alg::Source::String& name, bool value) override;
	virtual void setNumberValue(const Alg::Source::String& name, double value) override;
	virtual void setStringValue(const Alg::Source::String& name, const Alg::Source::String& value) override;
	virtual Alg::Source::String asString() const override;

private:
	TSharedPtr<FJsonObject>			mJsonObject;
};
