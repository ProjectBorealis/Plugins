// Copyright 2017 Tefel. All Rights Reserved.
#include "Slates/SAssistantList.h"
#include "Slates/SAssistantCombo.h"

#define LOCTEXT_NAMESPACE "SAssistantList"

void SAssistantList::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox)
		[
			//The actual list view creation
			SAssignNew(ListViewWidget, SListView<TSharedPtr<FAssetFile>>)
			.ItemHeight(24)
			.ListItemsSource(&AssetItems) //The Items array is the source of this listview
			.OnGenerateRow(this, &SAssistantList::OnGenerateRowForList)
			
			.HeaderRow
			(
				SNew(SHeaderRow)

				+ SHeaderRow::Column("ID")
				.DefaultLabel(LOCTEXT("ResultListHeader_ID", ""))
				.FixedWidth(40.0f)

				+ SHeaderRow::Column("Name")
				.DefaultLabel(LOCTEXT("ResultListHeader_Name", "Name"))
				.FillWidth(0.6f)

				+ SHeaderRow::Column("Type")
				.DefaultLabel(LOCTEXT("ResultListHeader_Type", "Type"))
				.FillWidth(0.4f)

				+ SHeaderRow::Column("Options")
				.DefaultLabel(LOCTEXT("ResultListHeader_Options", "Options"))
				.FixedWidth(70.0f)
			)
		]
	];
}


FReply SAssistantList::Initialize(TArray<FAssetFile> AssetFiles)
{
	HighestItemID = -1;
	AssetItems.Empty();
	for (auto AssetFile : AssetFiles)
	{
		HighestItemID++;
		FAssetFile* aFile = new FAssetFile(AssetFile);
		aFile->ItemID = HighestItemID;
		AssetItems.Add(MakeShareable(aFile));
	}
	ListViewWidget->RequestListRefresh();
	return FReply::Handled();
}

FReply SAssistantList::UpdateIDs()
{
	for (int32 Index = 0; Index < AssetItems.Num(); Index++)
	{
		AssetItems[Index]->ItemID = Index;
	}
	return FReply::Handled();
}

TSharedRef<ITableRow> SAssistantList::OnGenerateRowForList(TSharedPtr<FAssetFile> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SAssistantListRow, OwnerTable, Item)
		.AssistantList(SharedThis(this));
}

FReply SAssistantListRow::OnAddButtonPressed()
{
	int32 Index;
	FAssetFile* ItemNew = new FAssetFile();
	if (AssistantList->AssetItems.Find(Item, Index))
	{
		AssistantList->AssetItems.Insert(MakeShareable(ItemNew), Index);
	}

	AssistantList->ListViewWidget->RequestListRefresh();
	return FReply::Handled();
}

FReply SAssistantListRow::OnBrowseButtonPressed()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData XAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*Item->AssetPath));
	TArray<FAssetData> AssetDataList;
	AssetDataList.Add(XAssetData);
	GEditor->SyncBrowserToObjects(AssetDataList);

	return FReply::Handled();
}

FReply SAssistantListRow::OnDeleteButtonPressed()
{
	AssistantList->AssetItems.Remove(Item);
	AssistantList->ListViewWidget->RequestListRefresh();

	FAssetAssistantEdMode* EditMode = (FAssetAssistantEdMode*)GLevelEditorModeTools().GetActiveMode(FAssetAssistantEdMode::EM_AssetAssistantEdModeId);
	EditMode->SaveResultList();
	return FReply::Handled();
}

void SAssistantListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FAssetFile> InItem)
{
	AssistantList = InArgs._AssistantList;
	Item = InItem;
	SMultiColumnTableRow< TSharedPtr<FAssetFile> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
}

TSharedRef<SWidget> SAssistantListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "ID")
	{
		return SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item.Get()->ItemID))
			];
	}
	else if (ColumnName == "Name")
	{
		return SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ToolTipText(FText::FromString(*Item.Get()->AssetPath))
				.Text(FText::FromString(*Item.Get()->AssetName))
			];
	}
	else if (ColumnName == "Type")
	{
		return SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*Item.Get()->AssetClass))
				.ColorAndOpacity(FLinearColor(0.824f, 0.706f, 0.549f))
			];
	}
	else if (ColumnName == "Options")
	{
		return SNew(SHorizontalBox)
			// TODO instead of adding new item, check if asset exist
			//+ SHorizontalBox::Slot()
			//	.Padding(1, 0)
			//	.AutoWidth()
			//	.VAlign(VAlign_Center)
			//	[

			//		SNew(SItemButton)
			//		.Text(LOCTEXT("NewBlueprintButtonLabel", "New Blueprint"))
			//		.ToolTipText(LOCTEXT("Add_Button_tooltip", "Add item before this element"))
			//		.Image(FEditorStyle::GetBrush("PropertyWindow.Button_AddToArray"))
			//		.OnClicked(this, &SAssistantListRow::OnAddButtonPressed)
			//	]

			+SHorizontalBox::Slot()
			.Padding(1, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SItemButton)
				.ToolTipText(LOCTEXT("FindRowContentBrowser", "Find this element in a Content Browser"))
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
				.OnClicked(this, &SAssistantListRow::OnBrowseButtonPressed)
			]

		+ SHorizontalBox::Slot()
			.Padding(1, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SItemButton)
				.ToolTipText(LOCTEXT("RemoveRow", "Remove row"))
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Delete"))
				.OnClicked(this, &SAssistantListRow::OnDeleteButtonPressed)
			];
	}
	return SNullWidget::NullWidget;
}

void SAssistantCombo::Construct(const FArguments& InArgs)
{
	// from directories
	InArgs._ComboDirectories.Names;
	if (InArgs._ComboDirectories.Names.Num() > 0)
	{
		for (auto Name : InArgs._ComboDirectories.Names)
		{
			Options.Add(MakeShareable(new FString(Name)));
		}
		CurrentItem = Options[0];
	}

	// from names
	if (InArgs._OptionNames.Num() > 0)
	{
		for (auto Name : InArgs._OptionNames)
		{
			Options.Add(MakeShareable(new FString(Name)));
		}
		CurrentItem = Options[0];
	}

	ChildSlot
	[
		SNew(SComboBox<FComboItemType>)
		.OptionsSource(&Options)
		.OnSelectionChanged(this, &SAssistantCombo::OnSelectionChanged)
		.OnGenerateWidget(this, &SAssistantCombo::MakeWidgetForOption)
		.InitiallySelectedItem(CurrentItem)
		[
			SNew(STextBlock)
			.Text(this, &SAssistantCombo::GetCurrentItemLabel)
		]
	];
}

#undef LOCTEXT_NAMESPACE