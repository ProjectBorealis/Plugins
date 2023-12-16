// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "LogViewerOutputDevice.h"

class FOutputLogTextLayoutMarshaller;
class SMultiLineEditableTextBox;
class SLogViewerWidgetCategoriesView;
class SLogViewerSettingsButton;
class SLogViewerWidgetTopBar;
class FTabManager;
class SDockTab;
class FLogViewerModule;

class SLogViewerWidgetMain 
	: public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS( SLogViewerWidgetMain )
		: _LogOutputDevice(nullptr)
		{}
		SLATE_ARGUMENT(TSharedPtr<FLogViewerOutputDevice>, LogOutputDevice)
		SLATE_ARGUMENT(FLogViewerModule*, Module)
	SLATE_END_ARGS()

	virtual ~SLogViewerWidgetMain();

	void Construct( const FArguments& InArgs );
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void BeginListenEngine();
	void StopListenEngine();
	bool IsListeningEngine() const;
private:
	FDelegateHandle ListeningDelegate;
public:

	/** Converts the array of messages into something the text box understands */
	TSharedPtr< FOutputLogTextLayoutMarshaller > MessagesTextMarshaller;
	
	void HandleNewLogMessageReceived(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category);

	//void HandleNewLogMessageReceived();

	/** The editable text showing all log messages */
	TSharedPtr< SMultiLineEditableTextBox > MessagesTextBox;
	TSharedPtr< SLogViewerWidgetCategoriesView > CategoryMenu;
	TSharedPtr< SLogViewerWidgetTopBar > TopBar;
	/** The editable text showing all log messages */
	TSharedPtr< SSearchBox > FilterTextBox;

	/** Forces re-population of the messages list */
	void Refresh();
	void CleanupMessagesOnly();
	void CleanupAll();

	void BeginFollowLog();
	void EndFollowLog();
	bool IsFollowLog() const;
	void OnUserScrolled(float ScrollOffset);

	void SetParentTab(TWeakPtr<SDockTab> InParentTab) { ParentTab = InParentTab; };
	TWeakPtr<SDockTab> GetParentTab() const { return ParentTab; };
	FLogViewerModule* GetLogViewerModule() const { return LogViewerModule; };

	TSharedPtr< FLogViewerOutputDevice > LogDevice;

	FLogColorTable* GetColorTable() const { return LogColorTable.Get(); };
	SLogViewerSettingsButton* GetSettings() const;
	void HandleBeginPIE(bool bIsSimulating);
private:
	void SetFocusToFilterEditBox();
	void SetFocusToFilterCategoryBox();

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	TWeakPtr<SDockTab> ParentTab;

	bool bIsFollowLog;
	FLogViewerModule* LogViewerModule;

	TUniquePtr< FLogColorTable > LogColorTable;
};
