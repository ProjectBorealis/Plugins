/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"

#include "SlateBasics.h"

class UToolbagImportUI;

class SToolbagPathPickerWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SToolbagPathPickerWindow )
		: _WidgetWindow()
		, _Path()
		, _bCanceled(true)
	{}

	SLATE_ARGUMENT( TSharedPtr<SWindow>, WidgetWindow )
	SLATE_ARGUMENT( FString, Path )
	SLATE_ARGUMENT( bool, bCanceled )
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	static FString ShowPathPicker( FString OriginalPath );

	void OnPathChange(const FString& NewPath)
	{
		Path = NewPath;
	}

	FReply OnOk()
	{
		if ( WidgetWindow.IsValid() )
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	FReply OnCancel()
	{
		bCanceled = true;
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
private:

	bool CanImport() const;

private:
	TWeakPtr< SWindow > WidgetWindow;
	bool bCanceled;
	FString Path;
};
