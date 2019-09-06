// Copyright 2017 Tefel. All Rights Reserved.
#pragma once

#include "Slates/SAssetAssistantEditor.h"
#include "Toolkits/BaseToolkit.h"

class FAssetAssistantEdModeToolkit : public FModeToolkit
{
public:
	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FAssetAssistantEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return AssetAssistantEditorWidget; }

	TSharedRef<class FAssetAssistantEdModeToolkit> getShared();

protected:
	void OnChangeMode(EToolMode InMode);
	bool IsModeEnabled(EToolMode InMode) const;
	bool IsModeActive(EToolMode InMode) const;

private:
	TSharedPtr<SAssetAssistantEditor> AssetAssistantEditorWidget;
};
