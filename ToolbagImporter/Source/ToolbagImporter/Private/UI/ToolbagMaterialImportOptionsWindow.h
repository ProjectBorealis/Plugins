/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"

#include "SlateBasics.h"
#include "ToolbagPathPickerWindow.h"

class UToolbagImportUI;

class SToolbagMaterialImportOptionsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SToolbagMaterialImportOptionsWindow )
		: _ImportUI(NULL)
		, _WidgetWindow()
		, _SourcePath()
		, _TargetPath()
	{}

	SLATE_ARGUMENT( UToolbagImportUI*, ImportUI )
		SLATE_ARGUMENT( TSharedPtr<SWindow>, WidgetWindow )
		SLATE_ARGUMENT( FText, SourcePath )
		SLATE_ARGUMENT( FText, TargetPath )
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }

	FReply OnChangeTexturesPath()
	{
		FString Path = ImportUI->TexturesPath;
		if(Path.RemoveFromStart("."))
		{
			Path = TargetPath + Path;
		}
		Path = SToolbagPathPickerWindow::ShowPathPicker(Path);
		ImportUI->AbsoluteTexturesPath = Path;
		if(Path.RemoveFromStart(TargetPath))
		{
			Path = "." + Path;
		}
		ImportUI->TexturesPath = Path;
		TexturesPathLabel->SetText(FText::FromString( Path ));
		return FReply::Handled();
	}

	void OnMetalnessConversionChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo  );

	TSharedRef<SWidget> MakeMetalnessConversionComboWidget( TSharedPtr<FString> InItem )
	{
		return SNew(STextBlock).Text( FText::FromString(*InItem) ) .Font( FEditorStyle::GetFontStyle( TEXT("PropertyWindow.NormalFont") ));
	}

	void OnDirectImportChanged(ECheckBoxState state)
	{
		ImportUI->bDirectImport = state == ECheckBoxState::Checked;
	}

	void OnImportSuppressWarningsChanged(ECheckBoxState state)
	{
		ImportUI->bSuppressWarnings = state == ECheckBoxState::Checked;
	}

	FReply OnImport()
	{
		bShouldImport = true;
		if ( WidgetWindow.IsValid() )
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	FReply OnImportAll()
	{
		bShouldImportAll = true;
		return OnImport();
	}

	FReply OnCancel()
	{
		bShouldImport = false;
		bShouldImportAll = false;
		if ( WidgetWindow.IsValid() )
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override
	{
		if( InKeyEvent.GetKey() == EKeys::Escape )
		{
			return OnCancel();
		}

		return FReply::Unhandled();
	}

	bool ShouldImport() const
	{
		return bShouldImport;
	}

	bool ShouldImportAll() const
	{
		return bShouldImportAll;
	}

	SToolbagMaterialImportOptionsWindow() 
		: ImportUI(NULL)
		, bShouldImport(false)
		, bShouldImportAll(false)
	{}


private:

	bool CanImport() const;
	FText GetImportTooltip() const;
	FText GetImportAllTooltip() const;

private:
	UToolbagImportUI*	ImportUI;
	TWeakPtr< SWindow > WidgetWindow;
	TSharedPtr< SButton > ImportButton;
	bool			bShouldImport;
	bool			bShouldImportAll;

	FString TargetPath;

	TSharedPtr< STextBlock > MeshesPathLabel;
	TSharedPtr< STextBlock > MaterialsPathLabel;
	TSharedPtr< STextBlock > TexturesPathLabel;

	TSharedPtr< STextBlock > MetalnessComboBoxLabel;
	TArray< TSharedPtr< FString > >	MetalnessSourceComboList;
};