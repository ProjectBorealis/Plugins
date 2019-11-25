// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceTexture2D.h

#pragma once
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_SubstanceTexture2D : public FAssetTypeActions_Base
{
public:
	/** Gets the Asset Actions Name */
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SubstanceTexture2D", "Substance Texture 2D"); }

	/** Returns the icon color for this asset within the content explorer window */
	virtual FColor GetTypeColor() const override { return FColor(255, 128, 0); }

	/** Gets the asset class that this class will be effecting */
	virtual UClass* GetSupportedClass() const override { return USubstanceTexture2D::StaticClass(); }

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
		USubstanceTexture2D* Texture = Cast<USubstanceTexture2D>(AssetData.GetAsset());

		if (Texture && NULL == Texture->ParentInstance)
		{
			return FText::FromString(TEXT("Substance Texture, Graph Instance missing: Texture can't be updated."));
		}
		else
		{
			return FText::FromString(TEXT("Substance Texture."));
		}
	}

private:
	/** Searches for the Parent Graph Instance of a Substance Texture 2D*/
	void ExecuteFindParentInstance(TArray<TWeakObjectPtr<USubstanceTexture2D>> Objects);

	/** Opens the asset editor for a Substance Texture 2D */
	void virtual OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
};
