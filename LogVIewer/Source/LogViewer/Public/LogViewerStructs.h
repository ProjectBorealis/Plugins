// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Serialization/JsonSerializerMacros.h"

class FParsedLogLine
{
public:
	FParsedLogLine() : LogMessage(), LogCategory(), VerbosityLevel() {};
	FParsedLogLine(FString UncategorizedMessage) : LogMessage(UncategorizedMessage), LogCategory(TEXT("NoCategory")), VerbosityLevel(ELogVerbosity::Log) {};
	FString LogMessage;
	FString LogCategory;
	ELogVerbosity::Type VerbosityLevel;
};

class FLogColorTable
{
public:

	static FLinearColor GenerateRandomColor()
	{
		return FLinearColor(FMath::FRandRange(0.15f, 0.7f), FMath::FRandRange(0.15f, 1.f), FMath::FRandRange(0.15f, 1.f));
	}
	void RegisterCategory(FName Category)
	{
		ColorMap.Add({ Category, GenerateRandomColor() });
	}

	FLinearColor GetColorForCategory(FName Category) const
	{
		auto* Color = ColorMap.Find(Category);
		if (!Color)
		{
			FLinearColor(0.8f, 0.8f, 0.8f); //In case can't find color for category, use some greyish
		}
		return *Color;
		//return FLinearColor(FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f));
	}

	void SetColorForCategory(FName Category, const FLinearColor& InColor)
	{
		if (!ColorMap.Contains(Category))
		{
			return;
		}
		ColorMap[Category] = FLinearColor(InColor.R, InColor.G, InColor.B, 1.f); //Override whatever alpha we receive, because color picker returns 0.f for example
	}

private:
	TMap<FName, FLinearColor> ColorMap;

};

class FLVCategoriesSaveItem : public FJsonSerializable
{
public:
	FLVCategoriesSaveItem()
		: CategoryName()
		, bIsActive(true)
		, CategoryHexColor()
	{}

	FLVCategoriesSaveItem(const FString& InCategoryName, bool bInIsActive, const FString& InCategoryHexColor)
		: CategoryName(InCategoryName)
		, bIsActive(bInIsActive)
		, CategoryHexColor(InCategoryHexColor)
	{}

	virtual ~FLVCategoriesSaveItem() {}

	FString CategoryName;
	bool bIsActive;
	FString CategoryHexColor;

	// FJsonSerializable
	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE("CategoryName", CategoryName);
		JSON_SERIALIZE("IsActive", bIsActive);
		JSON_SERIALIZE("ColorHex", CategoryHexColor);
	END_JSON_SERIALIZER
};

class FLVCategoriesSave : public FJsonSerializable
{
public:
	FLVCategoriesSave()
	{}
	virtual ~FLVCategoriesSave() {}

	TArray< FLVCategoriesSaveItem > Categories;

	// FJsonSerializable
	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE_ARRAY_SERIALIZABLE("Categories", Categories, FLVCategoriesSaveItem);
	END_JSON_SERIALIZER
};

struct FLogMessage
{

	TSharedRef<FString> Message;
	ELogVerbosity::Type Verbosity;
	FName Category;
	FName Style;

	FLogMessage(const TSharedRef<FString>& NewMessage, FName NewCategory, FName NewStyle = NAME_None)
		: Message(NewMessage)
		, Verbosity(ELogVerbosity::Log)
		, Category(NewCategory)
		, Style(NewStyle)
	{
	}

	FLogMessage(const TSharedRef<FString>& NewMessage, ELogVerbosity::Type NewVerbosity, FName NewCategory, FName NewStyle = NAME_None)
		: Message(NewMessage)
		, Verbosity(NewVerbosity)
		, Category(NewCategory)
		, Style(NewStyle)
	{
	}
};
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLogMessageReceivedDelegate, const TCHAR* /*Message*/, ELogVerbosity::Type /*Verbosity*/, const class FName& /*Category*/);
