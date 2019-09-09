// Copyright 2017 Tefel. All Rights Reserved.

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

#define LOCTEXT_NAMESPACE "AssetAssistant.UISetting"

TSharedRef<IDetailCustomization> FAssetAssistantEditorUISettingCustomization_Modify::MakeInstance(class FAssetAssistantEdMode* InEditMode)
{
	FAssetAssistantEditorUISettingCustomization_Modify *AssetModify = new FAssetAssistantEditorUISettingCustomization_Modify();
	auto Instance = MakeShareable(AssetModify);
	EditMode = InEditMode;
	EditMode->AssetModify = AssetModify;
	return Instance;
}

void FAssetAssistantEditorUISettingCustomization_Modify::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Modify);
	IDetailCategoryBuilder& PatternCategory = DetailBuilder.EditCategory("Pattern", LOCTEXT("Pattern", "Pattern"), ECategoryPriority::Default);

	// -- Combo patterns
	PatternCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(100)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Pattern", "Pattern:"))
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1)
		[
			SAssignNew(AssistantComboPatterns, SAssistantCombo)
			.OptionNames(EditMode->UISetting->PatternNames)
		]
		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("RemoveSelectedPatternTooltip", "Remove selected pattern"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Delete"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnPatternRemoveClicked)
		]
	];

	// -- Pattern text
	PatternCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(120)
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
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, PatternText)))
				.ShouldDisplayName(false)
				.ToolTipText(LOCTEXT("NewPatternTooltip", "Add new replacing/naming pattern"))
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(2, 2)
		.AutoWidth()
		[
			SNew(SItemButton)
			.ToolTipText(LOCTEXT("NewPatternTooltip", "Add new replacing/naming pattern"))
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_AddToArray"))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnPatternAddClicked)
		]
	];

	PatternCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, FragmentToReplace)));
	PatternCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, MultipleActionCount)));
	PatternCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, Arr1)));
	PatternCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, Arr2)));

	
	// Modify Category
	IDetailCategoryBuilder& ModifyCategory = DetailBuilder.EditCategory("Modify", LOCTEXT("Modify", "Modify"), ECategoryPriority::Default);
	ModifyCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(100)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("AssetActions", "Single actions:"))
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(this, &FAssetAssistantEditorUISettingCustomization_Modify::GetSingleActionText)
		]
	];

	ModifyCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		// -- Find Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "FindTooltip", "Find all Result assets" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyFindClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Search)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "Search", "Search" ) )
				]
			]
		]

		// -- Rename Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "RenameTooltip", "Rename all Result assets by applying selected pattern" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyRenameClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Pencil_Square_O)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "Rename", "Rename" ) )
				]
			]
		]

		// -- Save Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "SaveTooltip", "Save all assets listed in the Results" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifySaveClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Hdd_O)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "Save", "Save" ) )
				]
			]
		]

		// -- Open Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "OpenTooltip", "Open all assets listed in the Results" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyOpenClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::External_Link)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.Text( LOCTEXT( "Open", "Open" ) )
				]
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
	];


	ModifyCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		// -- Remove Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "RemoveTooltip", "Remove all assets listed in the Results" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyRemoveClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Times)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "Remove", "Remove" ) )
				]
			]
		]

		// -- Mark Dirty Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "MarkDirtyTooltip", "Mark as dirty all assets listed in the Results" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyMarkDirtyClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Thumb_Tack)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "Dirty", "Dirty" ) )
				]
			]
		]

		// -- Auto Consolidate Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "AutoConsolidateTooltip", "Try to automaticly consolidate similar files in the project" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyAutoConsolidateClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Angellist)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "AutoConsolidate", "AutoConsolidate" ) )
				]
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
	];


	ModifyCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(100)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("BlueprintActions", "Multiple action:"))
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(this, &FAssetAssistantEditorUISettingCustomization_Modify::GetMultipleActionText)
			
		]
	];


	ModifyCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		// -- Duplicate
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "DuplicateTooltip", "Creating [MultipleActionCount] copies of first asset in the Results." ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Modify::OnModifyDuplicateClicked)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Files_O)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
					.Text( LOCTEXT( "Duplicate", "Duplicate" ) )
				]
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
	];
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnPatternAddClicked()
{
	if (!EditMode->UISetting->PatternNames.Contains(EditMode->UISetting->PatternText))
	{
		EditMode->UISetting->PatternNames.Insert(EditMode->UISetting->PatternText, 0);
		AssistantComboPatterns->Initialize(EditMode->UISetting->PatternNames);
	}
	EditMode->UISetting->PatternText = "";

	EditMode->UISetting->Save();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnPatternRemoveClicked()
{
	if (EditMode->UISetting->PatternNames.Num() > 0)
	{
		EditMode->UISetting->PatternNames.Remove(*AssistantComboPatterns->CurrentItem);
		AssistantComboPatterns->Initialize(EditMode->UISetting->PatternNames);
	}
	
	EditMode->UISetting->Save();
	return FReply::Handled();
}


FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyFindClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;

	for (auto AssetItem : EditMode->ViewResult->ResultListWidget->AssetItems)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetItem->AssetPath));
		AssetDataList.Add(XAssetData);
	}
	GEditor->SyncBrowserToObjects(AssetDataList);
	return FReply::Handled();
}


FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyRenameClicked()
{
	TArray<FString> PatternList = GeneratePatternList(*AssistantComboPatterns->CurrentItem, EditMode->ViewResult->ResultListWidget->AssetItems.Num(), false);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetsAndNames;
	TArray<FAssetFile> AssetFiles;
	for (int32 AssetIndex = 0; AssetIndex <  EditMode->ViewResult->ResultListWidget->AssetItems.Num(); AssetIndex++)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*EditMode->ViewResult->ResultListWidget->AssetItems[AssetIndex]->AssetPath));
		UObject* Asset = XAssetData.GetAsset();

		if (!Asset)
		{
			continue;
		}

		const FString PackagePath = FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName());
		FString NewAssetName = PatternList[AssetIndex];
		// only fragment
		if (EditMode->UISetting->FragmentToReplace != "")
		{
			NewAssetName = Asset->GetName().Replace(*EditMode->UISetting->FragmentToReplace, *PatternList[AssetIndex]);
		}
		AssetsAndNames.Add(FAssetRenameData(Asset, PackagePath, NewAssetName));

		// create AssetFile to refresh list
		FAssetFile AssetFile = FAssetFile();
		AssetFile.AssetName = NewAssetName;
		AssetFile.AssetClass = Asset->GetClass()->GetName();
		AssetFile.AssetPath = PackagePath / NewAssetName + "." + NewAssetName;
		AssetFiles.Add(AssetFile);
	}
	AssetToolsModule.Get().RenameAssets(AssetsAndNames);
	EditMode->ViewResult->ResultListWidget->Initialize(AssetFiles);

	return FReply::Handled();
}

