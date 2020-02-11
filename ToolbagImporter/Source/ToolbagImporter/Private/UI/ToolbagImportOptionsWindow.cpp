/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagImportOptionsWindow.h"
#include "ToolbagImporterPrivatePCH.h"

#include "UnrealEd.h"
#include "IDocumentation.h"
#include "ToolbagImportUI.h"
//#include "SExpandableArea.h"
#include <Slate.h>
#include <Editor/PropertyEditor/Public/PropertyEditorModule.h>

#define LOCTEXT_NAMESPACE "ToolbagImport"

#define IMPORT_MODE_SCENE_NAME TEXT("Into Scene")
#define IMPORT_MODE_BLUEPRINT_NAME TEXT("As Blueprint")
#define IMPORT_MODE_ONLY_MATERIALS TEXT("Only Materials")

#define METALNESS_INSULATORS_NAME TEXT("Treat all as Insulators")
#define METALNESS_METALS_NAME TEXT("Treat all as Metals")
#define METALNESS_GENERATE_NAME TEXT("Generate Metalness Map")
#define METALNESS_ASK_NAME TEXT("Prompt per Material")

void SToolbagImportOptionsWindow::Construct(const FArguments& InArgs)
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

	ImportModeSourceComboList.Add(MakeShareable( new FString (IMPORT_MODE_SCENE_NAME) ));
	ImportModeSourceComboList.Add(MakeShareable( new FString (IMPORT_MODE_BLUEPRINT_NAME) ));
	ImportModeSourceComboList.Add(MakeShareable( new FString (IMPORT_MODE_ONLY_MATERIALS) ));

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
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew( STextBlock )
				.Text( LOCTEXT("ToolbagImportOptions_ImportMode", "Import Mode") )
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ImportModeSourceComboList)
				.InitiallySelectedItem(ImportModeSourceComboList[static_cast<int32>(ImportUI->ImportMode)])
				.OnSelectionChanged(this, &SToolbagImportOptionsWindow::OnImportModeChanged)
				.OnGenerateWidget(this, &SToolbagImportOptionsWindow::MakeImportModeComboWidget)
				.ToolTipText(LOCTEXT("ToolbagImportOptions_ImportIntoScene_ToolTip", "Scene: Objects will be imported directly into the Scene\nBlueprint: Create a Blueprint for the entire Scene\nOnly Materials: Only import the Materials of the tbscene File"))
				.Content()
				[
					SAssignNew( ImportModeSourceComboBoxLabel, STextBlock )
					.Text( FText::FromString( *(ImportModeSourceComboList[static_cast<int32>(ImportUI->ImportMode)].Get())) )
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged( this, &SToolbagImportOptionsWindow::OnImportSuppressWarningsChanged )
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
					.OnCheckStateChanged( this, &SToolbagImportOptionsWindow::OnDirectImportChanged )
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
						.OnSelectionChanged(this, &SToolbagImportOptionsWindow::OnMetalnessConversionChanged)
						.OnGenerateWidget(this, &SToolbagImportOptionsWindow::MakeMetalnessConversionComboWidget)
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
			.AreaTitle( LOCTEXT("ToolbagImportOptions_ImportFilters", "Import Filters") )
			.InitiallyCollapsed(true)
			.BodyContent()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(SCheckBox)
					.OnCheckStateChanged( this, &SToolbagImportOptionsWindow::OnImportLightsChanged )
					.IsChecked(ImportUI->bImportLights ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					[
						SNew( STextBlock )
						.Text( LOCTEXT("ToolbagImportOptions_ImportLights", "Import Lights") )
					]
				]
		//		+SVerticalBox::Slot()
		//		.AutoHeight()
		//		.Padding(2)
		//		[
		//			SNew(SCheckBox)
		//			.OnCheckStateChanged( this, &SToolbagImportOptionsWindow::OnImportCamerasChanged )
		//			.IsChecked(ImportUI->bImportCameras ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		//			[
		//				SNew( STextBlock )
		//				.Text( LOCTEXT("ToolbagImportOptions_ImportCameras", "Import Cameras") )
		//			]
		//		]
		//		+SVerticalBox::Slot()
		//			.AutoHeight()
		//			.Padding(2)
		//			[
		//				SNew(SCheckBox)
		//				.OnCheckStateChanged( this, &SToolbagImportOptionsWindow::OnImportSkyChanged )
		//			.IsChecked(ImportUI->bImportSky ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		//			[
		//				SNew( STextBlock )
		//				.Text( LOCTEXT("ToolbagImportOptions_ImportSky", "Import Sky") )
		//			]
		//			]
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
				+SVerticalBox::Slot()
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
						.Text(LOCTEXT("ToolbagImportOptions_MeshesPath", "Meshes Path: "))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SAssignNew(MeshesPathLabel, STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
						.Text(FText::FromString(ImportUI->MeshesPath))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("ToolbagImportOptions_Change", "Change"))
						.OnClicked(this, &SToolbagImportOptionsWindow::OnChangeMeshesPath)
					]
				]
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
						.Text(LOCTEXT("ToolbagImportOptions_MaterialsPath", "Materials Path: "))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SAssignNew(MaterialsPathLabel, STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
						.Text(FText::FromString(ImportUI->MaterialsPath))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("ToolbagImportOptions_Change", "Change"))
						.OnClicked(this, &SToolbagImportOptionsWindow::OnChangeMaterialsPath)
					]
				]
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
						.OnClicked(this, &SToolbagImportOptionsWindow::OnChangeTexturesPath)
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
				.ToolTipText(this, &SToolbagImportOptionsWindow::GetImportAllTooltip)
				.IsEnabled(this, &SToolbagImportOptionsWindow::CanImport)
				.OnClicked(this, &SToolbagImportOptionsWindow::OnImportAll)
				]
					+ SUniformGridPanel::Slot(2, 0)
				[
				SAssignNew(ImportButton, SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ToolbagImportOptions_Import", "Import"))
				.ToolTipText(this, &SToolbagImportOptionsWindow::GetImportTooltip)
				.IsEnabled(this, &SToolbagImportOptionsWindow::CanImport)
				.OnClicked(this, &SToolbagImportOptionsWindow::OnImport)
				]
					+ SUniformGridPanel::Slot(3, 0)
				[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ToolbagImportOptions_Cancel", "Cancel"))
				.ToolTipText(LOCTEXT("ToolbagImportOptionsWindow_Cancel_ToolTip", "Cancels importing this Toolbag file"))
				.OnClicked(this, &SToolbagImportOptionsWindow::OnCancel)
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

void SToolbagImportOptionsWindow::OnImportModeChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo  )
{
	FString NewValue = *NewSelection.Get();

	if (NewValue == IMPORT_MODE_SCENE_NAME)
	{
		ImportUI->ImportMode = INTO_SCENE;
	}
	else if(NewValue == IMPORT_MODE_BLUEPRINT_NAME)
	{
		ImportUI->ImportMode = BLUEPRINT;
	}
	else if(NewValue == IMPORT_MODE_ONLY_MATERIALS)
	{
		ImportUI->ImportMode = ONLY_MATERIALS;
	}
	ImportModeSourceComboBoxLabel->SetText(NewValue);
}


void SToolbagImportOptionsWindow::OnMetalnessConversionChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo  )
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

bool SToolbagImportOptionsWindow::CanImport()  const
{
	return !(ImportUI->bSuppressWarnings && ImportUI->MetalnessConversionOption == ASK);
}

FText SToolbagImportOptionsWindow::GetImportTooltip () const
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

FText SToolbagImportOptionsWindow::GetImportAllTooltip () const
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