// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/Text/BaseTextLayoutMarshaller.h"
#include "LogViewerStructs.h"

class FMenuBuilder;
class FOutputLogTextLayoutMarshaller;
class SLogViewerWidgetCategoriesView;
class SLogViewerWidgetMain;
class FLogFilter;
class FTextLayout;

/** Output log text marshaller to convert an array of FLogMessages into styled lines to be consumed by an FTextLayout */
class FOutputLogTextLayoutMarshaller : public FBaseTextLayoutMarshaller
{
public:

	static TSharedRef< FOutputLogTextLayoutMarshaller > Create(FLogFilter* InFilter, SLogViewerWidgetMain* MainWidget, FLogColorTable* ColorTable);

	virtual ~FOutputLogTextLayoutMarshaller();

	// ITextLayoutMarshaller
	virtual void SetText(const FString& SourceString, FTextLayout& TargetTextLayout) override;
	virtual void GetText(FString& TargetString, const FTextLayout& SourceTextLayout) override;

	bool AppendMessage(const TCHAR* InText, const ELogVerbosity::Type InVerbosity, const FName& InCategory);
	void ClearMessages();

	void CountMessages();

	int32 GetNumMessages() const;
	int32 GetNumFilteredMessages();

	void MarkMessagesFilterAsDirty();
	void MarkMessagesCacheAsDirty();

protected:

	FOutputLogTextLayoutMarshaller(FLogFilter* InFilter);

	void AppendMessageToTextLayout(const TSharedPtr<FLogMessage>& InMessage);
	void AppendMessagesToTextLayout(const TArray<TSharedPtr<FLogMessage>>& InMessages);

	/** All log messages to show in the text box */
	TArray< TSharedPtr<FLogMessage> > Messages;

	/** Holds cached numbers of messages to avoid unnecessary re-filtering */
	int32 CachedNumMessages;

	/** Flag indicating the messages count cache needs rebuilding */
	bool bNumMessagesCacheDirty;

	/** Visible messages filter */
	FLogFilter* Filter;

	FTextLayout* TextLayout;
	FLogColorTable* ColorTable;
	SLogViewerWidgetMain* MainWidget;
};

