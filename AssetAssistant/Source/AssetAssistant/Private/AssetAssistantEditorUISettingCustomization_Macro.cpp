// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEditorUISettingCustomization_Macro.h"
#include "AssetAssistantEditorUISettingCustomization_Modify.h"
#include "AssetAssistantEditorUISetting.h"
#include "AssetAssistantEditorUIView.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "ObjectTools.h"
#include "Factories/BlueprintFactory.h"
#include "FileHelpers.h"
#include "ConsolidateWindow.h"
#include "Kismet/KismetMathLibrary.h"
#include "Toolkits/AssetEditorManager.h"
#include "EditorStyleSet.h"
#include "EditorFontGlyphs.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "Slates/SItemButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyCustomizationHelpers.h"
#include "Slates/SAssistantCombo.h"

#define LOCTEXT_NAMESPACE "AssetAssistant.UISetting"

TSharedRef<IDetailCustomization> FAssetAssistantEditorUISettingCustomization_Macro::MakeInstance(class FAssetAssistantEdMode* InEditMode)
{
	FAssetAssistantEditorUISettingCustomization_Macro *AssetMacro = new FAssetAssistantEditorUISettingCustomization_Macro();
	auto Instance = MakeShareable(AssetMacro);
	EditMode = InEditMode;
	EditMode->AssetMacro = AssetMacro;
	return Instance;
}

void FAssetAssistantEditorUISettingCustomization_Macro::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Macro);

	IDetailCategoryBuilder& AutosaveRecoveryCategory = DetailBuilder.EditCategory("AutosaveRecoveryCategory", LOCTEXT("AutosaveRecoveryCategory", "Auttosave recovery"), ECategoryPriority::Default);
	// -- Auto-saves
	AutosaveRecoveryCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(120)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("AutosaveWindows", "Autosave window sets:"))
		]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1)
		[
			SAssignNew(AssistantComboAutosaveSets, SAssistantCombo)
			.OptionNames(EditMode->UISetting->AutosaveSetsIDs)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("CreateAutosaveTooltip", "Create new autosave from opened asset windows"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_AddToArray"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnCreateAutosaveClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("ClearAutosavesTooltip", "Clear all autosaves"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_EmptyArray"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnClearAutosavesClicked)
		]
	];

		// Macro set actions
	AutosaveRecoveryCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)

		// -- Load asset set to result
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "LoadTooltip", "Load assets from Set to the Results" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnAutosaveLoadClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Arrow_Down) // Arrow_Circle_O_Down
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
					.Text( LOCTEXT( "Load", "Load" ) )
				]
			]
		]

		// -- Load set & find
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "LoadFindTooltip", "Load assets from selected set and find them in the content browser" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnAutosaveLoadFindClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Search_Plus)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
					.Text( LOCTEXT( "Find", "Find" ) )
				]
			]
		]

		// -- Load set & open
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "LoadOpenTooltip", "Load assets from Set to the Results and open them." ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnAutosaveLoadOpenClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Folder_Open)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
					.Text( LOCTEXT( "Open", "Open" ) )
				]
			]
		]
	];

	IDetailCategoryBuilder& SetsRecoveryCategory = DetailBuilder.EditCategory("SetsRecoveryCategory", LOCTEXT("SetsRecoveryCategory", "Sets recovery"), ECategoryPriority::Default);
	SetsRecoveryCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(120)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("AssetSets", "Asset sets:"))
		]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1)
		[
			SAssignNew(AssistantComboSets, SAssistantCombo)
			.OptionNames(EditMode->UISetting->SetsIDs)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("RemoveSelectedSetTooltip", "Remove selected set"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Delete"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnSetRemoveClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("ClearSetsTooltip", "Clear custom created sets"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_EmptyArray"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnClearSetsClicked)
		]
	];

	// -- Combo name
	SetsRecoveryCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(160)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("New", "New:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1.0f)
		[
			SNew(SBox)
			.Padding(2)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, SetText)))
				.ShouldDisplayName(false)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("SetAddTooltip", "Save asset set with name"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_AddToArray"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnSetAddClicked)
		]
	];

	// Macro set actions
	SetsRecoveryCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)

		// -- Load asset set to result
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "LoadTooltip", "Load assets from Set to the Results" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnSetLoadClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Arrow_Down) // Arrow_Circle_O_Down
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
					.Text( LOCTEXT( "Load", "Load" ) )
				]
			]
		]

		// -- Load set & find
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "LoadFindTooltip", "Load assets from selected set and find them in the content browser" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnSetLoadFindClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Search_Plus)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
					.Text( LOCTEXT( "Find", "Find" ) )
				]
			]
		]

		// -- Load set & open
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "LoadOpenTooltip", "Load assets from Set to the Results and open them." ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Macro::OnSetLoadOpenClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Folder_Open)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
					.Text( LOCTEXT( "Open", "Open" ) )
				]
			]
		]
	];

	IDetailCategoryBuilder& AutosaveConfigCategory = DetailBuilder.EditCategory("AutosaveConfig", LOCTEXT("AutosaveConfig", "Autosave configuration"), ECategoryPriority::Default);
	AutosaveConfigCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(170)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("TurnOnAutosave", "Recover Windows on start from:"))
		]
		]

	+ SHorizontalBox::Slot()
		.Padding(1, 0)
		.FillWidth(1)
		.VAlign(VAlign_Center)
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, AutosaveRecoverType)))
			.ShouldDisplayName(false)
		]
	];

	// -- Combo name
	AutosaveConfigCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("CreateAutosaveEvery", "Create autosave every "))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(60.0f)
			.Padding(2)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, AutosaveRepeatRate)))
				.ShouldDisplayName(false)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("seconds", "seconds. "))
		]

		+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
	];
}

void FAssetAssistantEditorUISettingCustomization_Macro::AddNewSet(FString textToUse)
{
	EditMode->UISetting->LastSetNR++;

	int32 Count;
	EditMode->UISetting->CurrentSetTArrayText = EditMode->ViewResult->GetTArrayToCopy(Count).Replace(TEXT("\r\n"), TEXT(""));
	FString SetName = "#" + FString::FromInt(EditMode->UISetting->LastSetNR) + " " + textToUse + " N" + FString::FromInt(Count);
	EditMode->UISetting->SetsIDs.Insert(SetName, 0);
	AssistantComboSets->Initialize(EditMode->UISetting->SetsIDs);

	EditMode->UISetting->SaveSet(SetName);
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnClearAutosavesClicked()
{
	for (auto autosaveSetID : EditMode->UISetting->AutosaveSetsIDs)
	{
		EditMode->UISetting->RemoveSet(*autosaveSetID);
	}
	EditMode->UISetting->AutosaveSetsIDs.Empty();
	AssistantComboAutosaveSets->Initialize(EditMode->UISetting->AutosaveSetsIDs);

	EditMode->UISetting->Save();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnCreateAutosaveClicked()
{
	TArray<UObject*> OpenedAssets = FAssetEditorManager::Get().GetAllEditedAssets();
	TArray<TSharedPtr<FAssetFile>> AssetFiles;

	if (OpenedAssets.Num() == 0) return FReply::Handled(); // no opened windows we skip save
	for (auto OpenedAsset : OpenedAssets)
	{
		FAssetFile* AssetFile = new FAssetFile();
		AssetFile->AssetName = OpenedAsset->GetName();
		AssetFile->AssetClass = OpenedAsset->GetClass()->GetName();
		AssetFile->AssetPath = OpenedAsset->GetPathName();
		if (!AssetFile->AssetPath.Contains("/Engine/"))
		{
			AssetFiles.Add(MakeShareable(AssetFile));
		}
	}

	int32 Count;
	FString AutosaveText = EditMode->ViewResult->GetTArrayToCopy(Count, AssetFiles).Replace(TEXT("\r\n"), TEXT(""));
	if (AutosaveText == EditMode->UISetting->CurrentSetTArrayText) return FReply::Handled();
	EditMode->UISetting->CurrentSetTArrayText = AutosaveText;
	

	EditMode->UISetting->LastAutosaveSetNR++;
	FString textToUse = FDateTime().Now().ToString(TEXT("[%d-%m-%Y_%H:%M:%S]"));

	FString SetName = "A#" + FString::FromInt(EditMode->UISetting->LastAutosaveSetNR) + " " + textToUse + " N" + FString::FromInt(Count);
	EditMode->UISetting->AutosaveSetsIDs.Insert(SetName, 0);

	if (EditMode->UISetting->AutosaveSetsIDs.Num() > 10)
	{
		EditMode->UISetting->RemoveSet(*EditMode->UISetting->AutosaveSetsIDs[10]);
		EditMode->UISetting->AutosaveSetsIDs.SetNum(10, true); // max 10 for autosave
	}

	if (GLevelEditorModeTools().GetActiveMode(FAssetAssistantEdMode::EM_AssetAssistantEdModeId) != nullptr)
	{
		AssistantComboAutosaveSets->Initialize(EditMode->UISetting->AutosaveSetsIDs);
	};

	EditMode->UISetting->SaveSet(SetName);
	EditMode->UISetting->Save();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnSetAddClicked()
{
	AddNewSet(EditMode->UISetting->SetText);
	
	EditMode->UISetting->Save();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnSetRemoveClicked()
{
	EditMode->UISetting->RemoveSet(*AssistantComboSets->CurrentItem);
	EditMode->UISetting->SetsIDs.Remove(*AssistantComboSets->CurrentItem);
	AssistantComboSets->Initialize(EditMode->UISetting->SetsIDs);


	EditMode->UISetting->Save();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnClearSetsClicked()
{
	for (auto setID : EditMode->UISetting->SetsIDs)
	{
		EditMode->UISetting->RemoveSet(*setID);
	}
	EditMode->UISetting->SetsIDs.Empty();
	AssistantComboSets->Initialize(EditMode->UISetting->SetsIDs);

	EditMode->UISetting->Save();
	return FReply::Handled();
}

void FAssetAssistantEditorUISettingCustomization_Macro::OnLoad(int32 loadType, bool bShouldSave)
{
	if (loadType == 0)
	{
		if (EditMode->AssetMacro->AssistantComboSets->CurrentItem.IsValid())
		{
			FString SetName = *EditMode->AssetMacro->AssistantComboSets->CurrentItem;
			EditMode->UISetting->LoadSet(SetName);
			EditMode->ViewResult->OnSetImportAssets(bShouldSave);
		}
	}
	else if (loadType == 1)
	{
		if (EditMode->AssetMacro->AssistantComboAutosaveSets->CurrentItem.IsValid())
		{
			FString SetName = *EditMode->AssetMacro->AssistantComboAutosaveSets->CurrentItem;
			EditMode->UISetting->LoadSet(SetName);
			EditMode->ViewResult->OnSetImportAssets(bShouldSave);
		}
	}
}

void FAssetAssistantEditorUISettingCustomization_Macro::OnRecover()
{
	OnLoad(2 - (uint8)EditMode->UISetting->AutosaveRecoverType, false);
	EditMode->AssetModify->OnModifyOpenClicked();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnSetLoadClicked()
{
	OnLoad((uint8)ESetType::AssetSets, true);
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnSetLoadFindClicked()
{
	OnSetLoadClicked();
	EditMode->AssetModify->OnModifyFindClicked();

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnSetLoadOpenClicked()
{	
	OnSetLoadClicked();
	EditMode->AssetModify->OnModifyOpenClicked();

	return FReply::Handled();
}


FReply FAssetAssistantEditorUISettingCustomization_Macro::OnAutosaveLoadClicked()
{
	OnLoad((uint8)ESetType::AssetAutosaves, true);
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnAutosaveLoadFindClicked()
{
	OnAutosaveLoadClicked();
	EditMode->AssetModify->OnModifyFindClicked();

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Macro::OnAutosaveLoadOpenClicked()
{
	OnAutosaveLoadClicked();
	EditMode->AssetModify->OnModifyOpenClicked();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE