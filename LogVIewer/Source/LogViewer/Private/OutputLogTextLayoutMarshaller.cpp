// Copyright Dmitrii Labadin 2019

#include "OutputLogTextLayoutMarshaller.h"
#include "Framework/Text/SlateTextRun.h"
#include "Framework/Text/SlateTextLayout.h"
#include "LogViewerOutputDevice.h"
#include "LogViewerWidgetMain.h"
#include "LogViewerSettingsButton.h"
#include "EditorStyleSet.h"
#include "LogFilter.h"

#define LOCTEXT_NAMESPACE "SLogViewerWidgetMain"


TSharedRef< FOutputLogTextLayoutMarshaller > FOutputLogTextLayoutMarshaller::Create(FLogFilter* InFilter, SLogViewerWidgetMain* MainWidget, FLogColorTable* ColorTable)
{
	TSharedRef< FOutputLogTextLayoutMarshaller > Marshaller = MakeShareable(new FOutputLogTextLayoutMarshaller(InFilter));
	Marshaller->MainWidget = MainWidget;
	Marshaller->ColorTable = ColorTable;
	return Marshaller;
}

FOutputLogTextLayoutMarshaller::~FOutputLogTextLayoutMarshaller()
{
	
}

void FOutputLogTextLayoutMarshaller::SetText(const FString& SourceString, FTextLayout& TargetTextLayout)
{
	TextLayout = &TargetTextLayout;
	AppendMessagesToTextLayout(Messages);
}

void FOutputLogTextLayoutMarshaller::GetText(FString& TargetString, const FTextLayout& SourceTextLayout)
{
	SourceTextLayout.GetAsText(TargetString);
}

bool FOutputLogTextLayoutMarshaller::AppendMessage(const TCHAR* InText, const ELogVerbosity::Type InVerbosity, const FName& InCategory)
{
	TArray< TSharedPtr<FLogMessage> > NewMessages;
	if (!FLogViewerOutputDevice::CreateLogMessages(InText, InVerbosity, InCategory, NewMessages))
	{
		return false;
	}

	const bool bWasEmpty = Messages.Num() == 0;
	Messages.Append(NewMessages);

	if (TextLayout)
	{
		// If we were previously empty, then we'd have inserted a dummy empty line into the document
		// We need to remove this line now as it would cause the message indices to get out-of-sync with the line numbers, which would break auto-scrolling
		if (bWasEmpty)
		{
			TextLayout->ClearLines();
		}

		// If we've already been given a text layout, then append these new messages rather than force a refresh of the entire document
		AppendMessagesToTextLayout(NewMessages);
	}
	else
	{
		MarkMessagesCacheAsDirty();
		MakeDirty();
	}

	return true;
}

void FOutputLogTextLayoutMarshaller::AppendMessageToTextLayout(const TSharedPtr<FLogMessage>& InMessage)
{
	if (!Filter->IsMessageAllowed(InMessage))
	{
		return;
	}

	// Increment the cached count if we're not rebuilding the log
	if (!IsDirty())
	{
		CachedNumMessages++;
	}

	const FTextBlockStyle& MessageTextStyle = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>(InMessage->Style);

	TSharedRef<FString> LineText = InMessage->Message;

	TArray<TSharedRef<IRun>> Runs;
	Runs.Add(FSlateTextRun::Create(FRunInfo(), LineText, MessageTextStyle));

	TextLayout->AddLine(FSlateTextLayout::FNewLineData(MoveTemp(LineText), MoveTemp(Runs)));
}

void FOutputLogTextLayoutMarshaller::AppendMessagesToTextLayout(const TArray<TSharedPtr<FLogMessage>>& InMessages)
{
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(InMessages.Num());

	int32 NumAddedMessages = 0;

	for (const auto& CurrentMessage : InMessages)
	{
		if (!Filter->IsMessageAllowed(CurrentMessage))
		{
			continue;
		}

		++NumAddedMessages;

		//const FTextBlockStyle& MessageTextStyle = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>(CurrentMessage->Style);
		FTextBlockStyle MessageTextStyle = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>(CurrentMessage->Style);
		
		const bool bUseColorForRegularLogs = CurrentMessage->Verbosity != ELogVerbosity::Warning && CurrentMessage->Verbosity != ELogVerbosity::Error;
		if(bUseColorForRegularLogs)
		{
			if ( MainWidget->GetSettings()->IsShowLogColors())
			{
				MessageTextStyle.ColorAndOpacity = ColorTable->GetColorForCategory(CurrentMessage->Category);
			}
		}

		TSharedRef<FString> LineText = CurrentMessage->Message;

		TArray<TSharedRef<IRun>> Runs;
		Runs.Add(FSlateTextRun::Create(FRunInfo(), LineText, MessageTextStyle));

		LinesToAdd.Emplace(MoveTemp(LineText), MoveTemp(Runs));
	}

	// Increment the cached message count if the log is not being rebuilt
	if (!IsDirty())
	{
		CachedNumMessages += NumAddedMessages;
	}

	TextLayout->AddLines(LinesToAdd);
}

void FOutputLogTextLayoutMarshaller::ClearMessages()
{
	Messages.Empty();
	MakeDirty();
}

void FOutputLogTextLayoutMarshaller::CountMessages()
{
	// Do not re-count if not dirty
	if (!bNumMessagesCacheDirty)
	{
		return;
	}

	CachedNumMessages = 0;

	for (const auto& CurrentMessage : Messages)
	{
		if (Filter->IsMessageAllowed(CurrentMessage))
		{
			CachedNumMessages++;
		}
	}

	// Cache re-built, remove dirty flag
	bNumMessagesCacheDirty = false;
}

int32 FOutputLogTextLayoutMarshaller::GetNumMessages() const
{
	return Messages.Num();
}

int32 FOutputLogTextLayoutMarshaller::GetNumFilteredMessages()
{
	// No need to filter the messages if the filter is not set
	//if (!Filter->IsFilterSet())
	//{
	//	return GetNumMessages();
	//}
	//;

	// Re-count messages if filter changed before we refresh
	if (bNumMessagesCacheDirty)
	{
		CountMessages();
	}

	return CachedNumMessages;
}

void FOutputLogTextLayoutMarshaller::MarkMessagesCacheAsDirty()
{
	bNumMessagesCacheDirty = true;
}

FOutputLogTextLayoutMarshaller::FOutputLogTextLayoutMarshaller(FLogFilter* InFilter)
	: CachedNumMessages(0)
	, Filter(InFilter)
	, TextLayout(nullptr)
{
}