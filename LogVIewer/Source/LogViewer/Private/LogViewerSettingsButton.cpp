// Copyright Dmitrii Labadin 2019

#include "LogViewerSettingsButton.h"
#include "LogViewerWidgetCategoriesView.h"
#include "LogViewerWidgetMain.h"
#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"

#include "Misc/FileHelper.h"
#include "DesktopPlatformModule.h"

#include "LogViewerStructs.h"

#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerSettingsButton::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;

	SComboButton::Construct(SComboButton::FArguments()
		.ContentPadding(0)
		.ForegroundColor(FSlateColor::UseForeground())
		.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButton")
		.HasDownArrow(false)
		.OnGetMenuContent(this, &SLogViewerSettingsButton::GenerateSettingsMenu)
		.ButtonContent()
	[
		SNew(SHorizontalBox)
		
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(FMargin(4.0, 2.0))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Settings", "Settings"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(FMargin(4.0, 2.0))
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.Settings"))
				.ColorAndOpacity(FSlateColor::UseForeground())
		]
	]
	);

	//FSlateColorBrush brushClr(FLinearColor::White);
}

//SETTINGS

TSharedRef<SWidget> SLogViewerSettingsButton::GenerateSettingsMenu()
{
	// Get all menu extenders for this context menu from the content browser module

	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, nullptr, nullptr, /*bCloseSelfOnly=*/ true);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("LogViewerSettingsVisualSettings_SaveSettings", "Save Settings"),
		LOCTEXT("LogViewerSettingsVisualSettings_SaveSettingsTooltip", "Make current settings configuration default"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SLogViewerSettingsButton::SaveSettings)), NAME_None, EUserInterfaceActionType::Button);
	//MenuBuilder.AddMenuEntry(LOCTEXT("CollapseAll", "Collapse All"), LOCTEXT("CollapseAll_Tooltip", "Collapses the entire tree"), FSlateIcon(), FUIAction(FExecuteAction::CreateSP(this, &SClassViewer::SetAllExpansionStates, bool(false))), NAME_None, EUserInterfaceActionType::Button);

	MenuBuilder.BeginSection("Behavior", LOCTEXT("LVSettingsBehavior", "Behavior"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LVPSettingsCleanUp", "Clean Up On PIE"),
			LOCTEXT("LVPSettingsCleanUpTooltip", "Clean up logs each time PIE starts"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerSettingsButton::ToggleCleanUpLogsOnPie),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerSettingsButton::IsCleanUpLogsOnPie)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("LogCategories", LOCTEXT("LogViewerSettingsLogCategories", "Colorcoding"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowLogColors", "Colorful logs"),
			LOCTEXT("LogViewerSettingsVisualShowLogColorsToolTip", "Switch between legacy view and colored view"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerSettingsButton::ToggleShowLogColors),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerSettingsButton::IsShowLogColors)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);

	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("LogSupport", LOCTEXT("LogViewerSettingsSupport", "Support"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualSettings_JoinDis", "Join Discord"),
			LOCTEXT("LogViewerSettingsVisualSettings_JoinDisTooltip", "If you have questions about the plugin you are welcome in official Discord."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SLogViewerSettingsButton::JoinDiscordServer)), NAME_None, EUserInterfaceActionType::Button);
		

		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualSettings_Upgrade", "Upgrade to Pro"),
			LOCTEXT("LogViewerSettingsVisualSettings_UpgradeTooltip", "If you love that plugin, consider upgrading to pro version. It has even more good stuff."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SLogViewerSettingsButton::NavigateToProMarketplace)), NAME_None, EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();


	return MenuBuilder.MakeWidget();
}

void SLogViewerSettingsButton::ToggleShowLogColors()
{
	SettingsData.bColorfulLogs = !SettingsData.bColorfulLogs;
	//MainWidget->Invalidate(EInvalidateWidgetReason::PaintAndVolatility);
	//MainWidget->LogListView->Invalidate(EInvalidateWidgetReason::PaintAndVolatility);
	//MainWidget->CategoryMenu->Invalidate(EInvalidateWidgetReason::PaintAndVolatility);
	//MainWidget->CategoryMenu->MarkDirty();
	MainWidget->Refresh();
}

bool SLogViewerSettingsButton::IsShowLogColors() const
{
	return SettingsData.bColorfulLogs;
}

bool SLogViewerSettingsButton::IsCleanUpLogsOnPie() const
{
	return SettingsData.bCleanUpLogsOnPIE;
}

void SLogViewerSettingsButton::ToggleCleanUpLogsOnPie()
{
	SettingsData.bCleanUpLogsOnPIE = !SettingsData.bCleanUpLogsOnPIE;
}

void SLogViewerSettingsButton::TryLoadAndApplySettings()
{
	FString Path = GetDefaultSettingsFilePath();
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString , *Path))
	{
		return;
	}

	SettingsData.FromJson(JsonString);

	MainWidget->Refresh();
}

void SLogViewerSettingsButton::SaveSettings()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "Saving is not supported on this platform!"));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString DefaultPath = GetDefaultSettingsFolderPath();
	FPaths::NormalizeDirectoryName(DefaultPath);
	if (!PlatformFile.DirectoryExists(*DefaultPath))
	{
		PlatformFile.CreateDirectory(*DefaultPath);
	}

	FString Filename = GetDefaultSettingsFilePath();

	// save file
	FArchive* LogFile = IFileManager::Get().CreateFileWriter(*Filename);

	if (LogFile != nullptr)
	{

		const FString SettingsString = SettingsData.ToJson(true);
		LogFile->Serialize(TCHAR_TO_ANSI(*SettingsString), SettingsString.Len());
		LogFile->Close();
		delete LogFile;
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogFileError", "Failed to save settings to save file. Please that file: Project/Config/LogViewer/LogViewerSettings.ini is writeable."));
	}
}


void SLogViewerSettingsButton::JoinDiscordServer()
{
	const FString Link = TEXT("https://discord.gg/aYFGHpb2pa");
	FPlatformProcess::LaunchURL(*Link, nullptr, nullptr);
}


void SLogViewerSettingsButton::NavigateToProMarketplace()
{
	const FString Link = TEXT("https://www.unrealengine.com/marketplace/en-US/product/log-viewer-pro");
	FPlatformProcess::LaunchURL(*Link, nullptr, nullptr);
}

FString SLogViewerSettingsButton::GetDefaultSettingsFolderPath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewer");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}


//@TODO move to helper function and replace all entries
FString SLogViewerSettingsButton::GetDefaultSettingsFilePath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewerPro") + TEXT("/LogViewerSettings.ini");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE