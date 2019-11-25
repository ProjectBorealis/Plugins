// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceImageInput.h

#pragma once
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_SubstanceImageInput : public FAssetTypeActions_Base
{
public:
	/** Gets the Asset Actions Name */
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SubstanceImageInput", "Substance Image Input"); }

	/** Returns the icon color for this asset within the content explorer window */
	virtual FColor GetTypeColor() const override { return FColor(192, 128, 32); }

	/** Gets the asset class that this class will be effecting */
	virtual UClass* GetSupportedClass() const override { return USubstanceImageInput::StaticClass(); }

	/** Checks to see if the supported asset has actions */
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }

	/** Gets the actions for the supported asset */
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

	/** Returns the Categories this asset belongs to (EAssetTypeCategories) */
	virtual uint32 GetCategories() override { return EAssetTypeCategories::MaterialsAndTextures; }

protected:
	/** Returns additional tooltip information for the specified asset, if it has any (otherwise return the null widget) */
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return FText::FromString(TEXT("Substance Image Input."));
	}

private:
	/** Handler for when Reimport is selected */
	void ExecuteReimport(TArray<TWeakObjectPtr<USubstanceImageInput>> Objects);
};
