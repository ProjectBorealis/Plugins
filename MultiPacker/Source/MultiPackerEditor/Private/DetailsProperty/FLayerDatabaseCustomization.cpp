/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "DetailsProperty/FLayerDatabaseCustomization.h"
#include "DetailsProperty/FChannelDatabaseCustomization.h"
#include "CoreMinimal.h"
#include <DetailLayoutBuilder.h>
#include "MultiPackerDataBase.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "FLayerDatabaseCustomizationLayout"
TSharedRef<IPropertyTypeCustomization> FLayerDatabaseCustomizationLayout::MakeInstance()
{
	return MakeShareable(new FLayerDatabaseCustomizationLayout());
}

void FLayerDatabaseCustomizationLayout::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
	if (InStructPropertyHandle->GetChildHandle("Color")->HasMetaData(TEXT("RowType")))
	{
		const FString& RowType = InStructPropertyHandle->GetChildHandle("Color")->GetMetaData(TEXT("RowType"));
		ColorRowTypeFilter = FName(*RowType);
	}
	if (InStructPropertyHandle->GetChildHandle("Alpha")->HasMetaData(TEXT("RowType")))
	{
		const FString& RowType = InStructPropertyHandle->GetChildHandle("Alpha")->GetMetaData(TEXT("RowType"));
		AlphaRowTypeFilter = FName(*RowType);
	}
	HeaderRow.NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget(FText::GetEmpty(), FText::GetEmpty(), false)
		];
}

void FLayerDatabaseCustomizationLayout::CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	//Create further customization here
	ColorStructPropertyHandle = InStructPropertyHandle->GetChildHandle("Color");
	AlphaStructPropertyHandle = InStructPropertyHandle->GetChildHandle("Alpha");
	check(ColorStructPropertyHandle.IsValid());
	check(AlphaStructPropertyHandle.IsValid());
	ColorDatabasePropertyHandle = ColorStructPropertyHandle->GetChildHandle("Database");
	ColorNamePropertyHandle = ColorStructPropertyHandle->GetChildHandle("Name");
	check(ColorDatabasePropertyHandle.IsValid());
	check(ColorNamePropertyHandle.IsValid());
	AlphaDatabasePropertyHandle = AlphaStructPropertyHandle->GetChildHandle("Database");
	AlphaNamePropertyHandle = AlphaStructPropertyHandle->GetChildHandle("Name");
	check(AlphaDatabasePropertyHandle.IsValid());
	check(AlphaNamePropertyHandle.IsValid());

	/** Queue up a refresh of the selected item, not safe to do from here */
	StructCustomizationUtils.GetPropertyUtilities()->EnqueueDeferredAction(FSimpleDelegate::CreateSP(this, &FLayerDatabaseCustomizationLayout::ColorOnDataTableChanged));
	StructCustomizationUtils.GetPropertyUtilities()->EnqueueDeferredAction(FSimpleDelegate::CreateSP(this, &FLayerDatabaseCustomizationLayout::AlphaOnDataTableChanged));

	/** Setup Change callback */
	FSimpleDelegate ColorOnDataTableChangedDelegate = FSimpleDelegate::CreateSP(this, &FLayerDatabaseCustomizationLayout::ColorOnDataTableChanged);
	ColorDatabasePropertyHandle->SetOnPropertyValueChanged(ColorOnDataTableChangedDelegate);

	FSimpleDelegate AlphaOnDataTableChangedDelegate = FSimpleDelegate::CreateSP(this, &FLayerDatabaseCustomizationLayout::AlphaOnDataTableChanged);
	AlphaDatabasePropertyHandle->SetOnPropertyValueChanged(AlphaOnDataTableChangedDelegate);

	/** Construct a asset picker widget with a custom filter */
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
	StructBuilder.AddChildContent(LOCTEXT("Database", "MultiPacker Databases"))
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
		StructBuilder.AddCustomRow(LOCTEXT("Database", "MultiPacker Databases"))
#endif
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Database", "MultiPacker Databases"))
			.Font(StructCustomizationUtils.GetRegularFont())
		]
		.ValueContent()
			.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(3.0, 0.0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Color", "Color Database"))
					.Font(StructCustomizationUtils.GetRegularFont())
				]
				+ SVerticalBox::Slot()
					[
						SNew(SObjectPropertyEntryBox)
						.PropertyHandle(ColorDatabasePropertyHandle)
						.AllowedClass(UMultiPackerDataBase::StaticClass())
					.OnShouldFilterAsset(this, &FLayerDatabaseCustomizationLayout::ColorShouldFilterAsset)
				]
				+ SVerticalBox::Slot()
				[
					SAssignNew(ColorRowNameComboButton, SComboButton)
					.ToolTipText(this, &FLayerDatabaseCustomizationLayout::GetColorNameComboBoxContentText)
					.OnGetMenuContent(this, &FLayerDatabaseCustomizationLayout::GetColorListContent)
					.ContentPadding(FMargin(2.0f, 2.0f))
					.ButtonContent()
					[
						SNew(STextBlock)
						.Text(this, &FLayerDatabaseCustomizationLayout::GetColorNameComboBoxContentText)
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(3.0, 0.0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Alpha", "Alpha Database"))
				.Font(StructCustomizationUtils.GetRegularFont())
				]
			+ SVerticalBox::Slot()
				[
					SNew(SObjectPropertyEntryBox)
					.PropertyHandle(AlphaDatabasePropertyHandle)
				.AllowedClass(UMultiPackerDataBase::StaticClass())
				.OnShouldFilterAsset(this, &FLayerDatabaseCustomizationLayout::AlphaShouldFilterAsset)
				]
			+ SVerticalBox::Slot()
				[
					SAssignNew(AlphaRowNameComboButton, SComboButton)
					.ToolTipText(this, &FLayerDatabaseCustomizationLayout::GetAlphaNameComboBoxContentText)
				.OnGetMenuContent(this, &FLayerDatabaseCustomizationLayout::GetAlphaListContent)
				.ContentPadding(FMargin(2.0f, 2.0f))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FLayerDatabaseCustomizationLayout::GetAlphaNameComboBoxContentText)
				]
				]

			]
		];
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
		StructBuilder.AddChildContent(LOCTEXT("Booleans", "Booleans"))
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
			StructBuilder.AddCustomRow(LOCTEXT("Booleans", "Booleans"))
#endif
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Booleans", "Booleans"))
			.Font(StructCustomizationUtils.GetRegularFont())
			]
		.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.Padding(3.0, 0.0)
			[
				SNew(SProperty, InStructPropertyHandle->GetChildHandle("SDF"))
			]
		+ SHorizontalBox::Slot()
			.Padding(3.0, 0.0)
			[
				SNew(SProperty, InStructPropertyHandle->GetChildHandle("UseColor"))
			]
		+ SHorizontalBox::Slot()
			.Padding(3.0, 0.0)
			[
				SNew(SProperty, InStructPropertyHandle->GetChildHandle("Outline"))
			]
			];
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
		StructBuilder.AddChildContent(LOCTEXT("Property", "Property"))
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
			StructBuilder.AddCustomRow(LOCTEXT("Property", "Property"))
#endif
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Property", "Property"))
			.Font(StructCustomizationUtils.GetRegularFont())
			]
		.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SProperty, InStructPropertyHandle->GetChildHandle("EdgeSoftness"))
			];
}

TSharedPtr<FString> FLayerDatabaseCustomizationLayout::InitWidgetContent(bool IsColor)
{
	TSharedPtr<FString> InitialValue = MakeShareable(new FString(LOCTEXT("Name", "None").ToString()));;

	if (IsColor)
	{
		FName RowName;
		const FPropertyAccess::Result RowResult = ColorNamePropertyHandle->GetValue(RowName);
		ColorRowNames.Empty();

		/** Get the properties we wish to work with */
		UMultiPackerDataBase* MultiPackerDataBase = NULL;
		ColorDatabasePropertyHandle->GetValue((UObject*&)MultiPackerDataBase);
		if (MultiPackerDataBase != NULL)
		{
			/** Extract all the row names from the RowMap */
			for (TMap<FName, FTileDatabase>::TConstIterator Iterator(MultiPackerDataBase->TileMap); Iterator; ++Iterator)
			{
				/** Create a simple array of the row names */
				TSharedRef<FString> RowNameItem = MakeShareable(new FString(Iterator.Key().ToString()));
				ColorRowNames.Add(RowNameItem);

				/** Set the initial value to the currently selected item */
				if (Iterator.Key() == RowName)
				{
					InitialValue = RowNameItem;
				}
			}
		}

		/** Reset the initial value to ensure a valid entry is set */
		if (RowResult != FPropertyAccess::MultipleValues)
		{
			FName NewValue = FName(**InitialValue);
			ColorNamePropertyHandle->SetValue(NewValue);
		}
	}
	else
	{
		FName RowName;
		const FPropertyAccess::Result RowResult = ColorNamePropertyHandle->GetValue(RowName);
		ColorRowNames.Empty();

		/** Get the properties we wish to work with */
		UMultiPackerDataBase* MultiPackerDataBase = NULL;
		ColorDatabasePropertyHandle->GetValue((UObject*&)MultiPackerDataBase);
		if (MultiPackerDataBase != NULL)
		{
			/** Extract all the row names from the RowMap */
			for (TMap<FName, FTileDatabase>::TConstIterator Iterator(MultiPackerDataBase->TileMap); Iterator; ++Iterator)
			{
				/** Create a simple array of the row names */
				TSharedRef<FString> RowNameItem = MakeShareable(new FString(Iterator.Key().ToString()));
				ColorRowNames.Add(RowNameItem);

				/** Set the initial value to the currently selected item */
				if (Iterator.Key() == RowName)
				{
					InitialValue = RowNameItem;
				}
			}
		}

		/** Reset the initial value to ensure a valid entry is set */
		if (RowResult != FPropertyAccess::MultipleValues)
		{
			FName NewValue = FName(**InitialValue);
			ColorNamePropertyHandle->SetValue(NewValue);
		}
	}
	return InitialValue;
}

TSharedRef<SWidget> FLayerDatabaseCustomizationLayout::GetColorListContent()
{
	SAssignNew(ColorRowNameComboListView, SListView<TSharedPtr<FString> >)
		.ListItemsSource(&ColorRowNames)
		.OnSelectionChanged(this, &FLayerDatabaseCustomizationLayout::ColorOnSelectionChanged)
		.OnGenerateRow(this, &FLayerDatabaseCustomizationLayout::HandleRowNameComboBoxGenarateWidget)
		.SelectionMode(ESelectionMode::Single);

	// Ensure no filter is applied at the time the menu opens
	ColorOnFilterTextChanged(FText::GetEmpty());

	if (ColorCurrentSelectedItem.IsValid())
	{
		ColorRowNameComboListView->SetSelection(ColorCurrentSelectedItem);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSearchBox)
			.OnTextChanged(this, &FLayerDatabaseCustomizationLayout::ColorOnFilterTextChanged)
		]
	+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			ColorRowNameComboListView.ToSharedRef()
		];
}

TSharedRef<SWidget> FLayerDatabaseCustomizationLayout::GetAlphaListContent()
{
	SAssignNew(AlphaRowNameComboListView, SListView<TSharedPtr<FString> >)
		.ListItemsSource(&AlphaRowNames)
		.OnSelectionChanged(this, &FLayerDatabaseCustomizationLayout::AlphaOnSelectionChanged)
		.OnGenerateRow(this, &FLayerDatabaseCustomizationLayout::HandleRowNameComboBoxGenarateWidget)
		.SelectionMode(ESelectionMode::Single);

	// Ensure no filter is applied at the time the menu opens
	AlphaOnFilterTextChanged(FText::GetEmpty());

	if (AlphaCurrentSelectedItem.IsValid())
	{
		AlphaRowNameComboListView->SetSelection(AlphaCurrentSelectedItem);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSearchBox)
			.OnTextChanged(this, &FLayerDatabaseCustomizationLayout::AlphaOnFilterTextChanged)
		]
	+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			AlphaRowNameComboListView.ToSharedRef()
		];
}

void FLayerDatabaseCustomizationLayout::ColorOnDataTableChanged()
{
	ColorCurrentSelectedItem = InitWidgetContent(true);
	if (ColorRowNameComboListView.IsValid())
	{
		ColorRowNameComboListView->SetSelection(ColorCurrentSelectedItem);
		ColorRowNameComboListView->RequestListRefresh();
	}
}

void FLayerDatabaseCustomizationLayout::AlphaOnDataTableChanged()
{
	AlphaCurrentSelectedItem = InitWidgetContent(false);
	if (AlphaRowNameComboListView.IsValid())
	{
		AlphaRowNameComboListView->SetSelection(AlphaCurrentSelectedItem);
		AlphaRowNameComboListView->RequestListRefresh();
	}
}

