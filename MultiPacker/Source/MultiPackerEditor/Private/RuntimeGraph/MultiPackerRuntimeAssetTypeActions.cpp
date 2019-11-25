/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "RuntimeGraph/MultiPackerRuntimeAssetTypeActions.h"
#include "RuntimeGraph/MultiPackerRuntimeAssetEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FMultiPackerRuntimeAssetTypeActions::FMultiPackerRuntimeAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FMultiPackerRuntimeAssetTypeActions::GetName() const
{
	return LOCTEXT("FMultiPackerRuntimeAssetTypeActionsName", "MultiPacker Runtime Graph");
}

FColor FMultiPackerRuntimeAssetTypeActions::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FMultiPackerRuntimeAssetTypeActions::GetSupportedClass() const
{
	return UMultiPackerRuntimeGraph::StaticClass();
}

void FMultiPackerRuntimeAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UMultiPackerRuntimeGraph* Graph = Cast<UMultiPackerRuntimeGraph>(*ObjIt))
		{
			TSharedRef<FMultiPackerRuntimeAssetEditor> NewGraphEditor(new FMultiPackerRuntimeAssetEditor());
			NewGraphEditor->InitGenericGraphAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

uint32 FMultiPackerRuntimeAssetTypeActions::GetCategories()
{
	return  MyAssetCategory;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE