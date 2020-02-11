/*
Marmoset Toolbag Scene Importer

Copyright (C) 2016, Marmoset LLC
*/
#pragma once
#include "ToolbagImporterPrivatePCH.h"

#include "SlateBasics.h"
#include "ToolbagImportUI.h"

class SToolbagMetalnessConversionOptionsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SToolbagMetalnessConversionOptionsWindow )
		: _WidgetWindow()
		, _MaterialName()
		, _bCanceled(true)
	{}

	SLATE_ARGUMENT( TSharedPtr<SWindow>, WidgetWindow )
		SLATE_ARGUMENT( FString, MaterialName )
		SLATE_ARGUMENT( bool, bCanceled )
		SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	static ToolbagMetalnessConversion ShowMetalnessOptionWindow( FString MaterialName  );

	FReply CloseWindow()
	{
		if ( WidgetWindow.IsValid() )
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	FReply OnInsulator()
	{
		Result = INSULATOR;
		return CloseWindow();
	}

	FReply OnMetal()
	{
		Result = METAL;
		return CloseWindow();
	}

	FReply OnConvert()
	{
		Result = CONVERT;
		return CloseWindow();
	}
private:

	TWeakPtr< SWindow > WidgetWindow;
	bool bCanceled;
	FString MaterialName;
	ToolbagMetalnessConversion Result;
};
