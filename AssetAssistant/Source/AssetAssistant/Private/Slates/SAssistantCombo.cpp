// Copyright 2017 Tefel. All Rights Reserved.
#include "Slates/SAssistantCombo.h"

#define LOCTEXT_NAMESPACE "SAssistantCombo"

void SAssistantCombo::Initialize(const TArray<FString> OptionNames)
{
	Options.Empty();
	if (OptionNames.Num() > 0)
	{
		for (auto Name : OptionNames)
		{
			Options.Add(MakeShareable(new FString(Name)));
		}
		
		CurrentItem = Options[0];
		Construct(FArguments());
	}
	else
	{
		CurrentItem = MakeShareable(new FString("<<Empty>>"));
	}
}

TSharedRef<SWidget> SAssistantCombo::MakeWidgetForOption(FComboItemType InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void SAssistantCombo::OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type)
{
	CurrentItem = NewValue;
}

FText SAssistantCombo::GetCurrentItemLabel() const
{
	if (CurrentItem.IsValid())
	{
		return FText::FromString(*CurrentItem);
	}
	
	return (Options.Num()> 0) ?  LOCTEXT("InvalidComboEntryText", "<<Invalid option>>") : LOCTEXT("InvalidComboEntryText", "<<Empty>>");
}

#undef LOCTEXT_NAMESPACE