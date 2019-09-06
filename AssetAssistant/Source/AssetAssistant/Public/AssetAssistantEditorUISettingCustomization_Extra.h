// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "AssetAssistantEditorUISettingCustomization.h"

class FAssetAssistantEditorUISettingCustomization_Extra : public FAssetAssistantEditorUISettingCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FAssetAssistantEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};