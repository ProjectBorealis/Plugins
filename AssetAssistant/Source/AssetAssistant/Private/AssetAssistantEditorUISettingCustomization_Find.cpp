// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEditorUISettingCustomization_Find.h"
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
#include "Slates/SAssistantRichCombo.h"
#include "Slates/SAssistantList.h"

#define LOCTEXT_NAMESPACE "AssetAssistant.UISetting"

TSharedRef<IDetailCustomization> FAssetAssistantEditorUISettingCustomization_Find::MakeInstance(class FAssetAssistantEdMode* InEditMode)
{
	FAssetAssistantEditorUISettingCustomization_Find *AssetFind = new FAssetAssistantEditorUISettingCustomization_Find();
	auto Instance = MakeShareable(AssetFind);
	EditMode = InEditMode;
	EditMode->AssetFind = AssetFind;
	return Instance;
}

TSharedRef<SWidget>	FAssetAssistantEditorUISettingCustomization_Find::GetListWidget()
{
	return SNew(SBox);
}

void FAssetAssistantEditorUISettingCustomization_Find::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Find);

	IDetailCategoryBuilder& FindCategory = DetailBuilder.EditCategory("Find", LOCTEXT("FindCategory", "Find"), ECategoryPriority::Default);
	TArray<FString> AvaiableClassesNames;
	TArray<int32> AvaiableClassesCounts;
	FAssetAssistantEditorUISettingCustomization::GetAvaiableClasses(AvaiableClassesNames, AvaiableClassesCounts);

	// -- Find Class
	FindCategory.AddCustomRow(FText::GetEmpty())
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
				.Text(LOCTEXT("Type", "Type:"))
			]
		]

	+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1)
		[
			SAssignNew(AssistantComboClasses, SAssistantRichCombo)
			.OptionNames(AvaiableClassesNames)
			.OptionCounts(AvaiableClassesCounts)
		]
		];


	// -- Find Directory Tree
	FindCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2,0)
		.AutoWidth()
		.VAlign(VAlign_Top)
		[
			SNew(SBox)
			.WidthOverride(100)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Font(DetailBuilder.GetDetailFont())
					.Text(LOCTEXT("Directory", "Directory:"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.Padding(2, 0)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(20.0f)
						.HeightOverride(20.0f)
						[
							SNew(SItemButton)
							.Text(LOCTEXT("Expand_all_items", "Expand all items"))
							.ToolTipText(LOCTEXT("Expand_all_items", "Expand all items"))
							.Image(FEditorStyle::GetBrush("Profiler.EventGraph.ExpandAll"))
							.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Find::OnExpandDirectoriesClicked)
						]
					]

				+ SHorizontalBox::Slot()
					.Padding(2, 0)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(20.0f)
						.HeightOverride(20.0f)
						[
							SNew(SItemButton)
							.Text(LOCTEXT("Collapse_all_items", "Collapse all items"))
							.ToolTipText(LOCTEXT("Collapse_all_items", "Collapse all items"))
							.Image(FEditorStyle::GetBrush("Profiler.EventGraph.CollapseAll"))
							.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Find::OnCollapseDirectoriesClicked)
						]
					]
				]
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1)
		[
			SAssignNew(AssistantTree, SAssistantTree)
		]
	];

	// -- Find From
	FindCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(122)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Find_from", "Find from:"))
		]
		]

	+ SHorizontalBox::Slot()
		.Padding(1, 0)
		.FillWidth(1)
		.VAlign(VAlign_Center)
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, FindFrom)))
			.ShouldDisplayName(false)
		]
	];


	// -- Find Text
	FindCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(122)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Text", "Text:"))
		]
		]

	+ SHorizontalBox::Slot()
		.Padding(3, 2)
		.FillWidth(1)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.Padding(2)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAssetAssistantEditorUISetting, TextToFind)))
				.ShouldDisplayName(false)
			]
		]
		];

	// Button Find
	FindCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew( SHorizontalBox )
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew( SButton )
				.ButtonStyle(FEditorStyle::Get(), "RoundButton")
				.ToolTipText( LOCTEXT( "FindAssetsTooltip", "Find assets using filter from Find category" ) )
				.ContentPadding(FMargin(7, 3, 7, 2))
				.OnClicked(this, &FAssetAssistantEditorUISettingCustomization_Find::OnFindButtonClicked)
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
						.Font(FEditorStyle::Get().GetFontStyle("TinyText"))
						.Text( LOCTEXT( "FindFiles", "Find files" ) )
					]
				]
			]
	];
}

