// Copyright 2017 Tefel. All Rights Reserved.
#pragma once
#include "AssetAssistantEditorUISettingCustomization.h"

class FAssetAssistantEditorUISettingCustomization_Modify : public FAssetAssistantEditorUISettingCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FAssetAssistantEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply OnPatternAddClicked();
	FReply OnPatternRemoveClicked();

	FReply OnModifyFindClicked();
	FReply OnModifyRenameClicked();
	FReply OnModifyOpenClicked();
	FReply OnModifySaveClicked();
	FReply OnModifyRemoveClicked();
	FReply OnModifyMarkDirtyClicked();
	FReply OnModifyAutoConsolidateClicked();
	FReply OnModifyCompileClicked();
	FReply OnModifyCreateChildClicked();
	FReply OnModifyDuplicateClicked();

	TArray<FString> GeneratePatternList(const FString PatternIn, int32 ItemsCount, bool isDuplicate);

	FText GetSingleActionText() const;
	FText GetMultipleActionText() const;

	static void Recover();

};

