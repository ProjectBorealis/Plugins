/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"
#include "GraphEditor.h"

class SMultiPackerAssetEditorDrop;
class FMultiPackerRuntimeAssetEditorToolbar;
class UMultiPackerRuntimeGraph;
class UMultiPackerRuntimeOutputNode;
class SMultiPackerAssetEditorDrop;
class FAssetEditorManager;
class SDockTab;


class FMultiPackerRuntimeAssetEditor : public FAssetEditorToolkit, public FNotifyHook, public FGCObject
{
public:
	FMultiPackerRuntimeAssetEditor();
	virtual ~FMultiPackerRuntimeAssetEditor();

	void InitGenericGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UMultiPackerRuntimeGraph* Graph);

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void SaveAsset_Execute() override;
	// End of FAssetEditorToolkit

	//Toolbar
	void UpdateToolbar();
	TSharedPtr<class FMultiPackerRuntimeAssetEditorToolbar> GetToolbarBuilder() { return ToolbarBuilder; }
	void RegisterToolbarTab(const TSharedRef<class FTabManager>& TabManager);


	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

private:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_ContentBrowser(const FSpawnTabArgs& Args);
	void CreateInternalWidgets();
	TSharedRef<SGraphEditor> CreateViewportWidget();

	void BindCommands();

	void CreateEdGraph();

	void CreateCommandList();

	TSharedPtr<SGraphEditor> GetCurrGraphEditor();

	FGraphPanelSelectionSet GetSelectedNodes();

	void RebuildGenericGraph();

	// Delegates for graph editor commands
	void SelectAllNodes();
	bool CanSelectAllNodes();
	void DeleteSelectedNodes();
	bool CanDeleteNodes();
	void DeleteSelectedDuplicatableNodes();
	void CutSelectedNodes();
	bool CanCutNodes();
	void CopySelectedNodes();
	bool CanCopyNodes();
	void PasteNodes();
	void PasteNodesHere(const FVector2D& Location);
	bool CanPasteNodes();
	void DuplicateNodes();
	bool CanDuplicateNodes();

	void GraphSettings();
	bool CanGraphSettings() const;
	void ProcessGraph();
	bool CanProcessGraph();
	void OnCreateComment();
	//////////////////////////////////////////////////////////////////////////
	// graph editor event
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	void OnNodeDoubleClicked(UEdGraphNode* Node);

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

	void OnPackageSaved(const FString& PackageFileName, UObject* Outer);

	void HandleAssetDropped(UObject* AssetObject);
	bool IsAssetAcceptableForDrop(const UObject* AssetObject) const;
	FVector2D GetAssetDropGridLocation() const;

	UMultiPackerRuntimeGraph* EditingGraph;
	bool NodesChanged = true;
	bool canProcess = true;
	//Toolbar
	TSharedPtr<class FMultiPackerRuntimeAssetEditorToolbar> ToolbarBuilder;

	/** Handle to the registered OnPackageSave delegate */
	FDelegateHandle OnPackageSavedDelegateHandle;

	TSharedPtr<SGraphEditor> ViewportWidget;
	TSharedPtr<class IDetailsView> PropertyWidget;
	//TSharedRef<SDockTab> ViewportTab;

	/** The command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;

	TSharedPtr<SMultiPackerAssetEditorDrop> AssetDropTarget;

	/**
	* Called when a node's title is committed for a rename
	*
	* @param	NewText				New title text
	* @param	CommitInfo			How text was committed
	* @param	NodeBeingChanged	The node being changed
	*/
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);
};