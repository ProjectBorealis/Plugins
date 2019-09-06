// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "AssetAssistantEditorCommands.h"
#include "EdMode.h"
#include "Framework/Commands/UICommandList.h"

DECLARE_DELEGATE(FOnSelectionChanged);

class FAssetAssistantEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_AssetAssistantEdModeId;
public:
	FAssetAssistantEdMode();
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual void Enter() override;
	virtual void Exit() override;
	virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent) override;
	bool UsesToolkits() const override;

	void NotifyMessage(const FText& Message);
	void SaveResultList();
	void LoadResultList();

	// Tool Modes
	void SetCurrentToolMode(EToolMode InMode);
	EToolMode GetCurrentToolMode() const;

	class UAssetAssistantEditorUISetting* UISetting;
	class FAssetAssistantEditorUISettingCustomization_Find* AssetFind;
	class FAssetAssistantEditorUISettingCustomization_Modify* AssetModify;
	class FAssetAssistantEditorUISettingCustomization_Macro* AssetMacro;
	class FAssetAssistantEditorUISettingCustomization_Extra* AssetExtra;
	class FAssetAssistantEditorUIView* ViewResult;

protected:
	TSharedPtr<FUICommandList> UICommandList;

private:
	FOnSelectionChanged OnSelectionChangedDelegate;
	EToolMode CurrentToolMode;
};