// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "Styling/SlateTypes.h"

class FAssetAssistantStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static TSharedPtr<class ISlateStyle> Get();
	static FName GetStyleSetName();

private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:
	static TSharedPtr <class FSlateStyleSet> StyleSet;
	FButtonStyle Button;
};
