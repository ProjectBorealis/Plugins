// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "LogViewerOutputDevice.h"
#include "Misc/TextFilterExpressionEvaluator.h"

class SLogViewerWidgetMain;
class SLogViewerWidgetCategoriesView;

struct FCategoryItem
{
	FName CategoryName;
	bool bIsChecked;

	static bool SortItemsByName(const TSharedPtr<FCategoryItem>& A, const TSharedPtr<FCategoryItem>& B)
	{
		return A->CategoryName.ToString().Compare(B->CategoryName.ToString()) < 0;
	}

	static bool IsEqualByName(const TSharedPtr<FCategoryItem>& A, const FName& InCategoryName)
	{
		return A->CategoryName == InCategoryName;
	}
};

/**
* Holds information about filters
*/
class FLogFilter
{
public:
	//@TODO it's a mess here with two arrays for AllItesms and Available categories, Shouldn't be like that probably, but who knows
	TArray<TSharedPtr<FCategoryItem>> AllItems;
	TArray<TSharedPtr<FCategoryItem>> FilteredItems;
	TArray<FName> AvailableLogCategories;
	TArray<FName> SelectedLogCategories;

	bool bShowVeryVerbose;
	bool bShowVerbose;
	bool bShowLog;
	bool bShowDisplay;
	bool bShowWarnings;
	bool bShowErrors;

	bool bShowAllCategories;

	/** Enable all filters by default */
	FLogFilter()
		: TextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString)
	{
		bShowAllCategories = true;
		bShowErrors = bShowWarnings = bShowDisplay =  bShowLog = bShowVerbose = bShowVeryVerbose = true;
	}

	class SLogViewerWidgetMain* OwnerWidget;

	/** Returns true if any messages should be filtered out */
	bool IsFilterSet() { return !bShowErrors || !bShowLog || !bShowWarnings || TextFilterExpressionEvaluator.GetFilterType() != ETextFilterExpressionType::Empty || !TextFilterExpressionEvaluator.GetFilterText().IsEmpty(); }

	/** Checks the given message against set filters */
	bool IsMessageAllowed(const TSharedPtr<FLogMessage>& Message);

	/** Set the Text to be used as the Filter's restrictions */
	void SetFilterText(const FText& InFilterText) { TextFilterExpressionEvaluator.SetFilterText(InFilterText); }

	/** Get the Text currently being used as the Filter's restrictions */
	const FText GetFilterText() const { return TextFilterExpressionEvaluator.GetFilterText(); }

	/** Returns Evaluator syntax errors (if any) */
	FText GetSyntaxErrors() { return TextFilterExpressionEvaluator.GetFilterErrorText(); }

	const TArray<FName>& GetAvailableLogCategories() { return AvailableLogCategories; }

	/** Adds a Log Category to the list of available categories, if it isn't already present */
	void AddAvailableLogCategory(const FName& LogCategory, bool bSelected);

	/** Enables or disables a Log Category in the filter */
	void ToggleLogCategory(const FName& LogCategory);
	/** Enables or disables a Log Category in the filter */
	void EnableLogCategory(const FName& LogCategory);
	/** Enables or disables a Log Category in the filter */
	void DisableLogCategory(const FName& LogCategory);


	/** Returns true if the specified log category is enabled */
	bool IsLogCategoryEnabled(const FName& LogCategory) const;

	/** Returns true if the specified log category is enabled */
	bool IsLogCategoryExist(const FName& LogCategory) const { return AvailableLogCategories.Find(LogCategory) != INDEX_NONE; };

	/** Empties the list of selected log categories */
	void ClearSelectedLogCategories();

private:
	/** Expression evaluator that can be used to perform complex text filter queries */
	FTextFilterExpressionEvaluator TextFilterExpressionEvaluator;
};
