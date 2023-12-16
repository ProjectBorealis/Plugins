// Copyright Dmitrii Labadin 2023

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "LogViewerStructs.h"
#include "Serialization/JsonSerializerMacros.h"

#include "Widgets/Input/SComboButton.h"


class SLogViewerWidgetMain;


class FLVSettingsData : public FJsonSerializable
{
public:
	FLVSettingsData() 
		: bColorfulLogs(true)
		, bCleanUpLogsOnPIE(false)
		{}

	virtual ~FLVSettingsData() {}

	bool bColorfulLogs;
	bool bCleanUpLogsOnPIE;

	// FJsonSerializable
	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE("ColorfulLogs", bColorfulLogs);
		JSON_SERIALIZE("ClenUpLogsOnPie", bCleanUpLogsOnPIE);
	END_JSON_SERIALIZER
};

class SLogViewerSettingsButton : public SComboButton
{
public:

	SLATE_BEGIN_ARGS(SLogViewerSettingsButton)
		: _MainWidget()
	{}
		SLATE_ARGUMENT(SLogViewerWidgetMain*, MainWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	bool IsShowLogColors() const;
	bool IsCleanUpLogsOnPie() const;

	void TryLoadAndApplySettings();

private:

	SLogViewerWidgetMain* MainWidget;

	TSharedRef<SWidget> GenerateSettingsMenu();
	void ToggleShowLogColors();
	void ToggleCleanUpLogsOnPie();

	void SaveSettings();
	void JoinDiscordServer();
	void NavigateToProMarketplace();
	
	FString GetDefaultSettingsFolderPath() const;
	FString GetDefaultSettingsFilePath() const;

	FLVSettingsData SettingsData;
};
