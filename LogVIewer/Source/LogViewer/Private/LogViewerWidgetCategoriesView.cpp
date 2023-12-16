// Copyright Dmitrii Labadin 2019

#include "LogViewerWidgetCategoriesView.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SSearchBox.h"
#include "LogViewerStructs.h"
#include "LogViewerWidgetMain.h"
#include "LogViewerSettingsButton.h"
#include "LogViewer.h"

#include "DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Interfaces/IMainFrameModule.h"
#include "HAL/IPlatformFileModule.h"

#define LOCTEXT_NAMESPACE "SLogViewerWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

namespace FLogViewerCategoriesViewFilter
{
	void GetBasicStrings(const FCategoryItem& InCategoryItem, TArray<FString>& OutBasicStrings)
	{
		OutBasicStrings.Add(InCategoryItem.CategoryName.ToString());
	}
}

void SLogViewerWidgetCategoriesView::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;
	Filter.OwnerWidget = MainWidget;
	const FButtonStyle& ButtonStyle = FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Menu.Button");
	DeselectedImage = ButtonStyle.Normal;
	HoveredImage = ButtonStyle.Pressed;
	HoveredSelectedImage = *FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder");
	HoveredSelectedImage.TintColor = FSlateColor(FLinearColor(0.f, 0.0f, 1.f));
	SelectedImage = *FAppStyle::Get().GetBrush("ToolPanel.GroupBorder");

	CategoryHighlightImage = FSlateBrush();

	SearchTextFilter = MakeShareable(new FCategoriesEntryTextFilter(
		FCategoriesEntryTextFilter::FItemToStringArray::CreateStatic(&FLogViewerCategoriesViewFilter::GetBasicStrings)
	));

	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				SAssignNew(SearchBoxPtr, SSearchBox)
				.HintText(NSLOCTEXT("LogViewer", "SearchCategories", "Search Categories (Alt + C)"))
				.OnTextChanged(this, &SLogViewerWidgetCategoriesView::OnSearchChanged)
				.OnTextCommitted(this, &SLogViewerWidgetCategoriesView::OnSearchCommitted)
			]

			+ SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked(this, &SLogViewerWidgetCategoriesView::IsDefaultCategoriesEnabled)
				.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::OnMakeDefaultCategoriesPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.TextStyle(FAppStyle::Get(), "SmallText")
						.Text_Lambda([&]() { return MainWidget->GetLogViewerModule()->IsDefaultCategoriesEnabled() ? LOCTEXT("LogViewerClearDefault", "Clear Default") : LOCTEXT("LogViewerMakeDefault", "Make Default"); })
					]
				]
			]

			+ SVerticalBox::Slot()
			.Padding(0)
			[
				SAssignNew(DataDrivenContent, SBox)
				[
					SNew(SHorizontalBox)
				
					+ SHorizontalBox::Slot()
					[
						SAssignNew(ListView, SListView<TSharedPtr<FCategoryItem>>)
						.ListItemsSource(&Filter.FilteredItems)
						.OnGenerateRow(this, &SLogViewerWidgetCategoriesView::OnGenerateWidgetForItem)
						.ExternalScrollbar(ScrollBar)
					]
				
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ScrollBar
					]
				]
			]

			+ SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteCategoriesEnableAll)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Enable All", "Enable All"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerWidgetCategoriesView::IsCheckedVerbosityVeryVerbose)
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteVerbosityVeryVerbose)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("VeryVerbose", "VeryVerbose"))
							]
						]
					]

					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerWidgetCategoriesView::IsCheckedVerbosityVerbose)
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteVerbosityVerbose)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Verbose", "Verbose"))
							]
						]
					]

					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerWidgetCategoriesView::IsCheckedVerbosityLog)
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteVerbosityLog)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Messages", "Messages"))
							]
						]
					]
				]
				
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteCategoriesDisableAll)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Disable All", "Disable All"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerWidgetCategoriesView::IsCheckedVerbosityDisplay)
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteVerbosityDisplay)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Display", "Display"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerWidgetCategoriesView::IsCheckedVerbosityWarning)
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteVerbosityWarning)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Warning", "Warnings"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerWidgetCategoriesView::IsCheckedVerbosityError)
						.OnCheckStateChanged(this, &SLogViewerWidgetCategoriesView::ExecuteVerbosityError)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Errors", "Errors"))
							]
						]

					]
				]
			]
		]
	];
}

