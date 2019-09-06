// Copyright 2017 Tefel. All Rights Reserved.

#include "AssetAssistantEditorUIView.h"
#include "AssetAssistantEditorUISetting.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "AssetAssistant.UIView"

FAssetAssistantEditorUIView::FAssetAssistantEditorUIView(FAssetAssistantEdMode* InEditMode)
{
	EditMode = InEditMode;

	SAssignNew(ViewWidget, SVerticalBox)
	+ SVerticalBox::Slot()
	.Padding(2, 2)
	.AutoHeight()
	[
		SNew( SHorizontalBox )

		// Button Get Content Browser files
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "ContentAssetsTooltip", "Get selected Content Browser assets" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnContentAssets)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(FEditorFontGlyphs::Folder_Open)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "SCSEditor.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text( LOCTEXT( "Cont", "Cont" ) )
				]
			]
		]

		// Button Edited
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "GetEditedAssetsTooltip", "Get all assets being currently modified (opened)" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnEditedAssets)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(FEditorFontGlyphs::Credit_Card)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "SCSEditor.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text( LOCTEXT( "Edited", "Edited" ) )
				]
			]
		]

		// Button Paste from Cllipboard
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "PasteAssetsTooltip", "Paste asset rows from clipboard (TArray or List)" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnPasteAssets, false)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(FEditorFontGlyphs::Cloud_Download)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "SCSEditor.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text( LOCTEXT( "Paste", "Paste" ) )
				]
			]
		]

		// BUTTON 3 RIGHT
		+ SHorizontalBox::Slot()
		.Padding(2)
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "CopyTarrayTooltip", "Copy all rows as TArray - ready to paste into the TArray var" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnCopyTArray)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(FEditorFontGlyphs::Clone)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "SCSEditor.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text( LOCTEXT( "TArr", "TArr" ) )
				]
			]
		]

		// BUTTON 3 RIGHT
		+ SHorizontalBox::Slot()
		.Padding(2)
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "CopyListTooltip", "Copy rows as list - every row is a new line" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnCopyList)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(FEditorFontGlyphs::Clipboard)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "SCSEditor.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text( LOCTEXT( "List", "List" ) )
				]
			]
		]
	]
	
	// Main result list
	+ SVerticalBox::Slot()
	.Padding(2, 0)
	.FillHeight(1)
	[
		SAssignNew(ResultListWidget, SAssistantList)
	]

	// Separator
	+ SVerticalBox::Slot()
	.Padding(2, 0)
	.AutoHeight()
	[
		SNew(SSeparator)
	]

	// Bottom Menu
	+ SVerticalBox::Slot()
	.Padding(2, 0)
	.AutoHeight()
	[
		SNew( SHorizontalBox )

		// Button (only image)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
			.ToolTipText( LOCTEXT( "ClearAllRowsTooltip", "Clear all rows" ) )
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnClearAllRows)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.14"))
				.Text(FEditorFontGlyphs::Trash_O)
			]
		]

		// Button (only image)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
			.ToolTipText( LOCTEXT( "RemoveSelectedRowsTooltip", "Remove all selected rows" ) )
			.ContentPadding(FMargin(4, 5, 3, 0))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnRemoveSelectedRows)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text(FEditorFontGlyphs::Minus_Square_O)
			]
		]

		// Button (only image)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
			.ToolTipText( LOCTEXT( "RemoveUnselectedRowsTooltip", "Remove all unselected rows" ) )
			.ContentPadding(FMargin(4, 5, 3, 0))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnRemoveUnselectedRows)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text(FEditorFontGlyphs::Share_Square_O)
			]
		]

		// Button (only image)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
			.ToolTipText( LOCTEXT( "PasteNewRowsTooltip", "Paste new rows from clipboard at the end" ) )
			.ContentPadding(FMargin(4, 5, 3, 0))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnPasteNewRows)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text(FEditorFontGlyphs::Plus_Square_O)
			]
		]

		// Separator
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(3, 0)
		[
			SNew(SSeparator)
			.Orientation(Orient_Vertical)
		]

		// Button Get Content Browser files
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(6)
		[
			SNew(STextBlock)
			.Text_Raw(this, &FAssetAssistantEditorUIView::GetRowsNumberLabel)
		]

		// Button Get Content Browser files
		+ SHorizontalBox::Slot()
		.Padding(2)
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		[
			SNew( SButton )
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ToolTipText( LOCTEXT( "SaveListTooltip", "Save list as set in Macro category with current date" ) )
			.ContentPadding(FMargin(7, 3, 7, 2))
			.OnClicked_Raw(this, &FAssetAssistantEditorUIView::OnSaveList)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()	// button icon
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(FEditorFontGlyphs::Hdd_O)
				]

				+ SHorizontalBox::Slot()	// button text
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew( STextBlock )
					.TextStyle( FEditorStyle::Get(), "SCSEditor.TopBar.Font" )
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text( LOCTEXT( "SaveList", "Save list" ) )
				]
			]
		]
	];
}

