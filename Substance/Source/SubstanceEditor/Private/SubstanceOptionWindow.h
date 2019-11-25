// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceOptionsWindow.h

#pragma once
#include "Styling/SlateTypes.h"
#include "AssetRegistryModule.h"
#include "SubstanceImportOptionsUi.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"

namespace Substance
{
static FString InstancePath;
static FString MaterialPath;
}

class SSubstanceOptionWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSubstanceOptionWindow)
		: _ImportUI(NULL)
		, _WidgetWindow()
	{}

	SLATE_ARGUMENT(USubstanceImportOptionsUi*, ImportUI)
	SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	FReply OnImport()
	{
		bCanImport = true;

		//Make sure the names are not the same (UE4-372)
		if (InstanceNameWidget.Pin()->GetText().ToString() == MaterialNameWidget.Pin()->GetText().ToString())
		{
			if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("The Graph Instance and Material names can not be the same.")))
			{
				return FReply::Handled();
			}
		}

		ImportUI->InstanceName = InstanceNameWidget.Pin()->GetText().ToString();
		ImportUI->MaterialName = MaterialNameWidget.Pin()->GetText().ToString();

		if (WidgetWindow.IsValid())
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}

		return FReply::Handled();
	}

	FReply OnCancel()
	{
		bCanImport = false;
		if (WidgetWindow.IsValid())
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	bool ShouldImport()
	{
		return bCanImport;
	}

	SSubstanceOptionWindow()
		: ImportUI(NULL)
		, bCanImport(false)
	{}

	void InstancePathSelected(const FString& InPathName)
	{
		ImportUI->InstanceDestinationPath = InPathName;
	}

	void MaterialPathSelected(const FString& InPathName)
	{
		ImportUI->MaterialDestinationPath = InPathName;
	}

private:
	bool bIsReportingError;

	USubstanceImportOptionsUi* ImportUI;
	bool bCanImport;
	bool bReimport;

	FString ErrorMessage;
	SVerticalBox::FSlot* CustomBox;

	TWeakPtr<SWindow> WidgetWindow;
	TSharedPtr<SButton> ImportButton;

	TSharedPtr<SWidget> InstancePathPicker;
	TSharedPtr<SWidget> MaterialPathPicker;

	/** Instance Name textbox widget */
	TWeakPtr<SEditableTextBox> InstanceNameWidget;

	/** Material Name textbox widget */
	TWeakPtr<SEditableTextBox> MaterialNameWidget;

	void OnInstanceNameChanged(const FText& InNewName);

	void OnMaterialNameChanged(const FText& InNewName);

	void OnCreateInstanceChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);
		if (ImportUI->bForceCreateInstance)
		{
			return;
		}

		ImportUI->bCreateInstance = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	ECheckBoxState GetCreateInstance() const
	{
		if (ImportUI->bForceCreateInstance)
		{
			return ECheckBoxState::Checked;
		}

		return ImportUI->bCreateInstance ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState GetOverrideInstancePath() const
	{
		check(ImportUI);
		return ImportUI->bOverrideInstancePath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnOverrideInstancePathChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);
		ImportUI->bOverrideInstancePath = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	void OnInstancePathCommitted(const FText& NewValue, ETextCommit::Type CommitInfo)
	{
		check(ImportUI);
		ImportUI->InstanceDestinationPath = NewValue.ToString();
	}

	bool ShowInstancePathPicker() const
	{
		check(ImportUI);
		return ImportUI->bOverrideInstancePath;
	}

	bool CreateInstance() const
	{
		return ImportUI->bCreateInstance ? true : false;
	}

	bool CreateMaterial() const
	{
		if (ImportUI->bCreateInstance)
		{
			return ImportUI->bCreateMaterial ? true : false;
		}

		return false;
	}

	ECheckBoxState GetCreateMaterial() const
	{
		return ImportUI->bCreateMaterial ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnCreateMaterialChanged(ECheckBoxState InNewValue) const
	{
		check(ImportUI);
		ImportUI->bCreateMaterial = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	ECheckBoxState GetOverrideMaterialPath() const
	{
		check(ImportUI);
		return ImportUI->bOverrideMaterialPath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnOverrideMaterialPathChanged(ECheckBoxState InNewValue) const
	{
		check(ImportUI);
		ImportUI->bOverrideMaterialPath = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	void OnMaterialPathCommitted(const FText& NewValue, ETextCommit::Type CommitInfo) const
	{
		check(ImportUI);
		ImportUI->MaterialDestinationPath = NewValue.ToString();
	}

	bool ShowMaterialPathPicker() const
	{
		check(ImportUI);
		return ImportUI->bOverrideMaterialPath;
	}

	void RefreshImportButtonState()
	{
		//Make sure that each of the names are valid
		bool InstanceNameValid = !InstanceNameWidget.Pin()->GetText().ToString().IsEmpty();
		bool MaterialNameValid = !MaterialNameWidget.Pin()->GetText().ToString().IsEmpty();
		ImportButton->SetEnabled(MaterialNameValid && InstanceNameValid);
	}
};