TSharedRef<ITableRow> SLogViewerWidgetCategoriesView::OnGenerateWidgetForItem(TSharedPtr<FCategoryItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FCategoryItem>>, OwnerTable)
		[
			SNew(SLogViewerWidgetCategoriesEntry, InItem.ToSharedRef())
				.MainWidget(MainWidget)
				.CategoryWidget(this)
		];
}

bool SLogViewerWidgetCategoriesView::IsSearchActive() const
{
	return !SearchTextFilter->GetRawFilterText().IsEmpty();
}

void SLogViewerWidgetCategoriesView::ClearCategories()
{
	Filter.AllItems.Reset();
	Filter.FilteredItems.Reset();
	Filter.AvailableLogCategories.Reset();
	Filter.SelectedLogCategories.Reset();
}

void SLogViewerWidgetCategoriesView::OnSearchChanged(const FText& InFilterText)
{
	MarkDirty();
	SearchTextFilter->SetRawFilterText(InFilterText);
	SearchBoxPtr->SetError(SearchTextFilter->GetFilterErrorText());
}

void SLogViewerWidgetCategoriesView::OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	OnSearchChanged(InFilterText);
}

void SLogViewerWidgetCategoriesView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bNeedsUpdate)
	{
		bNeedsUpdate = false;
		UpdateFilteredItems();
	}
}

void SLogViewerWidgetCategoriesView::AddCategory(const FName& InCategory, bool bSelected)
{
	if (Filter.IsLogCategoryExist(InCategory))
	{
		return;
	}
	Filter.AddAvailableLogCategory(InCategory, bSelected);
	TSharedPtr<FCategoryItem> Item = MakeShareable(new FCategoryItem{ InCategory, true });
	Filter.AllItems.Add(Item);
	MainWidget->GetColorTable()->RegisterCategory(InCategory);
	MarkDirty();
}

void SLogViewerWidgetCategoriesView::UpdateFilteredItems()
{
	if (IsSearchActive())
	{
		Filter.FilteredItems.Reset();
		auto FilterFtunction = [&](const TSharedPtr<FCategoryItem>& Item) { return SearchTextFilter->PassesFilter(*Item); };
		for (auto& Item : Filter.AllItems)
		{
			if (FilterFtunction(Item))
			{
				Filter.FilteredItems.Add(Item);
			}
		}
		Filter.FilteredItems.Sort(&FCategoryItem::SortItemsByName);
	}
	else
	{
		Filter.FilteredItems = Filter.AllItems;
		Filter.FilteredItems.Sort(&FCategoryItem::SortItemsByName);
	}

	//DataDrivenContent->SetVisibility(EVisibility::Visible);
	ListView->RequestListRefresh();
}

SLogViewerWidgetCategoriesView::~SLogViewerWidgetCategoriesView()
{

}

void SLogViewerWidgetCategoriesEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FCategoryItem>& InItem)
{
	MainWidget = InArgs._MainWidget;
	CategoryWidget = InArgs._CategoryWidget;
	bIsPressed = CategoryWidget->Filter.IsLogCategoryEnabled(InItem->CategoryName);
	Item = InItem;

	DeselectedImage = &CategoryWidget->DeselectedImage;
	HoveredImage = &CategoryWidget->HoveredImage;
	SelectedImage = &CategoryWidget->SelectedImage;
	HoveredSelectedImage = &CategoryWidget->HoveredSelectedImage;

	HighlightBrush = &CategoryWidget->CategoryHighlightImage;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(this, &SLogViewerWidgetCategoriesEntry::GetBorder)
		//.Cursor(EMouseCursor::GrabHand)
		//.ToolTip(AssetEntryToolTip)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::AsCultureInvariant(InItem->CategoryName.ToString()))
				.ColorAndOpacity_Lambda([this]()
				{
					return this->HandleGetCategoryColor();
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f)
			.HAlign(EHorizontalAlignment::HAlign_Right)
			[
				SNew(SBorder)
				.BorderImage(HighlightBrush)
				.BorderBackgroundColor_Lambda([this]()
				{
						if (!this->IsSelected())
						{
							return FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
						}
						return this->HandleGetCategoryColor();
				})
				//.ForegroundColor_Lambda([this]()
				//{
				//	if (!this->IsSelected())
				//	{
				//		 return FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
				//	}
				//	return this->HandleGetCategoryColor();
				//})
				[
					SNew(SBox)
					.WidthOverride(1.0f)
					//.WidthOverride_Lambda([this]()
					//{
					//		return this->IsSelected() ? 1.f : 0.f;
					//		//return 0.f;
					//})
				]
			]
			//+ SHorizontalBox::Slot()
			//.AutoWidth()
			//[
			//	SNew(SCheckBox)
			//]
		]
	];
}

FSlateColor SLogViewerWidgetCategoriesEntry::HandleGetCategoryColor() const
{
	if (MainWidget->GetSettings()->IsShowLogColors())
	{
		return MainWidget->GetColorTable()->GetColorForCategory(Item->CategoryName);
	}
	else
	{
		return FSlateColor::UseForeground();
	}
}


FReply SLogViewerWidgetCategoriesEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}

	//bIsPressed = true;
	ensure(MainWidget);
	MainWidget->CategoryMenu->Filter.ToggleLogCategory(Item->CategoryName);
	MainWidget->Refresh();

	return FReply::Handled();// .DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
}

FReply SLogViewerWidgetCategoriesEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}
	//if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	//{
	//	bIsPressed = false;
	//}
	//bIsPressed = false;
	return FReply::Handled();// .DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
}

bool SLogViewerWidgetCategoriesEntry::IsPressed() const
{
	return bIsPressed;
}

const FSlateBrush* SLogViewerWidgetCategoriesEntry::GetBorder() const
{
	if (IsHovered())
	{
		return IsSelected() ? HoveredSelectedImage : HoveredImage;
	}
	else if (IsSelected())
	{
		return SelectedImage;
	}
	else
	{
		return DeselectedImage;
	}
}


ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedVerbosityVeryVerbose() const { return Filter.bShowVeryVerbose ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedVerbosityVerbose() const { return Filter.bShowVerbose ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedVerbosityLog() const { return Filter.bShowLog ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedVerbosityDisplay() const { return Filter.bShowDisplay ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedVerbosityWarning() const { return Filter.bShowWarnings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedVerbosityError() const { return Filter.bShowErrors ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerWidgetCategoriesView::IsCheckedCategoriesShowAll() const { return Filter.bShowAllCategories ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };

ECheckBoxState SLogViewerWidgetCategoriesView::CategoriesSingle_IsChecked(FName InName) const
{
	return Filter.IsLogCategoryEnabled(InName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SLogViewerWidgetCategoriesView::ExecuteVerbosityVeryVerbose(ECheckBoxState CheckState)
{
	Filter.bShowVeryVerbose = !Filter.bShowVeryVerbose;
	MainWidget->Refresh();
}

void SLogViewerWidgetCategoriesView::ExecuteVerbosityVerbose(ECheckBoxState CheckState)
{
	Filter.bShowVerbose = !Filter.bShowVerbose;
	MainWidget->Refresh();
}

void SLogViewerWidgetCategoriesView::ExecuteVerbosityLog(ECheckBoxState CheckState)
{
	Filter.bShowLog = !Filter.bShowLog;
	MainWidget->Refresh();
}

void SLogViewerWidgetCategoriesView::ExecuteVerbosityDisplay(ECheckBoxState CheckState)
{
	Filter.bShowDisplay = !Filter.bShowDisplay;
	MainWidget->Refresh();
}

void SLogViewerWidgetCategoriesView::ExecuteVerbosityWarning(ECheckBoxState CheckState)
{
	Filter.bShowWarnings = !Filter.bShowWarnings;
	MainWidget->Refresh();
}

void SLogViewerWidgetCategoriesView::ExecuteVerbosityError(ECheckBoxState CheckState)
{
	Filter.bShowErrors = !Filter.bShowErrors;
	MainWidget->Refresh();
}

void SLogViewerWidgetCategoriesView::ExecuteCategoriesDisableAll(ECheckBoxState CheckState)
{
	Filter.ClearSelectedLogCategories();

	for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
	{
		Filter.EnableLogCategory(AvailableCategory);
	}

	for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
	{
		Filter.ToggleLogCategory(AvailableCategory);
	}


	MainWidget->Refresh();
}


void SLogViewerWidgetCategoriesView::ExecuteCategoriesEnableAll(ECheckBoxState CheckState)
{
	Filter.ClearSelectedLogCategories();

	for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
	{
		Filter.EnableLogCategory(AvailableCategory);
	}

	MainWidget->Refresh();
}


ECheckBoxState SLogViewerWidgetCategoriesView::IsDefaultCategoriesEnabled() const
{
	return MainWidget->GetLogViewerModule()->IsDefaultCategoriesEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
};


void SLogViewerWidgetCategoriesView::OnMakeDefaultCategoriesPressed(ECheckBoxState CheckState)
{
	if (!IsDefaultCategoriesFileExists())
	{
		MainWidget->GetLogViewerModule()->DefaultCategoriesEnable();
		SaveDefaultCategorties();
		//WriteDefaultCategories();
	}
	else
	{
		MainWidget->GetLogViewerModule()->DefaultCategoriesDisable();
		ClearDefaultCategories();
	}
}


void SLogViewerWidgetCategoriesView::ClearDefaultCategories()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "Saving is not supported on this platform!"));
		return;
	}

	if (!IsDefaultCategoriesFileExists())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CLearDefaultCategoriesDialogUnsupportedError", "Trying to clear Default categories, but file not found"));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteFile(*GetDefaultCategoriesFilePath());
}

bool SLogViewerWidgetCategoriesView::IsDefaultCategoriesFileExists() const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.FileExists(*GetDefaultCategoriesFilePath());
}

FString SLogViewerWidgetCategoriesView::GetDefaultFolderPath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewer");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}

FString SLogViewerWidgetCategoriesView::GetDefaultCategoriesFilePath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewer") + TEXT("/DefaultCategories.ini");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}

void SLogViewerWidgetCategoriesView::SaveDefaultCategorties()
{
	if (!CreateDefaultConfigFolder())
	{
		return;
	}
	FString Filename = GetDefaultCategoriesFilePath();
	SaveCategoriesIntoFile(Filename);
}

void SLogViewerWidgetCategoriesView::SaveCategoriesIntoFile(const FString& Filename)
{
	FArchive* LogFile = IFileManager::Get().CreateFileWriter(*Filename);
	if (LogFile == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogFileError", "Failed to open the specified file for saving!"));
		return;
	}

	FLVCategoriesSave CategoriesSave;
	CategoriesSave.Categories.Reserve(Filter.AvailableLogCategories.Num());

	for (const auto& Category : Filter.AvailableLogCategories)
	{
		FLVCategoriesSaveItem Item = FLVCategoriesSaveItem(Category.ToString(), Filter.IsLogCategoryEnabled(Category), MainWidget->GetColorTable()->GetColorForCategory(Category).ToFColor(true).ToHex());
		CategoriesSave.Categories.Add(Item);
	}

	const FString SettingsString = CategoriesSave.ToJson(true);
	LogFile->Serialize(TCHAR_TO_ANSI(*SettingsString), SettingsString.Len());
	LogFile->Close();
	delete LogFile;
}

bool SLogViewerWidgetCategoriesView::CreateDefaultConfigFolder()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "File managemetn is not supported on selected platform!"));
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString DefaultPath = GetLogViewerConfigPath();
	FPaths::NormalizeDirectoryName(DefaultPath);
	if (!PlatformFile.DirectoryExists(*DefaultPath))
	{
		PlatformFile.CreateDirectory(*DefaultPath);
	}
	return true;
}