FReply FAssetAssistantEditorUISettingCustomization_Find::OnFindButtonClicked()
{
	FAssetRegistryModule* AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetData;
	FARFilter Filter = FARFilter();

	// Classes
	if (*EditMode->AssetFind->AssistantComboClasses->CurrentItem != "" && *EditMode->AssetFind->AssistantComboClasses->CurrentItem != "All Types")
	Filter.ClassNames.Add(FName(**EditMode->AssetFind->AssistantComboClasses->CurrentItem));
	
	FString Directory = EditMode->AssetFind->AssistantTree->GetSelectedDirectory()->GetDirectoryPath();
	FString DirPrefix = Directory.IsEmpty() ? "/Game" : "/Game/";
	Filter.PackagePaths.Add(FName(*(DirPrefix + Directory)));

	// Flags
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	AssetRegistryModule->Get().GetAssets(Filter, AssetData);

	// Prepare names from content browser
	TArray<FString> ContentBrowserAssetNames;
	if (EditMode->UISetting->FindFrom == EFindFrom::SelectedInContentBrowser)
	{
		TArray<FAssetData> ContentBrowserClasses;
		GEditor->GetContentBrowserSelections(ContentBrowserClasses);
		for (auto CBClass : ContentBrowserClasses)
			ContentBrowserAssetNames.Add(CBClass.AssetName.ToString());
	}

	TArray<FAssetFile> AssetFiles;
	TArray<FName> ClassNames; // test
	TSet<FName> DerivedClassNames; // test
	for (TArray<FAssetData>::TConstIterator PkgIter = AssetData.CreateConstIterator(); PkgIter; ++PkgIter)
	{
		FAssetFile AssetFile = FAssetFile();
		AssetFile.AssetName = *PkgIter->AssetName.ToString();
		AssetFile.AssetClass = *PkgIter->AssetClass.ToString();
		AssetFile.AssetPath = *PkgIter->ObjectPath.ToString();
		
		ClassNames.AddUnique(FName(*AssetFile.AssetClass));

		FAssetData Asset = *PkgIter;
		//Asset.GetPackage()->HasThumbnailMap()
		if (EditMode->UISetting->TextToFind.IsEmpty() || AssetFile.AssetName.Contains(EditMode->UISetting->TextToFind))
		{
			// make sure that class is the same like searching one animBlueprint Blueprint etc
			if (AssetFile.AssetClass == *EditMode->AssetFind->AssistantComboClasses->CurrentItem || *EditMode->AssetFind->AssistantComboClasses->CurrentItem == "All Types")
			{
				if (EditMode->UISetting->FindFrom == EFindFrom::AllFiles || ContentBrowserAssetNames.Contains(AssetFile.AssetName))
				{
					AssetFiles.Add(AssetFile);
				}
			}
		}
	}

	EditMode->ViewResult->ResultListWidget->Initialize(AssetFiles);
	EditMode->SaveResultList();

	if (EditMode)
	{
		FString MsgClassName = "", MsgText = "", MsgPath = "";
		if (Filter.ClassNames.Num() > 0)
		{
			MsgClassName = " " + Filter.ClassNames[0].ToString() + " ";
		}

		if (!EditMode->UISetting->TextToFind.IsEmpty())
		{
			MsgText = "contains \""  + EditMode->UISetting->TextToFind + "\"";
		}

		if (Filter.PackagePaths.Num() > 0)
		{
			MsgPath = "\"" + Filter.PackagePaths[0].ToString() + "\"";
		}

		int32 TotalCount = EditMode->ViewResult->ResultListWidget->AssetItems.Num();
		FText Message = FText::Format(LOCTEXT("AssetsFoundMessage", "Found {0} {1} files in {2} {3}"), TotalCount, FText::FromString(MsgClassName), FText::FromString(MsgPath), FText::FromString(MsgText));
		EditMode->NotifyMessage(Message);
	}
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Find::OnContentButtonClicked()
{
	FAssetRegistryModule* AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetData;
	FARFilter Filter = FARFilter();
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName("/Game"));
	AssetRegistryModule->Get().GetAssets(Filter, AssetData);

	//Prepare names from content browser
	TArray<FString> ContentBrowserAssetPaths;
	TArray<FAssetData> ContentBrowserClasses;
	GEditor->GetContentBrowserSelections(ContentBrowserClasses);
	for (auto CBClass : ContentBrowserClasses)
		ContentBrowserAssetPaths.Add(CBClass.ObjectPath.ToString());
	
	TArray<FAssetFile> AssetFiles;
	for (TArray<FAssetData>::TConstIterator PkgIter = AssetData.CreateConstIterator(); PkgIter; ++PkgIter)
	{
		FAssetFile AssetFile = FAssetFile();
		AssetFile.AssetName = *PkgIter->AssetName.ToString();
		AssetFile.AssetClass = *PkgIter->AssetClass.ToString();
		AssetFile.AssetPath = *PkgIter->ObjectPath.ToString();
		if (ContentBrowserAssetPaths.Contains(AssetFile.AssetPath))
		{
			AssetFiles.Add(AssetFile);
		}
	}
	EditMode->ViewResult->ResultListWidget->Initialize(AssetFiles);
	EditMode->SaveResultList();

	FText Message = FText::Format(LOCTEXT("AssetsFoundMessage", "Obtained {0} Content Browser assets"), EditMode->ViewResult->ResultListWidget->AssetItems.Num());
	EditMode->NotifyMessage(Message);
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Find::OnCollapseDirectoriesClicked()
{
	EditMode->AssetFind->AssistantTree->CollapseAll();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUISettingCustomization_Find::OnExpandDirectoriesClicked()
{
	EditMode->AssetFind->AssistantTree->ExpandAll();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE