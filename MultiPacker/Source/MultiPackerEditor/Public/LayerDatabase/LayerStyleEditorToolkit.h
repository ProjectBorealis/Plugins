/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "EditorUndoClient.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"
#include <IDetailsView.h>
#include <Misc/NotifyHook.h>

class FSpawnTabArgs;
class IToolkitHost;
class SDockTab;
class SLayerStyleEditor;
class UMultiPackerLayerDatabase;
//class UMaterialInstanceDynamic;

/**
* Implements an Editor toolkit for textures.
*/
class FLayerStyleEditorToolkit : public FAssetEditorToolkit, public FNotifyHook, public FEditorUndoClient, public FGCObject
{
public:
	FLayerStyleEditorToolkit();
	/**
	* Creates and initializes a new instance.
	*
	* @param InStyle The style set to use.
	*/
	/*FLayerStyleEditorToolkit(const TSharedRef<ISlateStyle>& InStyle);*/

	/** Virtual destructor. */
	virtual ~FLayerStyleEditorToolkit();


	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged) override;

public:

	/**
	* Initializes the editor tool kit.
	*
	* @param InStyleAsset The UMultiPackerLayerDatabaseasset to edit.
	* @param InMode The mode to create the toolkit in.
	* @param InToolkitHost The toolkit host.
	*/
	void Initialize(UMultiPackerLayerDatabase* InStyleAsset, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost);


	void CreateInternalWidgets();
public:

	//~ FAssetEditorToolkit interface

	virtual FString GetDocumentationLink() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

public:

	//~ IToolkit interface

	virtual FText GetBaseToolkitName() const override;
	virtual FName GetToolkitFName() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;

public:

	//~ FGCObject interface

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:

	//~ FEditorUndoClient interface

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:

	/** Callback for spawning the Properties tab. */
/*	TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier);*/

	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	TSharedRef<SDockTab> SpawnTab_Material(const FSpawnTabArgs& Args);
private:

	/** The text asset being edited. */
	UMultiPackerLayerDatabase* StyleAsset;

	//UMaterial* Material;
	//UMaterialInstanceDynamic* Material;
	/** Pointer to the style set to use for toolkits. */
	//TSharedRef<ISlateStyle> Style;

	TSharedPtr<class IDetailsView> PropertyWidget;

	/** Preview viewport widget used for UI materials */
	TSharedPtr<class SLayerStyleEditor> PreviewUIViewport;

	//TSharedPtr<class SMaterialEditor3DPreviewViewport> PreviewViewport;

	static const FName AppId;
	static const FName PreviewTabId;
	static const FName DetailsTabId;
	static const FName TextTabId;
};