FString SLogViewerWidgetCategoriesView::GetLogViewerConfigPath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewer");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}


// LOAD DEFAULT CATEGORIES
void SLogViewerWidgetCategoriesView::LoadDefaultCategories()
{
	FString Path = GetDefaultCategoriesFilePath();

	FLVCategoriesSave CategoriesSave;
	const bool bParsed = ParseCategoriesFile(Path, CategoriesSave);
	if (!bParsed)
	{
		UE_LOG(LogTemp, Error, TEXT("LogViewer: Unable to parse categories save file using new formart. Previous JSON warning in logs is related to that issue. Trying old format..."));
		const bool bLegacyParse = ParseCategoriesFileOld(Path, CategoriesSave);
		if (!bLegacyParse)
		{
			UE_LOG(LogTemp, Error, TEXT("LogViewer: Unable to parse categories save file using old format."));
			return;
		}
	}
	ApplyCategoriesSave(CategoriesSave);
	MainWidget->Refresh();
}


bool SLogViewerWidgetCategoriesView::ParseCategoriesFile(const FString& Path, FLVCategoriesSave& CategoriesSave)
{
	if (!FPaths::FileExists(Path))
	{
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Path))
	{
		return false;
	}

	const bool bParsingSuccessful = CategoriesSave.FromJson(JsonString);

	if (!bParsingSuccessful)
	{
		return false;
	}

	if (CategoriesSave.Categories.Num() <= 0)
	{
		return false;
	}

	return true;
}


bool SLogViewerWidgetCategoriesView::ParseCategoriesFileOld(const FString& Path, FLVCategoriesSave& CategoriesSave)
{
	TArray<FString> CategoriesStrings;
	if (!FFileHelper::LoadANSITextFileToStrings(*Path, nullptr, CategoriesStrings) == true)
	{
		return false;
	}

	if (CategoriesStrings.Num() == 0)
	{
		return false;
	}

	for (const auto& CategoryString : CategoriesStrings)
	{
		FString Category;
		FString State;
		bool bResult = CategoryString.Split(TEXT(" = "), &Category, &State);
		if (!bResult)
		{
			continue;
		}

		bool bisActive = State.Equals(TEXT("Off"));
		FLVCategoriesSaveItem NewItem(Category, bisActive, FLogColorTable::GenerateRandomColor().ToFColor(true).ToHex());
		CategoriesSave.Categories.Add(NewItem);
	}
	return true;
}

void SLogViewerWidgetCategoriesView::ApplyCategoriesSave(const FLVCategoriesSave& CategoriesSave)
{
	for (const auto& CategoryItem : CategoriesSave.Categories)
	{
		const FName CategoryName = FName(CategoryItem.CategoryName);

		if (!Filter.IsLogCategoryExist(CategoryName))
		{
			AddCategory(CategoryName, true);
		}
		Filter.EnableLogCategory(CategoryName);
		if (!CategoryItem.bIsActive)
		{
			Filter.ToggleLogCategory(CategoryName);
		}

		FColor Color = FColor::FromHex(CategoryItem.CategoryHexColor);
		MainWidget->GetColorTable()->SetColorForCategory(CategoryName, Color);
	}
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE