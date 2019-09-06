// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "AssetAssistantEditorUISettingCustomization.h"

class FAssetAssistantEditorUISettingCustomization_Macro : public FAssetAssistantEditorUISettingCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FAssetAssistantEdMode* InEditMode);

	void AddNewSet(FString textToUse);

	FReply OnClearSetsClicked();
	FReply OnClearAutosavesClicked();
	FReply OnCreateAutosaveClicked();
	FReply OnSetAddClicked();
	FReply OnSetRemoveClicked();
	FReply OnSetLoadClicked();
	void OnLoad(int32 loadType, bool bShouldSave);
	void OnRecover();
	FReply OnSetLoadFindClicked();
	FReply OnSetLoadOpenClicked();

	FReply OnAutosaveLoadClicked();
	FReply OnAutosaveLoadFindClicked();
	FReply OnAutosaveLoadOpenClicked();

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	TSharedPtr<class SAssistantCombo> AssistantComboAutosaveSets;
};

