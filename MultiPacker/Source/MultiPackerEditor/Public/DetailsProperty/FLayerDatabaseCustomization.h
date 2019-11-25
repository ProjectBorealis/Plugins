/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "IPropertyUtilities.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FLayerDatabaseCustomizationLayout"

/**
* Customizes a DataTable asset to use a dropdown
*/
class FLayerDatabaseCustomizationLayout : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
	bool ColorShouldFilterAsset(const class FAssetData& AssetData)
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
		bool ColorShouldFilterAsset(const struct FAssetData& AssetData)
#endif
	{
		if (!ColorRowTypeFilter.IsNone())
		{
			const UMultiPackerDataBase* MultiPackerDataBase = Cast<UMultiPackerDataBase>(AssetData.GetAsset());
			if (MultiPackerDataBase->RowStruct && MultiPackerDataBase->RowStruct->GetFName() == ColorRowTypeFilter)
			{
				return false;
			}
			return true;
		}
		return false;
	}
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
	bool AlphaShouldFilterAsset(const class FAssetData& AssetData)
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
	bool AlphaShouldFilterAsset(const struct FAssetData& AssetData)
#endif
	{
		if (!AlphaRowTypeFilter.IsNone())
		{
			const UMultiPackerDataBase* MultiPackerDataBase = Cast<UMultiPackerDataBase>(AssetData.GetAsset());
			if (MultiPackerDataBase->RowStruct && MultiPackerDataBase->RowStruct->GetFName() == AlphaRowTypeFilter)
			{
				return false;
			}
			return true;
		}
		return false;
	}
	/** Init the contents the combobox sources its data off */
	TSharedPtr<FString> InitWidgetContent(bool IsColor);

	/** Returns the ListView for the ComboButton */
	TSharedRef<SWidget> GetColorListContent();
	TSharedRef<SWidget> GetAlphaListContent();

	/** Delegate to refresh the drop down when the datatable changes */
	void ColorOnDataTableChanged();
	void AlphaOnDataTableChanged();

	/** Display the current selection */
	FText GetColorNameComboBoxContentText() const;
	
	/** Display the current selection */
	FText GetAlphaNameComboBoxContentText() const;

	/** Return the representation of the the row names to display */
	TSharedRef<ITableRow>  HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** Update the root data on a change of selection */
	void ColorOnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);

	/** Update the root data on a change of selection */
	void AlphaOnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);

	/** Called by Slate when the filter box changes text. */
	void ColorOnFilterTextChanged(const FText& InFilterText);

	/** Called by Slate when the filter box changes text. */
	void AlphaOnFilterTextChanged(const FText& InFilterText);

	/** The comboButton objects */
	TSharedPtr<SComboButton> ColorRowNameComboButton;
	TSharedPtr<SComboButton> AlphaRowNameComboButton;
	TSharedPtr<FString> ColorCurrentSelectedItem;
	TSharedPtr<FString> AlphaCurrentSelectedItem;
	TSharedPtr<SListView<TSharedPtr<FString> > > ColorRowNameComboListView;
	TSharedPtr<SListView<TSharedPtr<FString> > > AlphaRowNameComboListView;
	/** Handle to the struct properties being customized */
	TSharedPtr<IPropertyHandle> ColorStructPropertyHandle;
	TSharedPtr<IPropertyHandle> ColorDatabasePropertyHandle;
	TSharedPtr<IPropertyHandle> ColorNamePropertyHandle;
	TSharedPtr<IPropertyHandle> AlphaStructPropertyHandle;
	TSharedPtr<IPropertyHandle> AlphaDatabasePropertyHandle;
	TSharedPtr<IPropertyHandle> AlphaNamePropertyHandle;
	/** A cached copy of strings to populate the combo box */
	TArray<TSharedPtr<FString> > ColorRowNames;
	TArray<TSharedPtr<FString> > AlphaRowNames;
	/** The MetaData derived filter for the row type */
	FName ColorRowTypeFilter;
	FName AlphaRowTypeFilter;
};

#undef LOCTEXT_NAMESPACE
