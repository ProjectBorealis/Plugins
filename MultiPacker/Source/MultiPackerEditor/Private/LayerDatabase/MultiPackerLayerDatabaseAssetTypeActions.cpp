/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "LayerDatabase/MultiPackerLayerDatabaseAssetTypeActions.h"
#include "LayerDatabase/LayerStyleEditorToolkit.h"
#include "MultiPackerLayerDatabase.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FMultiPackerLayerDatabaseAssetTypeActions::FMultiPackerLayerDatabaseAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
	
}

FText FMultiPackerLayerDatabaseAssetTypeActions::GetName() const
{
	return LOCTEXT("FMultiPackerLayerDatabaseAssetTypeActionsName", "LayerDatabase");
}

FColor FMultiPackerLayerDatabaseAssetTypeActions::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FMultiPackerLayerDatabaseAssetTypeActions::GetSupportedClass() const
{
	return UMultiPackerLayerDatabase::StaticClass();
}

void FMultiPackerLayerDatabaseAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UMultiPackerLayerDatabase* Graph = Cast<UMultiPackerLayerDatabase>(*ObjIt))
		{
			TSharedRef<FLayerStyleEditorToolkit> NewGraphEditor(new FLayerStyleEditorToolkit());
			NewGraphEditor->Initialize(Graph, Mode, EditWithinLevelEditor);
		}
	}
}

uint32 FMultiPackerLayerDatabaseAssetTypeActions::GetCategories()
{
	return  MyAssetCategory;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE