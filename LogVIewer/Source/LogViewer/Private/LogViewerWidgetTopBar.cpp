// Copyright Dmitrii Labadin 2019

#include "LogViewerWidgetTopBar.h"
#include "LogViewerWidgetCategoriesView.h"
#include "LogViewerWidgetMain.h"
#include "LogViewerSettingsButton.h"
#include "SlateOptMacros.h"
#include "IDesktopPlatform.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "ProjectDescriptor.h"
#include "EditorDirectories.h"
#include "DesktopPlatformModule.h"
#include "Settings/EditorSettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/DOcking/SDockTab.h"
#include "Widgets/Input/SSearchBox.h"
#include "Misc/FileHelper.h"
#include "LogViewerStructs.h"
#include "OutputLogTextLayoutMarshaller.h"

#define LOCTEXT_NAMESPACE "SLogViewerWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerWidgetTopBar::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;
	CategoryMenu = InArgs._CategoryMenu;
	
	//FSlateColorBrush brushClr(FLinearColor::White);

	SettingsButton = SNew(SLogViewerSettingsButton).MainWidget(MainWidget); ;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(0)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		//.BorderBackgroundColor(FLinearColor(1.0f, 0.5f, 0.0f, 0.75f))
		//.BorderImage(brushClr)
		//.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		//.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerWidgetTopBar::OnOpenFilePressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Open Log", "Open Log"))
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerWidgetTopBar::OnClearOutputPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text_Lambda([&]() { return MainWidget->MessagesTextMarshaller->GetNumMessages() > 0 ? LOCTEXT("LogViewerProClearOutput", "Clear Output") : LOCTEXT("LogViewerProClearOutput", "Clear Categories"); })
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.7)
			.Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
			[
				SAssignNew(FilterTextBox, SSearchBox)
				.HintText(LOCTEXT("FilterLogHint", "Filter Log (Ctrl + F)"))
				//.OnTextChanged(this, &SLogViewerWidgetTopBar::OnFilterTextChanged)
				.OnTextCommitted(this, &SLogViewerWidgetTopBar::OnFilterTextCommitted)
				.DelayChangeNotificationsWhileTyping(true)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return MainWidget->IsListeningEngine() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerWidgetTopBar::OnListenEnginePressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Listen Engine", "Listen Engine"))
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return MainWidget->IsFollowLog() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerWidgetTopBar::OnFollowLogPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Follow Log", "Follow Log"))
					]
				]
			]

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				[
					SettingsButton.ToSharedRef()
				]
		]
	];
}

void SLogViewerWidgetTopBar::OnListenEnginePressed(ECheckBoxState CheckState)
{
	if (!MainWidget->IsListeningEngine())
	{
		MainWidget->BeginListenEngine();
	}
	else
	{
		MainWidget->StopListenEngine();
	}
}

void SLogViewerWidgetTopBar::OnClearOutputPressed(ECheckBoxState CheckState)
{
	if (MainWidget->MessagesTextMarshaller->GetNumMessages() > 0)
	{
		MainWidget->CleanupMessagesOnly();
	}
	else
	{
		MainWidget->CleanupAll();
	}
	//MainWidget->Cleanup();
	//MainWidget->Refresh();
}

void SLogViewerWidgetTopBar::OnFollowLogPressed(ECheckBoxState CheckState)
{
	if (MainWidget->IsFollowLog())
	{
		MainWidget->EndFollowLog();
	}
	else
	{
		MainWidget->BeginFollowLog();
	}
}

void SLogViewerWidgetTopBar::OpenLog()
{
	OnOpenFilePressed(ECheckBoxState::Checked);
}

