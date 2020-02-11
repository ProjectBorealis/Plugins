/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagMaterialImportOptionsWindow.h"
#include "ToolbagImporterPrivatePCH.h"

#include "UnrealEd.h"
#include "IDocumentation.h"
#include "ToolbagImportUI.h"
//#include <SExpandableArea.h>
#include <Slate.h>
#include <Editor/PropertyEditor/Public/PropertyEditorModule.h>

#define LOCTEXT_NAMESPACE "ToolbagImport"

#define METALNESS_INSULATORS_NAME TEXT("Treat all as Insulators")
#define METALNESS_METALS_NAME TEXT("Treat all as Metals")
#define METALNESS_GENERATE_NAME TEXT("Generate Metalness Map")
#define METALNESS_ASK_NAME TEXT("Prompt per Material")

void SToolbagMaterialImportOptionsWindow::Construct(const FArguments& InArgs)
{
	ImportUI = InArgs._ImportUI;
	check (ImportUI);
	TargetPath = InArgs._TargetPath.ToString();
	WidgetWindow = InArgs._WidgetWindow;

	if(ImportUI->MeshesPath.Len() == 0)
		ImportUI->MeshesPath = "./Meshes";
	ImportUI->AbsoluteMeshesPath = ImportUI->MeshesPath;
	if(ImportUI->AbsoluteMeshesPath.RemoveFromStart("."))
	{
		ImportUI->AbsoluteMeshesPath = TargetPath + ImportUI->AbsoluteMeshesPath;
	}

	if(ImportUI->MaterialsPath.Len() == 0)
		ImportUI->MaterialsPath = "./Materials";
	ImportUI->AbsoluteMaterialsPath = ImportUI->MaterialsPath;
	if(ImportUI->AbsoluteMaterialsPath.RemoveFromStart("."))
	{
		ImportUI->AbsoluteMaterialsPath = TargetPath + ImportUI->AbsoluteMaterialsPath;
	}

	if(ImportUI->TexturesPath.Len() == 0)
		ImportUI->TexturesPath = "./Textures";
	ImportUI->AbsoluteTexturesPath = ImportUI->TexturesPath;
	if(ImportUI->AbsoluteTexturesPath.RemoveFromStart("."))
	{
		ImportUI->AbsoluteTexturesPath = TargetPath + ImportUI->AbsoluteTexturesPath;
	}

	MetalnessSourceComboList.Add(MakeShareable( new FString (METALNESS_INSULATORS_NAME) ));
	MetalnessSourceComboList.Add(MakeShareable( new FString (METALNESS_METALS_NAME) ));
	MetalnessSourceComboList.Add(MakeShareable( new FString (METALNESS_GENERATE_NAME) ));
	MetalnessSourceComboList.Add(MakeShareable( new FString (METALNESS_ASK_NAME) ));

	this->ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Font(FEditorStyle::GetFontStyle("CurveEd.LabelFont"))
					.Text(LOCTEXT("Import_CurrentFileTitle", "Current File: "))
				]
				+SHorizontalBox::Slot()
				.Padding(5, 0, 0, 0)
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
					.Text(InArgs._SourcePath)
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged( this, &SToolbagMaterialImportOptionsWindow::OnImportSuppressWarningsChanged )
			.ToolTipText(LOCTEXT("ToolbagImportOptions_SuppressWarnings_ToolTip", "Suppress warning popups, importing the whole scene in one go, so you can grab a coffee while the scene is importing"))
			.IsChecked(ImportUI->bSuppressWarnings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			[
				SNew( STextBlock )
				.Text( LOCTEXT("ToolbagImportOptions_SuppressWarnings", "Suppress Warnings") )
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SExpandableArea)
			.AreaTitle( LOCTEXT("ToolbagImportOptions_MaterialOptions", "Material Options") )
			.InitiallyCollapsed(true)
			.BodyContent()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(SCheckBox)
					.OnCheckStateChanged( this, &SToolbagMaterialImportOptionsWindow::OnDirectImportChanged )
					.ToolTipText(LOCTEXT("ToolbagImportOptions_DirectImport_ToolTip", "If checked Textures and Values are imported directly without doing special processing on the attributes\nThis leads to cleaner material graphs, and higher performance, but removes some flexibility when editing material instances"))
					.IsChecked(ImportUI->bDirectImport ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					[
						SNew( STextBlock )
						.Text( LOCTEXT("ToolbagImportOptions_DirectImport", "Perform direct import") )
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew( STextBlock )
						.Text( LOCTEXT("ToolbagImportOptions_MetalnessConversion", "Metalness Conversion Settings: ") )
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&MetalnessSourceComboList)
						.InitiallySelectedItem(MetalnessSourceComboList[static_cast<int32>(ImportUI->MetalnessConversionOption)])
						.OnSelectionChanged(this, &SToolbagMaterialImportOptionsWindow::OnMetalnessConversionChanged)
						.OnGenerateWidget(this, &SToolbagMaterialImportOptionsWindow::MakeMetalnessConversionComboWidget)
						.Content()
						[
							SAssignNew( MetalnessComboBoxLabel, STextBlock )
							.Text( FText::FromString( *(MetalnessSourceComboList[static_cast<int32>(ImportUI->MetalnessConversionOption)].Get())) )
						]
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SExpandableArea)
			.AreaTitle( LOCTEXT("ToolbagImportOptions_ImportLocations", "Import Locations") )
			.InitiallyCollapsed(true)
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.MinDesiredWidth(100)
						.Font(FEditorStyle::GetFontStyle("CurveEd.LabelFont"))
						.Text(LOCTEXT("ToolbagImportOptions_TexturesPath", "Textures Path: "))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SAssignNew(TexturesPathLabel, STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
						.Text(FText::FromString(ImportUI->TexturesPath))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("ToolbagImportOptions_Change", "Change"))
						.OnClicked(this, &SToolbagMaterialImportOptionsWindow::OnChangeTexturesPath)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(2)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ToolbagImportOptions_ImportAll", "Import All"))
				.ToolTipText(this, &SToolbagMaterialImportOptionsWindow::GetImportAllTooltip)
				.IsEnabled(this, &SToolbagMaterialImportOptionsWindow::CanImport)
				.OnClicked(this, &SToolbagMaterialImportOptionsWindow::OnImportAll)
				]
					+ SUniformGridPanel::Slot(2, 0)
				[
				SAssignNew(ImportButton, SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ToolbagImportOptions_Import", "Import"))
				.ToolTipText(this, &SToolbagMaterialImportOptionsWindow::GetImportTooltip)
				.IsEnabled(this, &SToolbagMaterialImportOptionsWindow::CanImport)
				.OnClicked(this, &SToolbagMaterialImportOptionsWindow::OnImport)
				]
					+ SUniformGridPanel::Slot(3, 0)
				[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ToolbagImportOptions_Cancel", "Cancel"))
				.ToolTipText(LOCTEXT("ToolbagMaterialImportOptionsWindow_Cancel_ToolTip", "Cancels importing this Toolbag Material file"))
				.OnClicked(this, &SToolbagMaterialImportOptionsWindow::OnCancel)
			]
		]
	];

	//FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	//FDetailsViewArgs DetailsViewArgs;
	//DetailsViewArgs.bAllowSearch = false;
	//DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	//TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	//
	//InspectorBox->SetContent(DetailsView->AsShared());
	//DetailsView->SetObject(ImportUI);
}


