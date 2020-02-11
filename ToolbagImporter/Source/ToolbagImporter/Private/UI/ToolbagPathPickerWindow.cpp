/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#include "ToolbagPathPickerWindow.h"
#include "ToolbagImporterPrivatePCH.h"
//#include <IMainFrameModule.h>
#include <Editor/ContentBrowser/Public/ContentBrowserModule.h>
#include <Editor/ContentBrowser/Public/IContentBrowserSingleton.h>

#define LOCTEXT_NAMESPACE "ToolbagImport"
void SToolbagPathPickerWindow::Construct(const FArguments& InArgs)
{
	WidgetWindow = InArgs._WidgetWindow;
	Path = InArgs._Path;


	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	FPathPickerConfig PathPickerConfig;
	PathPickerConfig.DefaultPath = Path;
	PathPickerConfig.OnPathSelected = FOnPathSelected::CreateSP(this, &SToolbagPathPickerWindow::OnPathChange);
	PathPickerConfig.bAddDefaultPath = true;
	this->ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1)
		.Padding(3)
		[
			ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig)
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
				.Text(LOCTEXT("ToolbagImportOptions_Ok", "Ok"))
				.OnClicked(this, &SToolbagPathPickerWindow::OnOk)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ToolbagImportOptions_Cancel", "Cancel"))
				.OnClicked(this, &SToolbagPathPickerWindow::OnCancel)
			]
		]
	];
}

FString SToolbagPathPickerWindow::ShowPathPicker(FString OriginalPath)
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

	TSharedPtr<SToolbagPathPickerWindow> PathPickerWindow;
	Window->SetContent
	(
		SAssignNew(PathPickerWindow, SToolbagPathPickerWindow)
		.WidgetWindow(Window)
		.Path(OriginalPath)
	);

	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);
	return PathPickerWindow->bCanceled ? OriginalPath : PathPickerWindow->Path;
}

#undef LOCTEXT_NAMESPACE