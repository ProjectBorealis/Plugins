/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagMetalnessConversionOptionsWindow.h"
#include "ToolbagImporterPrivatePCH.h"
//#include <IMainFrameModule.h>
#include <Editor/ContentBrowser/Public/ContentBrowserModule.h>

#define LOCTEXT_NAMESPACE "ToolbagImport"
void SToolbagMetalnessConversionOptionsWindow::Construct(const FArguments& InArgs)
{
	WidgetWindow = InArgs._WidgetWindow;
	MaterialName = InArgs._MaterialName;
	Result = INSULATOR;


	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	this->ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.FillHeight(1)
		.Padding(3)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("ToolbagImport_MetalnessSelectionPrompt", "Material \"{0}\" has no Metalness. Please select an import configuration"), FText::FromString(MaterialName)))
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
		.Text(LOCTEXT("ToolbagImportOptions_AllInsulator", "Treat as Insulator"))
		.ToolTipText(LOCTEXT("ToolbagImportOptionsWindow_AllInsulator_ToolTip", "Treats the entire Material as an insulator, setting metalness to 0"))
		.OnClicked(this, &SToolbagMetalnessConversionOptionsWindow::OnInsulator)
		]
	+ SUniformGridPanel::Slot(2, 0)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.Text(LOCTEXT("ToolbagImportOptions_AllMetal", "Treat as Metal"))
		.ToolTipText(LOCTEXT("ToolbagImportOptionsWindow_AllMetal_ToolTip", "Treats the entire Material as a Metal, setting metalness to 1"))
		.OnClicked(this, &SToolbagMetalnessConversionOptionsWindow::OnMetal)
		]
	+ SUniformGridPanel::Slot(3, 0)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.Text(LOCTEXT("ToolbagImportOptions_MetalnessConversion", "Generate Metalness"))
		.ToolTipText(LOCTEXT("ToolbagImportOptionsWindow_MetalnessConversion_ToolTip", "Automatic generation of a Metalness map based on Specular and Albedo input"))
		.OnClicked(this, &SToolbagMetalnessConversionOptionsWindow::OnConvert)
		]
		]
		];
}

ToolbagMetalnessConversion SToolbagMetalnessConversionOptionsWindow::ShowMetalnessOptionWindow(FString MaterialName)
{
	TSharedPtr<SWindow> ParentWindow;
	if( FModuleManager::Get().IsModuleLoaded( "MainFrame" ) )
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>( "MainFrame" );
		ParentWindow = MainFrame.GetParentWindow();
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(NSLOCTEXT("UnrealEd", "FBXImportOpionsTitle", "FBX Import Options"))
		.SizingRule( ESizingRule::Autosized );

	TSharedPtr<SToolbagMetalnessConversionOptionsWindow> MetalnessOptionWindow;
	Window->SetContent
		(
			SAssignNew(MetalnessOptionWindow, SToolbagMetalnessConversionOptionsWindow)
			.WidgetWindow(Window)
			.MaterialName(MaterialName)
			);

	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);
	return MetalnessOptionWindow->Result;
}

#undef LOCTEXT_NAMESPACE