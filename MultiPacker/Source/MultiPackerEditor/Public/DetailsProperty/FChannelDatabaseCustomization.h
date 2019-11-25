/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Layout/Margin.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "IPropertyTypeCustomization.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "IPropertyUtilities.h"
#include "MultiPackerImageCore.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MultiPackerDataBase.h"

#define LOCTEXT_NAMESPACE "FChannelDatabaseCustomizationLayout"

/**
* Customizes a DataTable asset to use a dropdown
*/
class FChannelDatabaseCustomizationLayout : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
	bool ShouldFilterAsset(const class FAssetData& AssetData)
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
		bool ShouldFilterAsset(const struct FAssetData& AssetData)
#endif
	{
		if (!RowTypeFilter.IsNone())
		{
			const UMultiPackerDataBase* MultiPackerDataBase = Cast<UMultiPackerDataBase>(AssetData.GetAsset());
			if (MultiPackerDataBase->RowStruct && MultiPackerDataBase->RowStruct->GetFName() == RowTypeFilter)
			{
				return false;
			}
			return true;
		}
		return false;
	}

	/** Init the contents the combobox sources its data off */
	TSharedPtr<FString> InitWidgetContent();

	/** Returns the ListView for the ComboButton */
	TSharedRef<SWidget> GetListContent();

	/** Delegate to refresh the drop down when the datatable changes */
	void OnDataTableChanged();

	/** Return the representation of the the row names to display */
	TSharedRef<ITableRow>  HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** Display the current selection */
	FText GetNameComboBoxContentText() const;

	/** Update the root data on a change of selection */
	void OnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	

	/** Called by Slate when the filter box changes text. */
	void OnFilterTextChanged(const FText& InFilterText);

	/** The comboButton objects */
	TSharedPtr<SComboButton> RowNameComboButton;
	TSharedPtr<SListView<TSharedPtr<FString> > > RowNameComboListView;
	TSharedPtr<FString> CurrentSelectedItem;
	/** Handle to the struct properties being customized */
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> DatabasePropertyHandle;
	TSharedPtr<IPropertyHandle> NamePropertyHandle;
	/** A cached copy of strings to populate the combo box */
	TArray<TSharedPtr<FString> > RowNames;
	/** The MetaData derived filter for the row type */
	FName RowTypeFilter;
};

#undef LOCTEXT_NAMESPACE