void SToolbagMaterialImportOptionsWindow::OnMetalnessConversionChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo  )
{
	FString NewValue = *NewSelection.Get();

	if (NewValue == METALNESS_INSULATORS_NAME)
	{
		ImportUI->MetalnessConversionOption = INSULATOR;
	}
	else if(NewValue == METALNESS_METALS_NAME)
	{
		ImportUI->MetalnessConversionOption = METAL;
	}
	else if(NewValue == METALNESS_GENERATE_NAME)
	{
		ImportUI->MetalnessConversionOption = CONVERT;
	}
	else if(NewValue == METALNESS_ASK_NAME)
	{
		ImportUI->MetalnessConversionOption = ASK;
	}
	MetalnessComboBoxLabel->SetText(NewValue);
}

bool SToolbagMaterialImportOptionsWindow::CanImport()  const
{
	return !(ImportUI->bSuppressWarnings && ImportUI->MetalnessConversionOption == ASK);
}

FText SToolbagMaterialImportOptionsWindow::GetImportTooltip () const
{
	if(ImportUI->bSuppressWarnings && ImportUI->MetalnessConversionOption == ASK)
	{
		return LOCTEXT("ToolbagImportOptions_Import_MetalnessPopupError_Tooltip", "Can't surpress all warnings, when metalness import settings are set to prompt per material");
	}
	else
	{
		return LOCTEXT("ToolbagImportOptions_Import_ToolTip", "Import this file");
	}
}

FText SToolbagMaterialImportOptionsWindow::GetImportAllTooltip () const
{
	if(ImportUI->bSuppressWarnings && ImportUI->MetalnessConversionOption == ASK)
	{
		return LOCTEXT("ToolbagImportOptions_Import_MetalnessPopupError_Tooltip", "Can't surpress all warnings, when metalness import settings are set to prompt per material");
	}
	else
	{
		return LOCTEXT("ToolbagImportOptions_ImportAll_ToolTip", "Import all files with these same settings");
	}
}
#undef LOCTEXT_NAMESPACE