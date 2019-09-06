// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "AssetAssistantEditorUISettingCustomization.h"

class FAssetAssistantEditorUISettingCustomization_Find : public FAssetAssistantEditorUISettingCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FAssetAssistantEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	TSharedRef<SWidget>	GetListWidget();

	// Find
	FReply OnFindButtonClicked();
	FReply OnContentButtonClicked();
	FReply OnCollapseDirectoriesClicked();
	FReply OnExpandDirectoriesClicked();
};