void SLogViewerWidgetTopBar::OnOpenFilePressed(ECheckBoxState CheckState)
{
	if (LastSelectedLogsPath.IsEmpty())
	{
		LastSelectedLogsPath = FPaths::ProjectLogDir();
	}

	//const FString ProjectFileDescription = LOCTEXT("FileTypeDescription", "Unreal Project File").ToString();
	//const FString ProjectFileExtension = FString::Printf(TEXT("*.%s"), *FProjectDescriptor::GetExtension());
	//const FString FileTypes = FString::Printf(TEXT("%s (%s)|%s"), *ProjectFileDescription, *ProjectFileExtension, *ProjectFileExtension);

	const FString ProjectFileDescription = LOCTEXT("FileTypeDescription", "Log file").ToString();
	const FString LogFileType(TEXT("log"));
	const FString LogFileExtension = FString::Printf(TEXT("*.%s"), *LogFileType);
	const FString FileTypes = FString::Printf(TEXT("%s (%s)|%s"), *ProjectFileDescription, *LogFileExtension, *LogFileExtension);

	// Prompt the user for the filenames
	TArray<FString> OpenFilenames;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bOpened = false;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		bOpened = DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			LOCTEXT("OpenFile", "Open Log file").ToString(),
			LastSelectedLogsPath,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::None,
			OpenFilenames
		);
	}

	if (!bOpened || OpenFilenames.Num() == 0)
	{
		return;
		//HandleProjectViewSelectionChanged(NULL, ESelectInfo::Direct, FText());
	}
	FString Path = OpenFilenames[0];
	if (FPaths::IsRelative(Path))
	{
		Path = FPaths::ConvertRelativePathToFull(Path);
	}
	LastSelectedLogsPath = Path;

	MainWidget->CleanupAll();

	if (!FPaths::FileExists(Path))
	{
		return;
	}

	TArray<FString> LogStrings;
	if (!FFileHelper::LoadANSITextFileToStrings(*Path, nullptr, LogStrings) == true)
	{
		return;
	}

	if (LogStrings.Num() == 0)
	{
		return;
	}
	
	//@HACK - there might be leftovers of UTF-8 signature in the first line. Remove if exist
	LogStrings[0].RemoveFromStart(TEXT("???"));

	// Parse each line from config
	bParsingLineWithTime = false;
	FString AccumulatorString;
	for (auto& LogString : LogStrings)
	{
		LogString.TrimStartAndEndInline();
		if (!IsLogLineWithTime(LogString) && !bParsingLineWithTime)
		{
			if (HasCategory(LogString))
			{
				const FParsedLogLine LogLine = ParseSimpleLogLine(LogString);
				MainWidget->HandleNewLogMessageReceived(*LogLine.LogMessage, LogLine.VerbosityLevel, FName(*LogLine.LogCategory));
			}
			else
			{
				const FParsedLogLine LogLine(LogString);
				MainWidget->HandleNewLogMessageReceived(*LogLine.LogMessage, LogLine.VerbosityLevel, FName(*LogLine.LogCategory));
			}
			continue;
		}
		else if (!IsLogLineWithTime(LogString) && bParsingLineWithTime)
		{
			AccumulatorString = AccumulatorString + TEXT("\r\n") + LogString;
			continue;
		}
		else
		{
			bParsingLineWithTime = true; //After that point, new lines are just parts of previous log message
			FParsedLogLine LogLine = ParseLogLineWithTime(LogString);
			if (!AccumulatorString.IsEmpty())
			{
				LogLine.LogMessage.Append(AccumulatorString);
				AccumulatorString.Reset();
			}
			MainWidget->HandleNewLogMessageReceived(*LogLine.LogMessage, LogLine.VerbosityLevel, FName(*LogLine.LogCategory));
			continue;
		}
	}
	if (MainWidget->GetParentTab().IsValid())
	{
		MainWidget->GetParentTab().Pin()->SetLabel(FText::FromString(FPaths::GetBaseFilename(Path)));
	}


	MainWidget->StopListenEngine();
	//Default categories support for Open Files
	if (CategoryMenu->IsDefaultCategoriesFileExists())
	{
		CategoryMenu->ExecuteCategoriesDisableAll(ECheckBoxState::Checked);
		CategoryMenu->LoadDefaultCategories();
	}
	else
	{
		CategoryMenu->ExecuteCategoriesEnableAll(ECheckBoxState::Checked);
	}
}

