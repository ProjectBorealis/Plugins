// Copyright 2017 Tefel. All Rights Reserved.

#pragma once
#include "Widgets/SCompoundWidget.h"
#include "PropertyEditorDelegates.h"

class SErrorText;
class IDetailsView;

class FAssetAssistantEdModeToolkit;

class SAssetAssistantEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssetAssistantEditor) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TSharedRef<FAssetAssistantEdModeToolkit> InParentToolkit);
	void Construct(const FArguments& InArgs);

	static class FAssetAssistantEdMode* GetEditorMode();
	FReply CreditsButtonClicked();


protected:
	bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;

protected:
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<IDetailsView> SelectionDetailView;
		
private:
	static EVisibility GetVisibility(EToolMode InMode);
	static EVisibility GetVisibility(int32 InModes);
};
