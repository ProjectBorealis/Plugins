// Copyright 2017 Tefel. All Rights Reserved.
#pragma once

class FAssetAssistantEditorUIView : public TSharedFromThis<FAssetAssistantEditorUIView>
{

public:
	class FAssetAssistantEdMode* EditMode;
	class TSharedPtr<SWidget> ViewWidget;
	class TSharedPtr<SAssistantList> ResultListWidget;


	FReply OnEditedAssets();
	FReply OnContentAssets();
	FReply OnPasteAssets(bool bAppend);
	FReply OnSetImportAssets(bool bShouldSave);

	FString GetTArrayToCopy();
	FString GetTArrayToCopy(int32 &Count);
	FString GetTArrayToCopy(int32 &Count, TArray<TSharedPtr<FAssetFile>> &AssetItems);
	FReply OnCopyTArray();
	FReply OnCopyList();

	// Result bottom buttons
	FReply OnClearAllRows();
	FReply OnRemoveSelectedRows();
	FReply OnRemoveUnselectedRows();
	FReply OnPasteNewRows();
	FReply OnSaveList();

	bool UpdateResultFromList(FString ListText, bool bAppend, bool bShouldSave);

	FAssetAssistantEditorUIView(FAssetAssistantEdMode* InEditMode);
	TSharedRef<SWidget> GetWidget();
	FText GetRowsNumberLabel() const;

};