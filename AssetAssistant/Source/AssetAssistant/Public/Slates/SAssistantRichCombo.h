#pragma once

class SAssistantRichCombo : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssistantRichCombo)
	{}
	SLATE_ARGUMENT(TArray<FString>, OptionNames)
	SLATE_ARGUMENT(TArray<int32>, OptionCounts)
	SLATE_END_ARGS()

	typedef TSharedPtr<FString> FComboItemType;

	void Construct(const FArguments& InArgs);
	void Initialize(const TArray<FString> OptionNames);
	TSharedRef<SWidget> MakeWidgetForOption(FComboItemType InOption);
	void OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabel() const;
	FText GetCurrentItemCount() const;
	const FSlateBrush *GetCurrentItemIcon() const;

	FComboItemType CurrentItem;
	TArray<FComboItemType> Options;
	TArray<TSharedPtr<int32>> OptionsCounts;

private:
	/** The Class Viewer Node this item is associated with. */
	TSharedPtr< FString > AssociatedString;
};

