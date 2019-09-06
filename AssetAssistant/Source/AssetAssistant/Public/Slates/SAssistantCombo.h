#pragma once

class SAssistantCombo : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssistantCombo)
	{}
	SLATE_ARGUMENT(struct FComboDirectories, ComboDirectories)
	SLATE_ARGUMENT(TArray<FString>, OptionNames)
	SLATE_END_ARGS()

	typedef TSharedPtr<FString> FComboItemType;
	
	void Construct(const FArguments& InArgs);
	void Initialize(const TArray<FString> OptionNames);
	TSharedRef<SWidget> MakeWidgetForOption(FComboItemType InOption);
	void OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabel() const;

	FComboItemType CurrentItem;
	TArray<FComboItemType> Options;

private:
	/** The Class Viewer Node this item is associated with. */
	TSharedPtr< FString > AssociatedString;
};

