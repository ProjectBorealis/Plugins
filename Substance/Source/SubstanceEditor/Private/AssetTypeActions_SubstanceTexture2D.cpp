// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceTexture2D.cpp

#include "AssetTypeActions_SubstanceTexture2D.h"
#include "SubstanceEditorPrivatePCH.h"
#include "SubstanceEditorModule.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "TextureEditor.h"

#include "Interfaces/ITextureEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SubstanceTexture2D::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto Texture = GetTypedWeakObjectPtrs<USubstanceTexture2D>(InObjects);

	MenuBuilder.AddMenuEntry(
	    LOCTEXT("GraphInstance_FindFactory", "Find parent Graph Instance"),
	    LOCTEXT("GraphInstance_FindFactory", "Find Parent Instance in content browser."),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceTexture2D::ExecuteFindParentInstance, Texture),
	        FCanExecuteAction()));
}

void FAssetTypeActions_SubstanceTexture2D::ExecuteFindParentInstance(TArray<TWeakObjectPtr<USubstanceTexture2D>> Objects)
{
	TArray<UObject*> AssetList;

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if (Object)
		{
			USubstanceTexture2D* Texture = Cast<USubstanceTexture2D>(Object);
			AssetList.AddUnique(Texture->ParentInstance);
		}
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
}

void FAssetTypeActions_SubstanceTexture2D::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Texture = Cast<UTexture>(*ObjIt);
		if (Texture != NULL)
		{
			ITextureEditorModule* TextureEditorModule = &FModuleManager::LoadModuleChecked<ITextureEditorModule>("TextureEditor");
			TextureEditorModule->CreateTextureEditor(Mode, EditWithinLevelEditor, Texture);
		}
	}
}

#undef LOCTEXT_NAMESPACE