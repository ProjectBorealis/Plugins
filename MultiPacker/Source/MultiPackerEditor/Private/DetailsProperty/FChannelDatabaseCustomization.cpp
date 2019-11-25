/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "DetailsProperty/FChannelDatabaseCustomization.h"
#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "FChannelDatabaseCustomizationLayout"

TSharedRef<IPropertyTypeCustomization> FChannelDatabaseCustomizationLayout::MakeInstance()
{
	return MakeShareable(new FChannelDatabaseCustomizationLayout);
}

void FChannelDatabaseCustomizationLayout::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	this->StructPropertyHandle = InStructPropertyHandle;

	if (StructPropertyHandle->HasMetaData(TEXT("RowType")))
	{
		const FString& RowType = StructPropertyHandle->GetMetaData(TEXT("RowType"));
		RowTypeFilter = FName(*RowType);
	}

	HeaderRow
		.NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget(FText::GetEmpty(), FText::GetEmpty(), false)
		];
}

void FChannelDatabaseCustomizationLayout::CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	/** Get all the existing property handles */
	DatabasePropertyHandle = InStructPropertyHandle->GetChildHandle("Database");
	NamePropertyHandle = InStructPropertyHandle->GetChildHandle("Name");

	if (DatabasePropertyHandle->IsValidHandle() && NamePropertyHandle->IsValidHandle())
	{
		/** Queue up a refresh of the selected item, not safe to do from here */
		StructCustomizationUtils.GetPropertyUtilities()->EnqueueDeferredAction(FSimpleDelegate::CreateSP(this, &FChannelDatabaseCustomizationLayout::OnDataTableChanged));

		/** Setup Change callback */
		FSimpleDelegate OnDataTableChangedDelegate = FSimpleDelegate::CreateSP(this, &FChannelDatabaseCustomizationLayout::OnDataTableChanged);
		DatabasePropertyHandle->SetOnPropertyValueChanged(OnDataTableChangedDelegate);

		/** Construct a asset picker widget with a custom filter */

#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
		StructBuilder.AddChildContent(LOCTEXT("Database", "Database"))
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
			StructBuilder.AddCustomRow(LOCTEXT("Database", "Database"))
#endif
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Database", "Database"))
			.Font(StructCustomizationUtils.GetRegularFont())
			]
		.ValueContent()
			.MaxDesiredWidth(0.0f) // don't constrain the combo button width
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(DatabasePropertyHandle)
			.AllowedClass(UMultiPackerDataBase::StaticClass())
			.OnShouldFilterAsset(this, &FChannelDatabaseCustomizationLayout::ShouldFilterAsset)
			];

		/** Construct a combo box widget to select from a list of valid options */
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 16)
		StructBuilder.AddChildContent(LOCTEXT("Name", "Name"))
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 16)
			StructBuilder.AddCustomRow(LOCTEXT("Name", "Name"))
#endif
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Name", "Name"))
			.Font(StructCustomizationUtils.GetRegularFont())
			]
		.ValueContent()
			.MaxDesiredWidth(0.0f) // don't constrain the combo button width
			[
				SAssignNew(RowNameComboButton, SComboButton)
				.ToolTipText(this, &FChannelDatabaseCustomizationLayout::GetNameComboBoxContentText)
			.OnGetMenuContent(this, &FChannelDatabaseCustomizationLayout::GetListContent)
			.ContentPadding(FMargin(2.0f, 2.0f))
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &FChannelDatabaseCustomizationLayout::GetNameComboBoxContentText)
			]
			];
	}
}

TSharedPtr<FString> FChannelDatabaseCustomizationLayout::InitWidgetContent()
{
	TSharedPtr<FString> InitialValue = MakeShareable(new FString(LOCTEXT("Name", "None").ToString()));;

	FName RowName;
	const FPropertyAccess::Result RowResult = NamePropertyHandle->GetValue(RowName);
	RowNames.Empty();

	/** Get the properties we wish to work with */
	UMultiPackerDataBase* MultiPackerDataBase = NULL;
	DatabasePropertyHandle->GetValue((UObject*&)MultiPackerDataBase);
	if (MultiPackerDataBase != NULL)
	{
		/** Extract all the row names from the RowMap */
		for (TMap<FName, FTileDatabase>::TConstIterator Iterator(MultiPackerDataBase->TileMap); Iterator; ++Iterator)
		{
			/** Create a simple array of the row names */
			TSharedRef<FString> RowNameItem = MakeShareable(new FString(Iterator.Key().ToString()));
			RowNames.Add(RowNameItem);

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
		NamePropertyHandle->SetValue(NewValue);
	}
	return InitialValue;
}

TSharedRef<SWidget> FChannelDatabaseCustomizationLayout::GetListContent()
{
	SAssignNew(RowNameComboListView, SListView<TSharedPtr<FString> >)
		.ListItemsSource(&RowNames)
		.OnSelectionChanged(this, &FChannelDatabaseCustomizationLayout::OnSelectionChanged)
		.OnGenerateRow(this, &FChannelDatabaseCustomizationLayout::HandleRowNameComboBoxGenarateWidget)
		.SelectionMode(ESelectionMode::Single);

	// Ensure no filter is applied at the time the menu opens
	OnFilterTextChanged(FText::GetEmpty());

	if (CurrentSelectedItem.IsValid())
	{
		RowNameComboListView->SetSelection(CurrentSelectedItem);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSearchBox)
			.OnTextChanged(this, &FChannelDatabaseCustomizationLayout::OnFilterTextChanged)
		]
	+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			RowNameComboListView.ToSharedRef()
		];
}

void FChannelDatabaseCustomizationLayout::OnDataTableChanged()
{
	CurrentSelectedItem = InitWidgetContent();
	if (RowNameComboListView.IsValid())
	{
		RowNameComboListView->SetSelection(CurrentSelectedItem);
		RowNameComboListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> FChannelDatabaseCustomizationLayout::HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromString(*InItem))
		];
}

FText FChannelDatabaseCustomizationLayout::GetNameComboBoxContentText() const
{
	FString RowNameValue;
	const FPropertyAccess::Result RowResult = NamePropertyHandle->GetValue(RowNameValue);
	if (RowResult != FPropertyAccess::MultipleValues)
	{
		TSharedPtr<FString> SelectedRowName = CurrentSelectedItem;
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

void FChannelDatabaseCustomizationLayout::OnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentSelectedItem = SelectedItem;
		FName NewValue = FName(**SelectedItem);
		NamePropertyHandle->SetValue(NewValue);

		// Close the combo
		RowNameComboButton->SetIsOpen(false);
	}
}

void FChannelDatabaseCustomizationLayout::OnFilterTextChanged(const FText& InFilterText)
{
	FString CurrentFilterText = InFilterText.ToString();

	FName RowName;
	const FPropertyAccess::Result RowResult = NamePropertyHandle->GetValue(RowName);
	RowNames.Empty();

	/** Get the properties we wish to work with */
	UMultiPackerDataBase* MultiPackerDataBase = NULL;
	DatabasePropertyHandle->GetValue((UObject*&)MultiPackerDataBase);

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
				RowNames.Add(RowNameItem);
			}
		}
	}
	RowNameComboListView->RequestListRefresh();
}

#undef LOCTEXT_NAMESPACE
