// Copyright 2017 Tefel. All Rights Reserved.
#include "Slates/SAssistantRichCombo.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "SAssistantRichCombo"

void SAssistantRichCombo::Construct(const FArguments& InArgs)
{
	// from names
	if (InArgs._OptionNames.Num() > 0)
	{
		for (auto Name : InArgs._OptionNames)
		{
			Options.Add(MakeShareable(new FString(Name)));
		}
		CurrentItem = Options[0];
	}

	// counts
	if (InArgs._OptionCounts.Num() > 0)
	{
		for (auto Count : InArgs._OptionCounts)
		{
			OptionsCounts.Add(MakeShareable(new int32(Count)));
		}
	}

	ChildSlot
	[
		SNew(SComboBox<FComboItemType>)
		.OptionsSource(&Options)
		.OnSelectionChanged(this, &SAssistantRichCombo::OnSelectionChanged)
		.OnGenerateWidget(this, &SAssistantRichCombo::MakeWidgetForOption)
		.InitiallySelectedItem(CurrentItem)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 2.0f, 6.0f, 2.0f)
			[
				SNew(SImage)
				.Image(this, &SAssistantRichCombo::GetCurrentItemIcon)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 3.0f, 6.0f, 3.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SAssistantRichCombo::GetCurrentItemLabel)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SAssistantRichCombo::GetCurrentItemCount)
			]
		]
	];
}

void SAssistantRichCombo::Initialize(const TArray<FString> OptionNames)
{
	if (OptionNames.Num() > 0)
	{
		for (auto Name : OptionNames)
		{
			Options.Add(MakeShareable(new FString(Name)));
		}

		CurrentItem = Options[0];
		Construct(FArguments());
	}
}

TSharedRef<SWidget> SAssistantRichCombo::MakeWidgetForOption(FComboItemType InOption)
{
	const UClass* ClassObject = FindObject<UClass>(ANY_PACKAGE, **InOption);
	const FSlateBrush* ClassIcon = FSlateIconFinder::FindIconBrushForClass(ClassObject);


	return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(3.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(SImage)
			.Image(ClassIcon)
			.Visibility(ClassIcon != FEditorStyle::GetDefaultBrush() ? EVisibility::Visible : EVisibility::Collapsed)
		]

		+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 3.0f, 6.0f, 3.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*InOption))
			]

		+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(STextBlock)
				// check null?
				.Text(FText::FromString(FString::FromInt(*OptionsCounts[Options.Find(InOption)])))
			];
}

void SAssistantRichCombo::OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type)
{
	CurrentItem = NewValue;
}

FText SAssistantRichCombo::GetCurrentItemLabel() const
{
	if (CurrentItem.IsValid())
	{
		return FText::FromString(*CurrentItem);
	}

	return (Options.Num()> 0) ? LOCTEXT("InvalidComboEntryText", "<<Invalid option>>") : LOCTEXT("InvalidComboEntryText", "<<Empty>>");
}

FText SAssistantRichCombo::GetCurrentItemCount() const
{
	if (CurrentItem.IsValid())
	{
		int32 index = Options.Find(CurrentItem);
		if (OptionsCounts.IsValidIndex(index))
		{
			return FText::FromString(FString::FromInt(*OptionsCounts[index]));
		}
	}

	return (Options.Num()> 0) ? LOCTEXT("InvalidComboEntryText", "<<Invalid option>>") : LOCTEXT("InvalidComboEntryText", "<<Empty>>");
}

const FSlateBrush* SAssistantRichCombo::GetCurrentItemIcon() const
{
	UClass* ExistingObject = NULL;
	if (CurrentItem.IsValid())
	{
		ExistingObject = FindObject<UClass>(ANY_PACKAGE, **CurrentItem);
	}
	const FSlateBrush* ClassIcon = FSlateIconFinder::FindIconBrushForClass(ExistingObject);

	return ClassIcon;
}

#undef LOCTEXT_NAMESPACE