TSharedRef<SWidget> FAssetAssistantEditorUIView::GetWidget()
{
	return ViewWidget.ToSharedRef();
}

FText FAssetAssistantEditorUIView::GetRowsNumberLabel() const
{
	int32 TotalRows = 0, SelectedRows = 0;
	if (ResultListWidget.Get() != nullptr)
	{
		TotalRows = ResultListWidget->AssetItems.Num();
		SelectedRows = ResultListWidget->ListViewWidget->GetSelectedItems().Num();
	}
	return FText::Format(LOCTEXT("RowsNumberLabel", "{0} rows ({1} selected)"), TotalRows, SelectedRows);
}

FReply FAssetAssistantEditorUIView::OnEditedAssets()
{
	TArray<UObject*> OpenedAssets = FAssetEditorManager::Get().GetAllEditedAssets();
	TArray<FAssetFile> AssetFiles;
	for (auto OpenedAsset : OpenedAssets)
	{
		FAssetFile AssetFile = FAssetFile();
		AssetFile.AssetName = OpenedAsset->GetName();
		AssetFile.AssetClass = OpenedAsset->GetClass()->GetName();
		AssetFile.AssetPath = OpenedAsset->GetPathName();
		if (!AssetFile.AssetPath.Contains("/Engine/"))
		{
			AssetFiles.Add(AssetFile);
		}
	}
	ResultListWidget->Initialize(AssetFiles);
	EditMode->SaveResultList();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnContentAssets()
{
	EditMode->AssetFind->OnContentButtonClicked();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnPasteAssets(bool bAppend)
{
	FString TextToPaste;
	int32 CountBefore = ResultListWidget->AssetItems.Num();
	FPlatformApplicationMisc::ClipboardPaste(TextToPaste);
	bool IsTArray = UpdateResultFromList(TextToPaste, bAppend, true);

	FText PasteType = IsTArray ? FText::FromString("TArray") : FText::FromString("List");
	FText Message = bAppend
					? FText::Format(LOCTEXT("AppendAssetsMessage", "Appended {0} assets from {1}"), ResultListWidget->AssetItems.Num() - CountBefore, PasteType)
					: FText::Format(LOCTEXT("PasteAssetsMessage", "Pasted {0} assets from {1}"), ResultListWidget->AssetItems.Num(), PasteType);
	
	EditMode->NotifyMessage(Message);
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnSetImportAssets(bool bShouldSave)
{
	UpdateResultFromList(EditMode->UISetting->CurrentSetTArrayText, false, bShouldSave);
	FText Message = FText::Format(LOCTEXT("SetImportAssetsMessage", "Imported from Set {0} assets"), ResultListWidget->AssetItems.Num());

	EditMode->NotifyMessage(Message);
	return FReply::Handled();
}

bool FAssetAssistantEditorUIView::UpdateResultFromList(FString ListText, bool bAppend, bool bShouldSave)
{
	int32 IndexTemp;
	bool IsTArray = (ListText.FindChar(TEXT('('), IndexTemp) && (ListText.FindChar(TEXT(')'), IndexTemp)));
	FString Splitter = IsTArray ? "," : "\r\n";
	if (IsTArray)
	{
		ListText =  ListText.Replace(TEXT("("), TEXT(""))
							.Replace(TEXT(")"), TEXT(""))
							.Replace(TEXT("\r\n"), TEXT(""))
							.Replace(TEXT("\""), TEXT(""));
		ListText = ListText + TEXT(",");
	}
	else
	{
		ListText = ListText + TEXT("\r\n");
	}

	FString Left, Right;
	TArray<FAssetFile> AssetFiles;
	while (ListText.Split(Splitter, &Left, &Right))
	{
		ListText = Right;
		FAssetFile AssetFile = FAssetFile();
		int32 QuotePos;
		if (Left.FindChar(TEXT('\''), QuotePos))
		{
			AssetFile.AssetClass = Left.LeftChop(Left.Len() - QuotePos);
			AssetFile.AssetPath = Left.Mid(QuotePos + 1, Left.Len() - QuotePos - 2);
			AssetFile.AssetPath.Split(".", NULL, &AssetFile.AssetName);
			AssetFiles.Add(AssetFile);
		}
	};

	// append
	if (bAppend)
	{
		TArray<FAssetFile> AssetExisting;
		for (auto AssetItem : ResultListWidget->AssetItems)
		{
			AssetExisting.Add(*AssetItem);
			AssetFiles.Remove(*AssetItem);
		}
		AssetExisting.Append(AssetFiles);
		AssetFiles = AssetExisting;
	}

	ResultListWidget->Initialize(AssetFiles);
	if (bShouldSave) EditMode->SaveResultList();

	return IsTArray;
}

FString FAssetAssistantEditorUIView::GetTArrayToCopy(int32 &Count)
{
	return GetTArrayToCopy(Count, ResultListWidget->AssetItems);
}

FString FAssetAssistantEditorUIView::GetTArrayToCopy()
{
	int32 Count;
	return GetTArrayToCopy(Count);
}

FString FAssetAssistantEditorUIView::GetTArrayToCopy(int32 &Count, TArray<TSharedPtr<FAssetFile>> &AssetItems)
{
	FString TArrayText = "(";
	int32 ItemCount = 0;
	for (int32 Index = 0; Index < AssetItems.Num(); ++Index)
	{
		FString listSeparator = (Index == AssetItems.Num() - 1) ? ")" : ",";
		TArrayText.Append(AssetItems[Index]->AssetClass + "'" + AssetItems[Index]->AssetPath + "'" + listSeparator + "\r\n");
		ItemCount++;
	}
	Count = ItemCount;
	return TArrayText;
}

FReply FAssetAssistantEditorUIView::OnCopyTArray()
{
	int32 tmp = 0;
	FString TextToCopy = GetTArrayToCopy();
	FPlatformApplicationMisc::ClipboardCopy(*TextToCopy);

	FText Message = FText::Format(LOCTEXT("CopyTArrayMessage", "Copy {0} assets as TArray"), ResultListWidget->AssetItems.Num());
	EditMode->NotifyMessage(Message);

	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnCopyList()
{
	FString TextToCopy;
	for (auto AssetItem : ResultListWidget->AssetItems)
	{
		TextToCopy.Append(AssetItem->AssetClass + "'" + AssetItem->AssetPath + "'\r\n");
	}
	FPlatformApplicationMisc::ClipboardCopy(*TextToCopy);

	FText Message = FText::Format(LOCTEXT("CopyListMessage", "Copy {0} assets as List"), ResultListWidget->AssetItems.Num());
	EditMode->NotifyMessage(Message);

	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnClearAllRows()
{
	ResultListWidget->AssetItems.Empty();
	ResultListWidget->ListViewWidget->RequestListRefresh();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnRemoveSelectedRows()
{
	auto SelectedItems = ResultListWidget->ListViewWidget->GetSelectedItems();
	for (auto SelectedItem : SelectedItems)
	{
		ResultListWidget->AssetItems.Remove(SelectedItem);
	}
	ResultListWidget->UpdateIDs();
	ResultListWidget->ListViewWidget->RebuildList();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnRemoveUnselectedRows()
{
	auto SelectedItems = ResultListWidget->ListViewWidget->GetSelectedItems();
	ResultListWidget->AssetItems = SelectedItems;
	ResultListWidget->UpdateIDs();
	ResultListWidget->ListViewWidget->RebuildList();
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnPasteNewRows()
{
	OnPasteAssets(true);
	return FReply::Handled();
}

FReply FAssetAssistantEditorUIView::OnSaveList()
{
	FString textToUse = FDateTime().Now().ToString(TEXT("[%d-%m-%Y %H:%M:%S]"));
	EditMode->AssetMacro->AddNewSet(textToUse);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE