// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "IDetailCustomization.h"

class FAssetAssistantEditorUISettingCustomization : public  IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override = 0;
	static struct FComboDirectories GetAllDirs();
	static TSharedRef<class FAssistantTreeItem> GetDirectoryTree();
	static void GetAvaiableClasses(TArray<FString> &AvaiableClassesNames, TArray<int32> &AvaiableClassesCounts);

	class TSharedPtr<SWidget> ViewWidget;

	TSharedPtr<class SAssistantList> AssistantList;
	TSharedPtr<class SAssistantCombo> AssistantComboPatterns;
	TSharedPtr<class SAssistantCombo> AssistantComboSets;
	TSharedPtr<class SAssistantRichCombo> AssistantComboClasses;
	TSharedPtr<class SAssistantTree> AssistantTree;

protected:
	void HideCategoryByToolMode(IDetailLayoutBuilder& DetailBuilder, EToolMode ToolMode);
	static class FAssetAssistantEdMode* EditMode;

};