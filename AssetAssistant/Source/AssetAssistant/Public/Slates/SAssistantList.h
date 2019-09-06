// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "Widgets/SCompoundWidget.h"
#include "AssetAssistantEditorUISetting.h"

class SAssistantList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssistantList)
	{}
	SLATE_END_ARGS()

	typedef TSharedPtr<FString> FComboItemType;

	void Construct(const FArguments& InArgs);
	FReply Initialize(TArray<FAssetFile> AssetFiles);
	FReply UpdateIDs();

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetFile> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TArray<TSharedPtr<FString>> Items;
	TArray<TSharedPtr<FAssetFile>> AssetItems;
	
	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FAssetFile> > > ListViewWidget;
	int32 HighestItemID;
};

class SAssistantListRow : public SMultiColumnTableRow< TSharedPtr<FAssetFile> >
{
public:
	SLATE_BEGIN_ARGS(SAssistantListRow) {}
	SLATE_ARGUMENT(TSharedPtr<SAssistantList>, AssistantList)
	SLATE_END_ARGS()

	FReply OnAddButtonPressed();
	FReply OnBrowseButtonPressed();
	FReply OnDeleteButtonPressed();

	TSharedPtr<FAssetFile> Item;
	TSharedPtr<SAssistantList> AssistantList;

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FAssetFile> InItem);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
};