// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceDetails.h

/** Description:: This class will contain the panel widget for the contents of the details window within the main panel when the user
clicks on an asset tile to view its details */

#pragma once
#include "SubstanceSourceModule.h"

// Forward declarations
class SSubstanceSourcePanel;
struct FSlateBrush;

class SSubstanceSourceDetails
	: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSubstanceSourceDetails) = default;

	SLATE_ARGUMENT(Alg::Source::AssetPtr, Asset)

	SLATE_ARGUMENT(TWeakPtr<SSubstanceSourcePanel>, ParentWindow)

	SLATE_END_ARGS()

	/** SCompoundWidget interface */
	void Construct(const FArguments& InArgs);

	/** Destructor */
	virtual ~SSubstanceSourceDetails() override;

	/** Create our thumbnail and add to widget */
	void ConstructDetailsSlicesImage();

	/** Create our output maps thumbnail and add to widget */
	void ConstructDetailsMapsImage();

	/** Reconstruct the layout based on if we have a thumbnail or not */
	void RegeneratePanelLayout();

	/** Accessor for passing the asset associated with this widget to the factory */
	Alg::Source::AssetPtr GetAsset() const;

	/** Heart beat that should only ever be used for updating and checking the timer */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** Switches the detail image to the next */
	FReply SwitchNextImage();

	/** Callback for when the user presses the close button */
	FReply OnDetailWindowCloseButton();

	/** Handles clearing up the framework memory before our destructors are lost */
	void OnModuleShutdown();

private:
	/** Timer for when to next switch the detail images*/
	float mFlipTimer;

	/** Have images been loaded yet? */
	bool mImagesLoaded;

	/** Stores our resource for the textures used to clear that flags that keep it from GC */
	TArray<UObject*> mTextureResources;

	/** Store a database asset - Should automatically be cleaned up when the widget is */
	Alg::Source::AssetPtr mAsset;

	/** The slate brush to use to display the asset image*/
	TSharedPtr<FSlateBrush> DetailsSlicesImageBrush;
	TSharedPtr<FSlateBrush> DetailsMapsImageBrush;
	TSharedPtr<FSlateBrush> CurrentSlateBrush;

	/** Reference to the window that we this belongs to */
	TWeakPtr<SSubstanceSourcePanel> mParentWindow;
};
