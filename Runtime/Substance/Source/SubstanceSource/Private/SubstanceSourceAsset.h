// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceAsset.h

/** Description:: This class will be used to store all of the information we will need to display the source asset through the
Source window panel / viewport */

#pragma once
#include "SubstanceSourceModule.h"

class SSubstanceSourcePanel;

class SSubstanceSourceAsset
	: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSubstanceSourceAsset)
		: _Label("Substance Asset")
	{}

	SLATE_ARGUMENT(FString, Label)

	SLATE_ARGUMENT(Alg::Source::AssetPtr, Asset)

	SLATE_ARGUMENT(TWeakPtr<SSubstanceSourcePanel>, ParentWindow)

	SLATE_END_ARGS()

	/** SCompoundWidget interface */
	void Construct(const FArguments& InArgs);

	/** Destructor */
	virtual ~SSubstanceSourceAsset() override;

	/** Accessor for the display text*/
	const FString& GetLabel()
	{
		return Label;
	}

	/** Parent override, used to pull the needed asset if none created */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** Create our thumbnail and add to widget */
	void ConstructThumbnail();

	/** Flags this class to no longer request the HTTP image data */
	void SetThumbnailLoaded(bool loaded);

	/** Flags the thumbnail has been prioritized within in the cache loading queue */
	void SetPrioritizedCache();

	/** Reconstruct the layout based on if we have a thumbnail or not */
	void RegenerateTileLayout();

	/** Accessor for passing the asset associated with this widget to the factory */
	Alg::Source::AssetPtr GetAsset() const;

	/** Offset Accessor */
	FVector2D GetDownloadOverlayOffeset() const;

	/** Load event */
	DECLARE_EVENT(SSubstanceSourceAsset, FLoadCachedThumbnail)
	FLoadCachedThumbnail& OnLoadCachedThumbnail()
	{
		return LoadCachedThumbnailEvent;
	}

private:
	/** Event */
	FLoadCachedThumbnail LoadCachedThumbnailEvent;

	/** Callback for when the user opens an assets details */
	FReply OnOpenDetailsPanel();

	/** Trigger the on download of the sbsar from the database */
	FReply OnDownloadOverlayClicked(const FGeometry& Geom, const FPointerEvent& PointerEvent);

	/** Returns whether or not our thumbnail has a valid cache entry */
	bool CanLoadThumbnailFromCache() const;

	/** Loads the thumbnail from cache */
	void LoadThumbnailFromCache();

	/** Callback for when the user purchases an asset */
	void OnSubstancePurchased(Alg::Source::AssetPtr Asset);

	/** Handles clearing up the framework memory before our destructors are lost */
	void OnModuleShutdown();

	/** Store a database asset - Should automatically be cleaned up when the widget is */
	Alg::Source::AssetPtr mAsset;

	/** Stores if the thumbnail has been loaded or not */
	bool ThumbnailLoaded;

	/** The display label to use for this widget */
	FString Label;

	/** The slate brush to use to display the asset image*/
	TSharedPtr<FSlateBrush> ThumbnailBrush;

	/** Stores our resource for the texture used to clear that flags that keep it from GC */
	UObject* TextureResource;

	/** Reference to the window that we this belongs to */
	TWeakPtr<SSubstanceSourcePanel> mParentWindow;

	/** Stores the completed substance URL */
	FString SubstanceURL;

	/** Used to store the offset position for the mouse over download overlay */
	TAttribute<FVector2D> DownloadOverlayOffsetAttribute;

	/** Offset used to create the vector offset on demand */
	float DownloadOverlayOffset;

	/** Flag for if this asset has already been bumped in the cache queue */
	bool PrioritizedLoad;

	/** Toggles the download overlay to slide into place */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Toggles the download overlay to slide out of view */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Limited to updating the VFX widget */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/**Constant values used for updating and sliding the download SFxWidget. Used for clamping and speed control*/
	static const float FullVisible;
	static const float FullHidden;
	static const float SlideSpeed;

	/** Used for toggling the update of the slider on and off */
	bool IsDownloadOverlayInView;

	/** Stores the delegate to a shared pointer that needs to be cleared post shared point destrcution */
	FDelegateHandle OnModuleShutdownDelegateHandle;
	FDelegateHandle OnAssetPurchasedDelegateHandle;
};