void FAssetAssistantEditorUISettingCustomization_Modify::Recover()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath("/Game/aaaa.aaaa");
	AssetDataList.Add(XAssetData);

	// Sort all selected assets by class
	TMap<UClass*, TArray<UObject*> > SelectedAssetsByClass;
	for (const auto& SelectedAsset : AssetDataList)
	{
		auto Asset = SelectedAsset.GetAsset();
		auto AssetClass = Asset->GetClass();

		if (!SelectedAssetsByClass.Contains(AssetClass))
		{
			SelectedAssetsByClass.Add(AssetClass);
		}

		SelectedAssetsByClass[AssetClass].Add(Asset);
	}

	for (const auto& AssetsByClassPair : SelectedAssetsByClass)
	{
		const auto& TypeAssets = AssetsByClassPair.Value;
		FAssetEditorManager::Get().OpenEditorForAssets(TypeAssets);
	}
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyOpenClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	for (auto AssetItem : EditMode->ViewResult->ResultListWidget->AssetItems)
	{

		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetItem->AssetPath));
		AssetDataList.Add(XAssetData);
	}

	// Sort all selected assets by class
	TMap<UClass*, TArray<UObject*> > SelectedAssetsByClass;
	for (const auto& SelectedAsset : AssetDataList)
	{
		auto Asset = SelectedAsset.GetAsset();
		if (IsValid(Asset))
		{
			auto AssetClass = Asset->GetClass();

			if (!SelectedAssetsByClass.Contains(AssetClass))
			{
				SelectedAssetsByClass.Add(AssetClass);
			}

			SelectedAssetsByClass[AssetClass].Add(Asset);
		}
	}

	for (const auto& AssetsByClassPair : SelectedAssetsByClass)
	{
		const auto& TypeAssets = AssetsByClassPair.Value;
		FAssetEditorManager::Get().OpenEditorForAssets(TypeAssets);
	}

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifySaveClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	TArray<UPackage*> PackagesToSave;
	for (auto AssetItem : EditMode->ViewResult->ResultListWidget->AssetItems)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetItem->AssetPath));
		PackagesToSave.Add(XAssetData.GetPackage());
	}
	bool bCheckDirty = false;
	bool bPromptToSave = true;
	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, bCheckDirty, bPromptToSave);

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyRemoveClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<UObject*> AssetsToRemove;
	for (int32 AssetIndex = 0; AssetIndex < EditMode->ViewResult->ResultListWidget->AssetItems.Num(); AssetIndex++)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*EditMode->ViewResult->ResultListWidget->AssetItems[AssetIndex]->AssetPath));
		AssetsToRemove.Add(XAssetData.GetAsset());
	}
	ObjectTools::DeleteObjects(AssetsToRemove);

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyMarkDirtyClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;

	TArray<UObject*> ObjectsToConsolidate;
	for (auto AssetItem : EditMode->ViewResult->ResultListWidget->AssetItems)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetItem->AssetPath));
		UObject* Asset = XAssetData.GetAsset();
		Asset->GetOuter()->MarkPackageDirty();	
	}

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyAutoConsolidateClicked()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	FARFilter Filter = FARFilter();
	Filter.PackagePaths.Add(FName("/Game"));
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	TArray<FAssetData> AssetDataList;
	for (auto AssetItem : EditMode->ViewResult->ResultListWidget->AssetItems)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetItem->AssetPath));
		FName CheckAssetName = XAssetData.AssetName;
		TArray<UObject*> ObjectsToConsolidate = TArray<UObject*>();
		for (TArray<FAssetData>::TConstIterator PkgIter = AssetData.CreateConstIterator(); PkgIter; ++PkgIter)
		{
			int32 LastDiskSizeToCheck = -1;
			float Tolerance = 0.1;
			if (PkgIter->AssetName.ToString().Contains(CheckAssetName.ToString()))
			{
				const FAssetPackageData* PackageData = AssetRegistryModule.Get().GetAssetPackageData(PkgIter->PackageName);
	
				if (PackageData && LastDiskSizeToCheck == -1)
				{
					UObject* Asset = PkgIter->GetAsset();
					ObjectsToConsolidate.Add(Asset);
				}
				else if (PackageData && LastDiskSizeToCheck != 0)
				{
					int32 Threshold = LastDiskSizeToCheck * Tolerance;
					if (UKismetMathLibrary::InRange_IntInt(PackageData->DiskSize, LastDiskSizeToCheck - Threshold, LastDiskSizeToCheck + Threshold, true, true))
					{
						UObject* Asset = PkgIter->GetAsset();
						ObjectsToConsolidate.Add(Asset);
					}
				}
				
				LastDiskSizeToCheck = PackageData->DiskSize;
			}
		}

		TArray<UObject*> CompatibleObjects = TArray<UObject*>();
		FConsolidateToolWindow::DetermineAssetCompatibility(ObjectsToConsolidate, CompatibleObjects);
		if (CompatibleObjects.Num() >  1)
		{
			FConsolidateToolWindow::AddConsolidationObjects(CompatibleObjects);
			return FReply::Handled();
		}
	}

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyCompileClicked()
{
	return FReply::Handled();
}

// todo start using it?
FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyCreateChildClicked()
{
	TArray<FString> PatternList = GeneratePatternList(*AssistantComboPatterns->CurrentItem, EditMode->ViewResult->ResultListWidget->AssetItems.Num(), true);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<UObject*> AssetsToRemove;
	for (int32 AssetIndex = 0; AssetIndex < EditMode->ViewResult->ResultListWidget->AssetItems.Num(); AssetIndex++)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*EditMode->ViewResult->ResultListWidget->AssetItems[AssetIndex]->AssetPath));
		UBlueprint * Blueprint = Cast<UBlueprint>(XAssetData.ToSoftObjectPath().ResolveObject());
		
		if (Blueprint)
		{
			UBlueprintFactory* BlueprintFactory = NewObject<UBlueprintFactory>();
			BlueprintFactory->ParentClass = Blueprint->GeneratedClass;
			UFactory* Factory = BlueprintFactory;
			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			const FString PackagePath = FPackageName::GetLongPackagePath(Blueprint->GetOutermost()->GetName());
			ContentBrowserModule.Get().CreateNewAsset(PatternList[AssetIndex], PackagePath, Blueprint->GetClass(), Factory);
			
		}
	}

	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Modify::OnModifyDuplicateClicked()
{
	TArray<FString> PatternList = GeneratePatternList(*AssistantComboPatterns->CurrentItem, EditMode->UISetting->MultipleActionCount, true);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetsAndNames;

	for (int32 Index = 0; Index < EditMode->UISetting->MultipleActionCount; Index++)
	{
		FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*EditMode->ViewResult->ResultListWidget->AssetItems[0]->AssetPath));
		UObject* Asset = XAssetData.GetAsset();
		const FString PackagePath = FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName());
		AssetToolsModule.Get().DuplicateAsset(PatternList[Index], PackagePath, Asset);

	}

	return FReply::Handled();
}

