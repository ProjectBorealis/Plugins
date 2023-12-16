// Copyright Dmitrii Labadin 2019

#include "LogFilter.h"
#include "LogViewerWidgetMain.h"
#include "LogViewerStructs.h"
#include "LogViewerWidgetCategoriesView.h"
#include "Misc/TextFilterExpressionEvaluator.h"

#define LOCTEXT_NAMESPACE "SLogViewerWidgetMain"


/** Expression context to test the given messages against the current text filter */
class FLogFilter_TextFilterExpressionContext : public ITextFilterExpressionContext
{
public:
	explicit FLogFilter_TextFilterExpressionContext(const FLogMessage& InMessage) : Message(&InMessage) {}
	virtual bool TestBasicStringExpression(const FTextFilterString& InValue, const ETextFilterTextComparisonMode InTextComparisonMode) const override { return TextFilterUtils::TestBasicStringExpression(*Message->Message, InValue, InTextComparisonMode); }
	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue, const ETextFilterComparisonOperation InComparisonOperation, const ETextFilterTextComparisonMode InTextComparisonMode) const override { return false; }

private:
	const FLogMessage* Message;
};

bool FLogFilter::IsMessageAllowed(const TSharedPtr<FLogMessage>& Message)
{
	// Filter Verbosity
	{
		if (Message->Verbosity == ELogVerbosity::Error && !bShowErrors)
		{
			return false;
		}

		if (Message->Verbosity == ELogVerbosity::Warning && !bShowWarnings)
		{
			return false;
		}

		if (Message->Verbosity == ELogVerbosity::Display && !bShowDisplay)
		{
			return false;
		}

		if (Message->Verbosity == ELogVerbosity::Log && !bShowLog)
		{
			return false;
		}

		if (Message->Verbosity == ELogVerbosity::Verbose && !bShowVerbose)
		{
			return false;
		}

		if (Message->Verbosity == ELogVerbosity::VeryVerbose && !bShowVeryVerbose)
		{
			return false;
		}
	}

	// Filter by Category
	{
		if (!IsLogCategoryEnabled(Message->Category))
		{
			return false;
		}
	}

	// Filter search phrase
	{
		if (!TextFilterExpressionEvaluator.TestTextFilter(FLogFilter_TextFilterExpressionContext(*Message)))
		{
			return false;
		}
	}

	return true;
}

void FLogFilter::AddAvailableLogCategory(const FName& LogCategory, bool bSelected)
{
	AvailableLogCategories.Add(LogCategory);
	// Use an insert-sort to keep AvailableLogCategories alphabetically sorted
	//int32 InsertIndex = 0;
	//for (InsertIndex = AvailableLogCategories.Num() - 1; InsertIndex >= 0; --InsertIndex)
	//{
	//	FName CheckCategory = AvailableLogCategories[InsertIndex];
	//	// No duplicates
	//	if (CheckCategory == LogCategory)
	//	{
	//		return;
	//	}
	//	else if (CheckCategory.Compare(LogCategory) < 0)
	//	{
	//		break;
	//	}
	//}
	//
	//AvailableLogCategories.Insert(LogCategory, InsertIndex + 1);
	if (bShowAllCategories && bSelected)
	{
		ToggleLogCategory(LogCategory);
	}
}

void FLogFilter::ToggleLogCategory(const FName& LogCategory)
{
	int32 FoundIndex = SelectedLogCategories.Find(LogCategory);
	if (FoundIndex == INDEX_NONE)
	{
		SelectedLogCategories.Add(LogCategory);
	}
	else
	{
		SelectedLogCategories.RemoveAt(FoundIndex, /*Count=*/1, /*bAllowShrinking=*/false);
	}
}

void FLogFilter::EnableLogCategory(const FName& LogCategory)
{
	int32 FoundIndex = SelectedLogCategories.Find(LogCategory);
	if (FoundIndex != INDEX_NONE)
	{
		return; //Already selected
	}
	SelectedLogCategories.Add(LogCategory);
}

void FLogFilter::DisableLogCategory(const FName& LogCategory)
{
	int32 FoundIndex = SelectedLogCategories.Find(LogCategory);
	if (FoundIndex != INDEX_NONE)
	{
		return; //Already removed
	}
	SelectedLogCategories.RemoveAt(FoundIndex, /*Count=*/1, /*bAllowShrinking=*/false);
}

bool FLogFilter::IsLogCategoryEnabled(const FName& LogCategory) const
{
	return SelectedLogCategories.Contains(LogCategory);
}

void FLogFilter::ClearSelectedLogCategories()
{
	// No need to churn memory each time the selected categories are cleared
	SelectedLogCategories.Reset(SelectedLogCategories.Num());
}

#undef LOCTEXT_NAMESPACE