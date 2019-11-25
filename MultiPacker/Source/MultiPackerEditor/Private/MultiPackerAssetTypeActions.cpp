/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerAssetTypeActions.h"
#include "MultiPackerAssetEditor/MultiPackerAssetEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FMultiPackerAssetTypeActions::FMultiPackerAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FMultiPackerAssetTypeActions::GetName() const
{
	return LOCTEXT("FMultiPackerAssetTypeActionsName", "MultiPacker");
}

FColor FMultiPackerAssetTypeActions::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FMultiPackerAssetTypeActions::GetSupportedClass() const
{
	return UMultiPacker::StaticClass();
}

void FMultiPackerAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UMultiPacker* Graph = Cast<UMultiPacker>(*ObjIt))
		{
			TSharedRef<FMultiPackerAssetEditor> NewGraphEditor(new FMultiPackerAssetEditor());
			NewGraphEditor->InitGenericGraphAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

uint32 FMultiPackerAssetTypeActions::GetCategories()
{
	return  MyAssetCategory;//EAssetTypeCategories::Animation |
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE