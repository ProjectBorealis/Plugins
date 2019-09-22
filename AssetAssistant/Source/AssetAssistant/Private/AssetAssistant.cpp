// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistant.h"
#include "EdMode.h"
#include "EditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "AssetAssistantEditorCommands.h"
#include "AssetAssistantEdMode.h"
#include "EditorModeManager.h"
#include "AssetAssistantEditorUISetting.h"
#include "AssetAssistantEdModeToolkit.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Docking/SDockTab.h"

static const FName AssetAssistantTabName("AssetAssistant");
#define LOCTEXT_NAMESPACE "FAssetAssistant"
#define DEFAULT_PLUGIN_HOTKEY 2

void FAssetAssistant::StartupModule()
{
	FAssetAssistantStyle::Initialize();
	FAssetAssistantEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AssetAssistantTabName, FOnSpawnTab::CreateRaw(this, &FAssetAssistant::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FAssetAssistantTabTitle", "AssetAssistant"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	int32 PluginHotkey = DEFAULT_PLUGIN_HOTKEY;
	GConfig->GetInt(TEXT("AssetAssistant"), TEXT("PluginHotkey"), PluginHotkey, GEditorPerProjectIni);

	FEditorModeRegistry::Get().RegisterMode<FAssetAssistantEdMode>(
		FAssetAssistantEdMode::EM_AssetAssistantEdModeId,
		LOCTEXT("AssetAssistantEdModeName", "Asset Assistant"),
		FSlateIcon(FAssetAssistantStyle::GetStyleSetName(), "AssetAssistant.AssetAssistantMode", "AssetAssistant.AssetAssistantMode.Small"),
		true,
		(PluginHotkey == 1) ? -1 : ((PluginHotkey - 1) * 100) + 50
	);

	TArray<FEditorModeInfo> EditorModeInfo;
	TArray<FEditorModeID> UnrIds;
	for (int32 index = 0; index < FEditorModeRegistry::Get().GetSortedModeInfo().Num(); index++)
	{
		EditorModeInfo.Add(FEditorModeRegistry::Get().GetSortedModeInfo()[index]);
		UnrIds.Add(FEditorModeRegistry::Get().GetSortedModeInfo()[index].ID);
	}
}

TSharedRef<SDockTab> FAssetAssistant::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if (!EDMode)
	{
		EDMode = (FAssetAssistantEdMode*)GLevelEditorModeTools().GetActiveMode(FAssetAssistantEdMode::EM_AssetAssistantEdModeId);
		FSlateFontInfo RichFont = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 12);
		FLinearColor RichColor = FLinearColor::White;
	}

	FAssetAssistantEdModeToolkit instanceEdModeToolkit = FAssetAssistantEdModeToolkit();
	FText VersionName = FText::FromString(IPluginManager::Get().FindPlugin(TEXT("AssetAssistant"))->GetDescriptor().VersionName);
	FText CreatedBy = FText::FromString(TEXT("by ") + IPluginManager::Get().FindPlugin(TEXT("AssetAssistant"))->GetDescriptor().CreatedBy);
	FText FriendlyName = FText::FromString(IPluginManager::Get().FindPlugin(TEXT("AssetAssistant"))->GetDescriptor().FriendlyName);
	FText ChangeLog = LOCTEXT("AssetAssistantChangeLog",
"1.5.0\n\
------------------\n\
* Adapting plugin to the 4.23 UE4 version \n\
\n\
\n\
1.4.0\n\
------------------\n\
* Fixed an issue where duplicated asset did not use{ Arr } pattern \n\
\n\
\n\
1.3.0\n\
------------------\n\
* Update to 4.19.0 version of UE4 \n\
\n\
\n\
1.2.0\n\
------------------\n\
* Update to 4.18.0 version of UE4 \n\
\n\
\n\
1.1.0\n\
------------------\n\
* Update to 4.17.1 version of UE4 \n\
* New look and functionality for macros - autosave recovery and sets recovery \n\
\n\
\n\
1.0.0\n\
------------------\n\
* Result category: Import - content, editor, paste; Export - list, Tarray, set; Clear; Remove; Append; Search element\n\
* Find category: type, directory, from, text\n\
* Modify category: Pattern [{Name}, {Date}, {Time}, {Arr1}, {Arr2}, {Nr}, {Nr+}, {Nr-}, multi actions], Search, Rename, Save, Open, Remove, Dirty, Duplicate\n\
* Macro category: Sets, Autosave sets, Recovery [Load, Load & find, Load & open], Autosave config\n\
* Extra category: Plugin Hotkey\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
 ");

	FText HowPluginWorks = LOCTEXT("AssetAssistantHowPluginWorks",
"<span size=\"12\">Find:</>\n\
<span size=\"4\">------------------------------------</>\n\
Plugin allows you to control your assets and the workflow better, with possibility of finding assets from many sources:\n\
- from <span color=\"#66CCFF\" size=\"10\">search</> selecting Directory, Type, Place and Text\n\
- selected files in <span color=\"#66CCFF\" size=\"10\">Content Browser</>\n\
- files currently being <span color=\"#66CCFF\" size=\"10\">opened / edited</>\n\
- paste from <span color=\"#66CCFF\" size=\"10\">TArray</> or standard <span color=\"#66CCFF\" size=\"10\">List</>\n\
\n\
<span size=\"12\">Results:</>\n\
<span size=\"4\">------------------------------------</>\n\
Gives you preview of assets list. It is used for Modifying and Macro operation on assets. You can modify this list as you want - removing selected rows, removing unselected rows, appending new results or clearing. In any time you can send list to the clipboard using TArray or List button. You can also save list to Set which is remembered.\n\
\n\
<span size=\"12\">Modify:</>\n\
<span size=\"4\">------------------------------------</>\n\
In this section you will find pattern which is used when assets are renamed or duplicated. You can add your own patterns using vars:\n\
<span color=\"#66CCFF\" size=\"10\">{Name}</> - origin asset name\n\
<span color=\"#66CCFF\" size=\"10\">{Date}</>, <span color=\"#66CCFF\" size=\"10\">{Time}</> - Current day and time with formats: <span color=\"#66CCFF\" size=\"10\">Y:M:D, h:m:s</>\n\
<span color=\"#66CCFF\" size=\"10\">{Arr1}</>, <span color=\"#66CCFF\" size=\"10\">{Arr2}</> - custom arrays\n\
<span color=\"#66CCFF\" size=\"10\">{Nr}</>, <span color=\"#66CCFF\" size=\"10\">{Nr+1}</>, <span color=\"#66CCFF\" size=\"10\">{Nr-7}</> these are number first param will create series: 0,1,2... next one 1,2,3... and last one -7,-6,-5... etc\n\
\n\
Example of pattern: <span color=\"#66CCFF\" size=\"10\">{Arr1}custom_string{Name}</> will create names based on arr1 elements appending 'custom_string' and origin name.\n\
\n\
\n\
Acording to the top results list you can do\n\
I) single actions for all result:\n\
- <span color=\"#66CCFF\" size=\"10\">Search</> - finding and selecting all assets from the result in the Content Browser\n\
- <span color=\"#66CCFF\" size=\"10\">Rename</> - renaming all assets using pattern\n\
- <span color=\"#66CCFF\" size=\"10\">Save</> - saving all result assets\n\
- <span color=\"#66CCFF\" size=\"10\">Open</> - opening all result assets\n\
- <span color=\"#66CCFF\" size=\"10\">Remove</> - removing all result assets\n\
- <span color=\"#66CCFF\" size=\"10\">Dirty</> - marking all result assets as dirty\n\
\n\
II) multiple action based on the first row:\n\
- <span color=\"#66CCFF\" size=\"10\">Duplicate</> - creating N copies (Multiple action count) of first item in the result\n\
\n\
pasting and specific asset selection in Content Browser.\n\
\n\
\n\
<span size=\"12\">Macro:</>\n\
<span size=\"4\">------------------------------------</>\n\
Macro is the category which allows you to create <span color=\"#66CCFF\" size=\"10\">sets</> of assets. You can use <span color=\"#66CCFF\" size=\"10\">Autosave</> feature and define how often opened windows will be save, but also save current result list.\n\
Every set has number and items count. After selecting one of the assets you are able to <span color=\"#66CCFF\" size=\"10\">load</> it to the results, <span color=\"#66CCFF\" size=\"10\">find</> in the Content Browser and <span color=\"#66CCFF\" size=\"10\">open</> it.\n\
\n\
\n\
<span size=\"12\">Extra:</>\n\
<span size=\"4\">------------------------------------</>\n\
Extra category stores some useful settings of the plugin. Here you can set <span color=\"#66CCFF\" size=\"10\">hotkey pluggin</> (position in the MODS tab)\n\
In the future you will be able to check useful data about the project, additional statistic, names of user defined Structs.\n\
");

	return SNew(SDockTab)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SRichTextBlock)
			.Text(FriendlyName)
			.TextStyle(FEditorStyle::Get(), "Credits.Strong")
			.DecoratorStyleSet(&FEditorStyle::Get())
			.Justification(ETextJustify::Center)
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SRichTextBlock)
			.Text(VersionName)
			.TextStyle(FEditorStyle::Get(), "Credits.Normal")
			.DecoratorStyleSet(&FEditorStyle::Get())
			
			.Justification(ETextJustify::Center)
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SRichTextBlock)
			.Text(CreatedBy)
			.DecoratorStyleSet(&FEditorStyle::Get())
			.Justification(ETextJustify::Center)
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.Padding(2, 5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4.0f)
			[
				SNew(SRichTextBlock)
				.Text(FText::FromString("How does plugin work:"))
				.TextStyle(FEditorStyle::Get(), "Credits.H5")
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(250.0f)
				[
					SNew(SRichTextBlock)
					.Text(FText::FromString("ChangeLogs:"))
					.TextStyle(FEditorStyle::Get(), "Credits.H5")
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
				]
			]
		]

		+ SVerticalBox::Slot()
		.Padding(2, 5)
		.FillHeight(1.0f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot().Padding(0, 5)
				[
					SNew(SRichTextBlock)
					.Text(HowPluginWorks)
					.Decorators(CreatedDecorators)
					.AutoWrapText(true)					
				]
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.Padding(FMargin(4.0f, 4.0f, 4.0f, -14.0f))
				.WidthOverride(250.0f)
				[
					SNew(SMultiLineEditableTextBox)
					.Margin(FMargin(4.0f))
					.AutoWrapText(true)
					.Padding(4.0f)
					.Style(FEditorStyle::Get(), "Log.TextBox")
					.TextStyle(FEditorStyle::Get(), "Log.Normal")
					.IsReadOnly(true)
					.Text(ChangeLog)
						
				]
			]
		]
	];
}

void FAssetAssistant::ShutdownModule()
{
	FAssetAssistantEditorCommands::Unregister();
	FAssetAssistantStyle::Shutdown();
	FEditorModeRegistry::Get().UnregisterMode(FAssetAssistantEdMode::EM_AssetAssistantEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetAssistant, AssetAssistant)