FParsedLogLine SLogViewerWidgetTopBar::ParseSimpleLogLine(FString& LogString)
{
	if (!HasCategory(LogString))
	{
		return FParsedLogLine(LogString);
	}

	FParsedLogLine ParsedLogLine;
	LogString.Split(TEXT(":"), &ParsedLogLine.LogCategory, &LogString, ESearchCase::CaseSensitive, ESearchDir::FromStart);

	FString StringVerbosityLevel(TEXT("Log"));
	ParsedLogLine.VerbosityLevel = ELogVerbosity::Log;
	
	if (LogString.Split(TEXT(":"), &StringVerbosityLevel, &ParsedLogLine.LogMessage, ESearchCase::CaseSensitive, ESearchDir::FromStart))
	{
		if (StringVerbosityLevel.Equals(TEXT(" Fatal")))
		{
			ParsedLogLine.VerbosityLevel = ELogVerbosity::Fatal;
		}
		else if (StringVerbosityLevel.Equals(TEXT(" Error")))
		{
			ParsedLogLine.VerbosityLevel = ELogVerbosity::Error;
		}
		else if (StringVerbosityLevel.Equals(TEXT(" Warning")))
		{
			ParsedLogLine.VerbosityLevel = ELogVerbosity::Warning;
		}
		else if (StringVerbosityLevel.Equals(TEXT(" Display")))
		{
			ParsedLogLine.VerbosityLevel = ELogVerbosity::Display;
		}
		else if (StringVerbosityLevel.Equals(TEXT(" Verbose")))
		{
			ParsedLogLine.VerbosityLevel = ELogVerbosity::Verbose;
		}
		else if (StringVerbosityLevel.Equals(TEXT(" VeryVerbose")))
		{
			ParsedLogLine.VerbosityLevel = ELogVerbosity::VeryVerbose;
		}
		else
		{
			ParsedLogLine.LogMessage = LogString;
		}
	}
	else
	{
		ParsedLogLine.LogMessage = LogString;
	}
	return ParsedLogLine;
}

FParsedLogLine SLogViewerWidgetTopBar::ParseLogLineWithTime(FString& LogString)
{
	int32 pos;
	LogString.FindChar(']', pos);
	FString Time;
	LogString. Split(TEXT("]"), &Time, &LogString, ESearchCase::CaseSensitive, ESearchDir::FromStart);
	FString FrameNumber;
	LogString.Split(TEXT("]"), &FrameNumber, &LogString, ESearchCase::CaseSensitive, ESearchDir::FromStart);
	return ParseSimpleLogLine(LogString);
}

bool SLogViewerWidgetTopBar::IsLogLineWithTime(const FString& LogString) const
{
	int32 pos = INDEX_NONE;
	LogString.FindChar('[', pos);
	return pos == 0;
}

bool SLogViewerWidgetTopBar::HasCategory(const FString& LogString) const
{
	int32 doubleDotPos = INDEX_NONE;
	LogString.FindChar(':', doubleDotPos);
	int32 spaceBarPos = INDEX_NONE;
	LogString.FindChar(' ', spaceBarPos);
	if (doubleDotPos == INDEX_NONE)
	{
		return false;
	}
	return doubleDotPos < spaceBarPos;
}

void SLogViewerWidgetTopBar::OnFilterTextChanged(const FText& InFilterText)
{
	// Flag the messages count as dirty

}

void SLogViewerWidgetTopBar::OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	MainWidget->MessagesTextMarshaller->MarkMessagesCacheAsDirty();
	CategoryMenu->Filter.SetFilterText(InFilterText);
	FilterTextBox->SetError(CategoryMenu->Filter.GetSyntaxErrors());
	MainWidget->Refresh();
	//OnFilterTextChanged(InFilterText);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE