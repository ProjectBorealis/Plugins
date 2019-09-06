// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "PropertyCustomizationHelpers.h"
#include "Slates/SItemButton.h"
#include "Widgets/Views/STreeView.h"

class IPropertyHandle;
class FAssetAssistantEdMode;

#define LOCTEXT_NAMESPACE "CustomSlates"

typedef TSharedPtr< class FAssistantTreeItem > FAssistantTreeItemPtr;
/**
* The Data for a single node in the Directory Tree
*/
class FAssistantTreeItem
{
public:

	/** @return Returns the parent or NULL if this is a root */
	const FAssistantTreeItemPtr GetParentCategory() const
	{
		return ParentDir.Pin();
	}

	/** @return the path on hard disk, read-only */
	const FString& GetDirectoryPath() const
	{
		return DirectoryPath;
	}

	/** @return name to display in file tree view! read-only */
	const FString& GetDisplayName() const
	{
		return DisplayName;
	}

	/** @return Returns all subdirectories, read-only */
	const TArray< FAssistantTreeItemPtr >& GetSubDirectories() const
	{
		return SubDirectories;
	}

	/** @return Returns all subdirectories, read or write */
	TArray< FAssistantTreeItemPtr >& AccessSubDirectories()
	{
		return SubDirectories;
	}

	/** Add a subdirectory to this node in the tree! */
	void AddSubDirectory(const FAssistantTreeItemPtr NewSubDir)
	{
		SubDirectories.Add(NewSubDir);
	}

public:
	/** Constructor for FAssistantTreeItem */
	FAssistantTreeItem(const FAssistantTreeItemPtr IN_ParentDir, const FString& IN_DirectoryPath, const FString& IN_DisplayName)
		: ParentDir(IN_ParentDir)
		, DirectoryPath(IN_DirectoryPath)
		, DisplayName(IN_DisplayName)
	{
	}

private:
	TWeakPtr< FAssistantTreeItem > ParentDir;
	FString DirectoryPath;
	FString DisplayName;
	TArray< FAssistantTreeItemPtr > SubDirectories;
};

typedef STreeView< FAssistantTreeItemPtr > SAssistantTreeView;

/**
* File Tree View
*/
class SAssistantTree : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssistantTree)
	{}
	SLATE_END_ARGS()

public:
	/** Widget constructor */
	void Construct(const FArguments& Args);

	/** @return Returns the currently selected category item */
	FAssistantTreeItemPtr GetSelectedDirectory() const;

	/** Selects the specified category */
	void SelectDirectory(const FAssistantTreeItemPtr& CategoryToSelect);

	/** @return Returns true if the specified item is currently expanded in the tree */
	bool IsItemExpanded(const FAssistantTreeItemPtr Item) const;

	const FSlateBrush *GetItemExpandedIcon(const FAssistantTreeItemPtr Item) const;

	void CollapseAll();
	void ExpandAll();
	void ExpandItems(TArray< FAssistantTreeItemPtr > Dirs);

private:
	/** Called to generate a widget for the specified tree item */
	TSharedRef<ITableRow> AssistantTree_OnGenerateRow(FAssistantTreeItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Given a tree item, fills an array of child items */
	void AssistantTree_OnGetChildren(FAssistantTreeItemPtr Item, TArray< FAssistantTreeItemPtr >& OutChildren);

	/** Rebuilds the category tree from scratch */
	void RebuildFileTree();

private:
	/** The tree view widget*/
	TSharedPtr< SAssistantTreeView > AssistantTreeView;

	/** The Core Data for the Tree Viewer! */
	TArray< FAssistantTreeItemPtr > Directories;
};

#undef LOCTEXT_NAMESPACE