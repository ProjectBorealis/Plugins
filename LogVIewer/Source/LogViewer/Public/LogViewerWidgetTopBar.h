// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SlateFwd.h"
#include "Brushes/SlateColorBrush.h"

class FOutputLogTextLayoutMarshaller;
class SLogViewerWidgetCategoriesView;
class SLogViewerSettingsButton;
class SLogViewerWidgetMain;
class SSearchBox;
class FParsedLogLine;

class SLogViewerWidgetTopBar
	: public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SLogViewerWidgetTopBar)
		: _MainWidget()
		, _CategoryMenu()
	{}
		SLATE_ARGUMENT(SLogViewerWidgetMain*, MainWidget)
		SLATE_ARGUMENT(SLogViewerWidgetCategoriesView*, CategoryMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

		SLogViewerWidgetMain* MainWidget;
	SLogViewerWidgetCategoriesView* CategoryMenu;

	TSharedPtr< SSearchBox > FilterTextBox;

	void OpenLog();

	SLogViewerSettingsButton* GetSettings() const { return SettingsButton.Get(); };
private:
	TSharedPtr<SLogViewerSettingsButton> SettingsButton;
	//FSlateColorBrush brushClr;// = FSlateColorBrush(FLinearColor::White);
	void OnListenEnginePressed(ECheckBoxState CheckState);
	void OnClearOutputPressed(ECheckBoxState CheckState);
	void OnOpenFilePressed(ECheckBoxState CheckState);
	void OnFollowLogPressed(ECheckBoxState CheckState);
	FString LastSelectedLogsPath;
	FParsedLogLine ParseSimpleLogLine(FString& LogString);
	FParsedLogLine ParseLogLineWithTime(FString& LogString);
	bool IsLogLineWithTime(const FString& LogString) const;
	bool HasCategory(const FString& LogString) const;
	bool bParsingLineWithTime = false;

	void OnFilterTextChanged(const FText& InFilterText);
	void OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);
};