TArray<FString> FAssetAssistantEditorUISettingCustomization_Modify::GeneratePatternList(const FString PatternIn, int32 ItemsCount, bool isDuplicate)
{
	TArray<FString> PatternList = TArray<FString>();
	bool date = PatternIn.Contains("{date}", ESearchCase::IgnoreCase);
	bool time = PatternIn.Contains("{time}", ESearchCase::IgnoreCase);
	bool hasArr1 = PatternIn.Contains("{arr1}", ESearchCase::IgnoreCase);
	bool hasArr2 = PatternIn.Contains("{arr2}", ESearchCase::IgnoreCase);
	bool name = PatternIn.Contains("{name}", ESearchCase::IgnoreCase);
	bool nr0 = PatternIn.Contains("{nr}", ESearchCase::IgnoreCase);
	bool nr = PatternIn.Contains("{nr", ESearchCase::IgnoreCase);

	FDateTime DateTime = FDateTime().Now();
	for (int32 ItemIndex = 0; ItemIndex < ItemsCount; ItemIndex++)
	{
		int32 PattIndex = (isDuplicate) ? 0 : ItemIndex;
		FString Pattern = PatternIn;
		if (date)
		{
			Pattern = Pattern.Replace(*FString("{date}"), *DateTime.ToString(TEXT("%Y-%m-%d")));
		}
		if (time)
		{
			Pattern = Pattern.Replace(*FString("{time}"), *DateTime.ToString(TEXT("%Hh%Mm%Ss")));
		}
		if (hasArr1)
		{
			FString arrName1 = EditMode->UISetting->Arr1.IsValidIndex(ItemIndex) ? EditMode->UISetting->Arr1[ItemIndex] : "";
			Pattern = Pattern.Replace(*FString("{arr1}"), *arrName1);
		}
		if (hasArr2)
		{
			FString arrName2 = EditMode->UISetting->Arr2.IsValidIndex(ItemIndex) ? EditMode->UISetting->Arr2[ItemIndex] : "";
			Pattern = Pattern.Replace(*FString("{arr1}"), *arrName2);
		}
		if (name)
		{
			Pattern = Pattern.Replace(*FString("{name}"), *EditMode->ViewResult->ResultListWidget->AssetItems[PattIndex]->AssetName);
		}
		if (nr0)
		{
			Pattern = Pattern.Replace(*FString("{nr}"), *FString::FromInt(ItemIndex));
		}
		if (nr)
		{
			while (Pattern.Find("{nr") != -1)
			{
				int32 Index = Pattern.Find("{nr");
				if (Pattern.GetCharArray().IsValidIndex(Index + 3) && (Pattern.GetCharArray()[Index + 3] == '+' || Pattern.GetCharArray()[Index + 3] == '-'))
				{
					TCHAR sign = Pattern.GetCharArray()[Index + 3];
					int32 IndexEnd = -1;
					for (int32 i = Index + 3; Index < Pattern.GetCharArray().Num(); i++)
					{
						if (Pattern.GetCharArray()[i] == '}')
						{
							IndexEnd = i;
							break;
						}
					}
					int32 Number = FCString::Atoi(*Pattern.Mid(Index + 4, IndexEnd - Index - 4));
					Number = (sign == '+') ? ItemIndex + Number : ItemIndex - Number;
					Pattern = Pattern.Replace(*Pattern.Mid(Index, IndexEnd - Index + 1), *FString::FromInt(Number));
				}
			}
		}
		PatternList.Add(Pattern);
	}

	return PatternList;
}

FText FAssetAssistantEditorUISettingCustomization_Modify::GetSingleActionText() const
{
	return FText::Format(LOCTEXT("SingleActionText", "Executed for all rows ({0})"), EditMode->ViewResult->ResultListWidget->AssetItems.Num());
}

FText FAssetAssistantEditorUISettingCustomization_Modify::GetMultipleActionText() const
{
	if (EditMode->ViewResult->ResultListWidget->AssetItems.Num() > 0)
	{
		return FText::Format(LOCTEXT("MultipleActionText", "Executed for first row ({0}) multiple times ({1}) "), FText::FromString(EditMode->ViewResult->ResultListWidget->AssetItems[0]->AssetName), EditMode->UISetting->MultipleActionCount);
	}
	return FText::FromString("First row is missing! Can't execute the action.");
}

#undef LOCTEXT_NAMESPACE