// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceImageInput.cpp

#include "AssetTypeActions_SubstanceImageInput.h"
#include "SubstanceEditorPrivatePCH.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceCoreClasses.h"

#include "ContentBrowserModule.h"
#include "AssetTypeActions_Base.h"
#include "AssetRegistryModule.h"
#include "ReferencedAssetsUtils.h"
#include "Internationalization/Text.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SubstanceImageInput::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto ImageInput = GetTypedWeakObjectPtrs<USubstanceImageInput>(InObjects);

	MenuBuilder.AddMenuEntry(
	    LOCTEXT("SubstanceImageInput_Reimport", "Reimport"),
	    LOCTEXT("SubstanceImageInput_ReimportTooltip", "Reimports the selected Image Input."),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceImageInput::ExecuteReimport, ImageInput),
	        FCanExecuteAction()
	    )
	);
}

void FAssetTypeActions_SubstanceImageInput::ExecuteReimport(TArray<TWeakObjectPtr<USubstanceImageInput>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if (Object)
		{
			FReimportManager::Instance()->Reimport(Object);
		}
	}
}

#undef LOCTEXT_NAMESPACE
