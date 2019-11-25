// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: FSubstanceLiveLinkStyle.h
#pragma once
#include "Templates/SharedPointer.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"

/**
 * Live Link Button Style
 */
class FSubstanceLiveLinkStyle
{
public:
	/**
	 * Initializes Substance Live Link Styles
	 */
	static void Initialize();

	/**
	 * Cleans up the styles
	 */
	static void Shutdown();

	/**
	 * Get the name of the style set
	 * @returns the Name of the style set
	 */
	static FName GetStyleSetName();

	/**
	 * Get our Style object for Slate
	 * @returns Slate Style object
	 */
	static TSharedPtr<class ISlateStyle> Get()
	{
		return StyleSet;
	}

private:
	/**
	 * Append Plugin Path to Incoming Relative Path
	 * @param RelativePath - The relative path to resolve from our plugin path
	 * @param Extension - The extension to add to the path
	 * @returns The resolved path in the content folder
	 */
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:
	static TSharedPtr<class FSlateStyleSet> StyleSet;
};
