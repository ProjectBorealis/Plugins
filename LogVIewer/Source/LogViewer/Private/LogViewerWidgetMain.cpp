// Copyright Dmitrii Labadin 2019

#include "LogViewerWidgetMain.h"

#include "SlateOptMacros.h"

#include "OutputLogTextLayoutMarshaller.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Misc/CoreDelegates.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"

#include "LogViewerStructs.h"
#include "LogViewerWidgetCategoriesView.h"
#include "LogViewerWidgetTopBar.h"
#include "LogViewerSettingsButton.h"
#include "Engine/Engine.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "LogViewer.h"

#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "SLogViewerWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLogViewerWidgetMain::Construct( const FArguments& InArgs )
{
	LogColorTable = MakeUnique<FLogColorTable>();

	LogDevice = InArgs._LogOutputDevice;
	LogViewerModule = InArgs._Module;

	FEditorDelegates::PreBeginPIE.AddRaw(this, &SLogViewerWidgetMain::HandleBeginPIE);

	CategoryMenu = SNew(SLogViewerWidgetCategoriesView)
		.MainWidget(this);
	
	//for (const auto& Message : InArgs._Messages)
	//{
	//	CategoryMenu->AddCategory(Message->Category, false);
	//}

	MessagesTextMarshaller = FOutputLogTextLayoutMarshaller::Create(&CategoryMenu->Filter, this, LogColorTable.Get());

	MessagesTextBox = SNew(SMultiLineEditableTextBox)
		.Style(FAppStyle::Get(), "Log.TextBox")
		//.TextStyle(FAppStyle::Get(), "Log.Normal")
		.ForegroundColor(FLinearColor::Gray)
		.Marshaller(MessagesTextMarshaller)
		.OnVScrollBarUserScrolled(this, &SLogViewerWidgetMain::OnUserScrolled)
		.IsReadOnly(true)
		.AlwaysShowScrollbars(true);

	TopBar = SNew(SLogViewerWidgetTopBar)
		.MainWidget(this)
		.CategoryMenu(CategoryMenu.Get());
	    //.OnVScrollBarUserScrolled(this, &SLogViewerWidgetMain::OnUserScrolled);
	//.ContextMenuExtender(this, &SLogViewerWidgetMain::ExtendTextBoxMenu);

	ChildSlot
	[
		SNew(SBorder)
		.Padding(3)
		//.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Output Log Filter
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0)
			.FillHeight(1.0)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.8)
				[
					SNew(SVerticalBox)
					// Output Log Filter
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					[
						TopBar.ToSharedRef()
					]

					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					.FillHeight(1.0)
					[
						MessagesTextBox.ToSharedRef()
					]
				]
			
				+SHorizontalBox::Slot()
				.FillWidth(0.2)
				[
					CategoryMenu.ToSharedRef()
				]
			]
		]
	];

	if (CategoryMenu->IsDefaultCategoriesFileExists())
	{
		GetLogViewerModule()->DefaultCategoriesEnable();
		CategoryMenu->ExecuteCategoriesDisableAll(ECheckBoxState::Checked);
		CategoryMenu->LoadDefaultCategories();
	}
	else
	{
		GetLogViewerModule()->DefaultCategoriesDisable();
	}

	GetSettings()->TryLoadAndApplySettings();
	BeginListenEngine();
}

void SLogViewerWidgetMain::HandleBeginPIE(bool bIsSimulating)
{
	if (IsListeningEngine() && GetSettings()->IsCleanUpLogsOnPie())
	{
		this->CleanupMessagesOnly();
	}
}

SLogViewerSettingsButton* SLogViewerWidgetMain::GetSettings() const
{
	return TopBar->GetSettings();
}

void SLogViewerWidgetMain::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Now processing of buffered lines must be triggered from slate thread, not from any thread as it was before
	LogDevice->ProcessBufferedLines();
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

SLogViewerWidgetMain::~SLogViewerWidgetMain()
{
	FEditorDelegates::PreBeginPIE.RemoveAll(this);
}

void SLogViewerWidgetMain::BeginListenEngine()
{
	ListeningDelegate = LogDevice->SubscribeOnMessageReceived(this);
	BeginFollowLog();
}

void SLogViewerWidgetMain::StopListenEngine()
{
	LogDevice->UnsibscribeOnMessageReceived(ListeningDelegate);
}

bool SLogViewerWidgetMain::IsListeningEngine() const
{
	return LogDevice->IsListening(this);
}

void SLogViewerWidgetMain::HandleNewLogMessageReceived(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	const bool bShouldEnableCategory = !GetLogViewerModule()->IsDefaultCategoriesEnabled();
	CategoryMenu->AddCategory(Category, bShouldEnableCategory);
	MessagesTextMarshaller->AppendMessage(V, Verbosity, Category);
	
	if (bIsFollowLog)
	{
		MessagesTextBox->ScrollTo(FTextLocation(MessagesTextMarshaller->GetNumFilteredMessages() - 1));
	}
}

void SLogViewerWidgetMain::BeginFollowLog()
{
	bIsFollowLog = true;
	MessagesTextBox->ScrollTo(FTextLocation(MessagesTextMarshaller->GetNumFilteredMessages() - 1));
}

void SLogViewerWidgetMain::EndFollowLog()
{
	bIsFollowLog = false;
}

bool SLogViewerWidgetMain::IsFollowLog() const
{
	return bIsFollowLog;
}

void SLogViewerWidgetMain::OnUserScrolled(float ScrollOffset)
{
	const bool bIsUserScrolled = ScrollOffset < 1.0 && !FMath::IsNearlyEqual(ScrollOffset, 1.0f);
	if (bIsUserScrolled)
	{
		EndFollowLog();
	}
}

void SLogViewerWidgetMain::Refresh()
{
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();
	CategoryMenu->MarkDirty();

	// Re-count messages if filter changed before we refresh
	MessagesTextMarshaller->CountMessages();

	MessagesTextBox->GoTo(FTextLocation(0));
	MessagesTextMarshaller->MakeDirty();
	MessagesTextBox->Refresh();

	//RequestForceScroll();
}

void SLogViewerWidgetMain::CleanupMessagesOnly()
{
	MessagesTextMarshaller->ClearMessages();
	Refresh();
}

void SLogViewerWidgetMain::CleanupAll()
{
	MessagesTextMarshaller->ClearMessages();
	CategoryMenu->ClearCategories();
	Refresh();
}

void SLogViewerWidgetMain::SetFocusToFilterEditBox()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), TopBar->FilterTextBox->AsShared());
}

void SLogViewerWidgetMain::SetFocusToFilterCategoryBox()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), CategoryMenu->SearchBoxPtr->AsShared());
}

FReply SLogViewerWidgetMain::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.IsControlDown())
	{
		if (InKeyEvent.GetKey() == EKeys::O)
		{
			TopBar->OpenLog();
			return FReply::Handled();
		}

		if (InKeyEvent.GetKey() == EKeys::F)
		{
			SetFocusToFilterEditBox();
			return FReply::Handled();
		}
	}

	if (InKeyEvent.IsAltDown())
	{
		if (InKeyEvent.GetKey() == EKeys::C)
		{
			SetFocusToFilterCategoryBox();
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION