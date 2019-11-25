// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SourceJSONObject.cpp
#include "SourceJSONObject.h"
#include "SubstanceSourcePrivatePCH.h"

#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"

//==================================================
SourceJSONArray::SourceJSONArray(const TArray<TSharedPtr<FJsonValue> >& arr)
	: mArray(arr)
{
}

int SourceJSONArray::getElementCount() const
{
	return mArray.Num();
}

bool SourceJSONArray::hasElementOfType(int index, Alg::Source::JSONType type) const
{
	check(index >= 0 && index < mArray.Num());

	switch (type)
	{
	case Alg::Source::JSONType::Array:
		return (mArray[index]->Type == EJson::Array);
	case Alg::Source::JSONType::Boolean:
		return (mArray[index]->Type == EJson::Boolean);
	case Alg::Source::JSONType::Null:
		return (mArray[index]->Type == EJson::Null);
	case Alg::Source::JSONType::Number:
		return (mArray[index]->Type == EJson::Number);
	case Alg::Source::JSONType::Object:
		return (mArray[index]->Type == EJson::Object);
	case Alg::Source::JSONType::String:
		return (mArray[index]->Type == EJson::String);
	default:
		return false;
	}
}

Alg::Source::IJSONArrayPtr SourceJSONArray::getArrayElementAt(int index) const
{
	check(index >= 0 && index < mArray.Num());
	return Alg::Source::MakeShared<SourceJSONArray>(mArray[index]->AsArray());
}

double SourceJSONArray::getNumberElementAt(int index) const
{
	check(index >= 0 && index < mArray.Num());
	return mArray[index]->AsNumber();
}

Alg::Source::IJSONObjectPtr SourceJSONArray::getObjectElementAt(int index) const
{
	check(index >= 0 && index < mArray.Num());
	return Alg::Source::MakeShared<SourceJSONObject>(mArray[index]->AsObject());
}

Alg::Source::String SourceJSONArray::getStringElementAt(int index) const
{
	check(index >= 0 && index < mArray.Num());
	return TCHAR_TO_UTF8(*mArray[index]->AsString());
}

//==================================================
SourceJSONObject::SourceJSONObject(const Alg::Source::String& json)
{
	TSharedRef< TJsonReader<> > JSONReader = TJsonReaderFactory<>::Create(json.c_str());
	FJsonSerializer::Deserialize(JSONReader, mJsonObject);
}

SourceJSONObject::SourceJSONObject(TSharedPtr<FJsonObject> object)
	: mJsonObject(object)
{
}

bool SourceJSONObject::hasFieldOfType(const Alg::Source::String& name, Alg::Source::JSONType type) const
{
	check(mJsonObject.IsValid());

	switch (type)
	{
	case Alg::Source::JSONType::Array:
		return mJsonObject->HasTypedField<EJson::Array>(name.c_str());
	case Alg::Source::JSONType::Boolean:
		return mJsonObject->HasTypedField<EJson::Boolean>(name.c_str());
	case Alg::Source::JSONType::Null:
		return mJsonObject->HasTypedField<EJson::Null>(name.c_str());
	case Alg::Source::JSONType::Number:
		return mJsonObject->HasTypedField<EJson::Number>(name.c_str());
	case Alg::Source::JSONType::Object:
		return mJsonObject->HasTypedField<EJson::Object>(name.c_str());
	case Alg::Source::JSONType::String:
		return mJsonObject->HasTypedField<EJson::String>(name.c_str());
	default:
		return false;
	}
}

Alg::Source::IJSONArrayPtr SourceJSONObject::getArrayValue(const Alg::Source::String& name) const
{
	check(mJsonObject.IsValid());
	return Alg::Source::MakeShared<SourceJSONArray>(mJsonObject->GetArrayField(name.c_str()));
}

bool SourceJSONObject::getBoolValue(const Alg::Source::String& name) const
{
	check(mJsonObject.IsValid());
	return mJsonObject->GetBoolField(name.c_str());
}

double SourceJSONObject::getNumberValue(const Alg::Source::String& name) const
{
	check(mJsonObject.IsValid());
	return mJsonObject->GetNumberField(name.c_str());
}

Alg::Source::IJSONObjectPtr SourceJSONObject::getObjectValue(const Alg::Source::String& name) const
{
	check(mJsonObject.IsValid());
	return Alg::Source::MakeShared<SourceJSONObject>(mJsonObject->GetObjectField(name.c_str()));
}

Alg::Source::String SourceJSONObject::getStringValue(const Alg::Source::String& name) const
{
	check(mJsonObject.IsValid());
	return TCHAR_TO_UTF8(*mJsonObject->GetStringField(name.c_str()));
}

void SourceJSONObject::setBoolValue(const Alg::Source::String& name, bool value)
{
	check(mJsonObject.IsValid());
	mJsonObject->SetBoolField(name.c_str(), value);
}

void SourceJSONObject::setNumberValue(const Alg::Source::String& name, double value)
{
	check(mJsonObject.IsValid());
	mJsonObject->SetNumberField(name.c_str(), value);
}

void SourceJSONObject::setStringValue(const Alg::Source::String& name, const Alg::Source::String& value)
{
	check(mJsonObject.IsValid());
	mJsonObject->SetStringField(name.c_str(), value.c_str());
}

Alg::Source::String SourceJSONObject::asString() const
{
	check(mJsonObject.IsValid());
	FString JsonString;
	auto writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR> >::Create(&JsonString);
	FJsonSerializer::Serialize(mJsonObject.ToSharedRef(), writer);
	writer->Close();
	return TCHAR_TO_UTF8(*JsonString);
}