FText FLayerDatabaseCustomizationLayout::GetColorNameComboBoxContentText() const
{
	FString RowNameValue;
	const FPropertyAccess::Result RowResult = ColorNamePropertyHandle->GetValue(RowNameValue);
	if (RowResult != FPropertyAccess::MultipleValues)
	{
		TSharedPtr<FString> SelectedRowName = ColorCurrentSelectedItem;
		if (SelectedRowName.IsValid())
		{
			return FText::FromString(*SelectedRowName);
		}
		else
		{
			return LOCTEXT("Database", "None");
		}
	}
	return LOCTEXT("MultipleValues", "Multiple Values");
}

FText FLayerDatabaseCustomizationLayout::GetAlphaNameComboBoxContentText() const
{
	FString RowNameValue;
	const FPropertyAccess::Result RowResult = AlphaNamePropertyHandle->GetValue(RowNameValue);
	if (RowResult != FPropertyAccess::MultipleValues)
	{
		TSharedPtr<FString> SelectedRowName = AlphaCurrentSelectedItem;
		if (SelectedRowName.IsValid())
		{
			return FText::FromString(*SelectedRowName);
		}
		else
		{
			return LOCTEXT("Database", "None");
		}
	}
	return LOCTEXT("MultipleValues", "Multiple Values");
}
TSharedRef<ITableRow> FLayerDatabaseCustomizationLayout::HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromString(*InItem))
		];
}

void FLayerDatabaseCustomizationLayout::ColorOnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		ColorCurrentSelectedItem = SelectedItem;
		FName NewValue = FName(**SelectedItem);
		ColorNamePropertyHandle->SetValue(NewValue);

		// Close the combo
		ColorRowNameComboButton->SetIsOpen(false);
	}
}

void FLayerDatabaseCustomizationLayout::AlphaOnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		AlphaCurrentSelectedItem = SelectedItem;
		FName NewValue = FName(**SelectedItem);
		AlphaNamePropertyHandle->SetValue(NewValue);

		// Close the combo
		AlphaRowNameComboButton->SetIsOpen(false);
	}
}

void FLayerDatabaseCustomizationLayout::ColorOnFilterTextChanged(const FText& InFilterText)
{
	FString CurrentFilterText = InFilterText.ToString();

	FName RowName;
	const FPropertyAccess::Result RowResult = ColorNamePropertyHandle->GetValue(RowName);
	ColorRowNames.Empty();

	/** Get the properties we wish to work with */
	UMultiPackerDataBase* MultiPackerDataBase = NULL;
	ColorDatabasePropertyHandle->GetValue((UObject*&)MultiPackerDataBase);

	if (MultiPackerDataBase != NULL)
	{
		/** Extract all the row names from the RowMap */
		for (TMap<FName, FTileDatabase>::TConstIterator Iterator(MultiPackerDataBase->TileMap); Iterator; ++Iterator)
		{
			/** Create a simple array of the row names */
			FString RowString = Iterator.Key().ToString();
			if (CurrentFilterText == TEXT("") || RowString.Contains(CurrentFilterText))
			{
				TSharedRef<FString> RowNameItem = MakeShareable(new FString(RowString));
				ColorRowNames.Add(RowNameItem);
			}
		}
	}
	ColorRowNameComboListView->RequestListRefresh();
}

void FLayerDatabaseCustomizationLayout::AlphaOnFilterTextChanged(const FText& InFilterText)
{
	FString CurrentFilterText = InFilterText.ToString();

	FName RowName;
	const FPropertyAccess::Result RowResult = AlphaNamePropertyHandle->GetValue(RowName);
	AlphaRowNames.Empty();

	/** Get the properties we wish to work with */
	UMultiPackerDataBase* MultiPackerDataBase = NULL;
	AlphaDatabasePropertyHandle->GetValue((UObject*&)MultiPackerDataBase);

	if (MultiPackerDataBase != NULL)
	{
		/** Extract all the row names from the RowMap */
		for (TMap<FName, FTileDatabase>::TConstIterator Iterator(MultiPackerDataBase->TileMap); Iterator; ++Iterator)
		{
			/** Create a simple array of the row names */
			FString RowString = Iterator.Key().ToString();
			if (CurrentFilterText == TEXT("") || RowString.Contains(CurrentFilterText))
			{
				TSharedRef<FString> RowNameItem = MakeShareable(new FString(RowString));
				AlphaRowNames.Add(RowNameItem);
			}
		}
	}
	AlphaRowNameComboListView->RequestListRefresh();
}

#undef LOCTEXT_NAMESPACE
