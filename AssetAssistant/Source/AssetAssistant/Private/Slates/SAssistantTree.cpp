// Copyright 2017 Tefel. All Rights Reserved.
#include "Slates/SAssistantTree.h"
#include "Styling/SlateIconFinder.h"
#include "AssetAssistantEditorUISetting.h"
#include "AssetAssistantEdMode.h"
#include "AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "AssetAssistant.UISlates"

void SAssistantTree::Construct(const FArguments& Args)
{
	//Build Core Data
	RebuildFileTree();

	//Build the tree view of the above core data
	AssistantTreeView =
		SNew(SAssistantTreeView)
		// For now we only support selecting a single folder in the tree
		.SelectionMode(ESelectionMode::Single)
		.ClearSelectionOnClick(false)		// Don't allow user to select nothing.
		.TreeItemsSource(&Directories)
		.OnGenerateRow(this, &SAssistantTree::AssistantTree_OnGenerateRow)
		.OnGetChildren(this, &SAssistantTree::AssistantTree_OnGetChildren)
		;

	ChildSlot
	[
		AssistantTreeView.ToSharedRef()
	];

	// root expanded by default
	for (auto Dir : Directories)
	{
		AssistantTreeView->SetItemExpansion(Dir, true);
	}

	// select first (ROOT) directory by default
	if (AssistantTreeView->GetSelectedItems().Num() == 0 && Directories.Num()>0)
	{
		SelectDirectory(Directories[0]);
	}
}

void SAssistantTree::RebuildFileTree()
{
	Directories.Empty();
	TSharedRef<FAssistantTreeItem> DirectoryRoot = FAssetAssistantEditorUISettingCustomization::GetDirectoryTree();
	Directories.Add(DirectoryRoot);

	//Refresh
	if (AssistantTreeView.IsValid())
	{
		AssistantTreeView->RequestTreeRefresh();
	}
}

TSharedRef<ITableRow> SAssistantTree::AssistantTree_OnGenerateRow(FAssistantTreeItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Item.IsValid())
	{
		return SNew(STableRow< FAssistantTreeItemPtr >, OwnerTable)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("THIS_WAS_NULL_SOMEHOW", "THIS WAS NULL SOMEHOW"))
		];
	}

	return SNew(STableRow< FAssistantTreeItemPtr >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2,0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SAssistantTree::GetItemExpandedIcon, Item)
			]

			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(1)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*Item->GetDisplayName()))
			]
		];
}

void SAssistantTree::AssistantTree_OnGetChildren(FAssistantTreeItemPtr Item, TArray< FAssistantTreeItemPtr >& OutChildren)
{
	const auto& SubCategories = Item->GetSubDirectories();
	OutChildren.Append(SubCategories);
}

FAssistantTreeItemPtr SAssistantTree::GetSelectedDirectory() const
{
	if (AssistantTreeView.IsValid())
	{
		auto SelectedItems = AssistantTreeView->GetSelectedItems();
		if (SelectedItems.Num() > 0)
		{
			const auto& SelectedCategoryItem = SelectedItems[0];
			return SelectedCategoryItem;
		}
	}
	return NULL;
}

void SAssistantTree::SelectDirectory(const FAssistantTreeItemPtr& CategoryToSelect)
{
	if (ensure(AssistantTreeView.IsValid()))
	{
		AssistantTreeView->SetSelection(CategoryToSelect);
	}
}

bool SAssistantTree::IsItemExpanded(const FAssistantTreeItemPtr Item) const
{
	return AssistantTreeView->IsItemExpanded(Item);
}

const FSlateBrush* SAssistantTree::GetItemExpandedIcon(const FAssistantTreeItemPtr Item) const
{
	if (AssistantTreeView->IsItemExpanded(Item))
	{
		return FEditorStyle::GetBrush("SceneOutliner.FolderOpen");
	}
		return FEditorStyle::GetBrush("SceneOutliner.FolderClosed");
}

void SAssistantTree::CollapseAll()
{
	AssistantTreeView->ClearExpandedItems();
	SelectDirectory(Directories[0]);
}

void SAssistantTree::ExpandAll()
{
	ExpandItems(Directories);
}

void SAssistantTree::ExpandItems(TArray< FAssistantTreeItemPtr > Dirs)
{
	for (auto Item : Dirs)
	{
		AssistantTreeView->SetItemExpansion(Item, true);
		ExpandItems(Item->GetSubDirectories());
	}
}

#undef LOCTEXT_NAMESPACE