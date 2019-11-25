// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceSourceStyle.h

#pragma once
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"

class FSubstanceSourceStyle
{
public:
	static const int ANIMLOADING_IMAGE_COUNT = 32;
	static const int ANIMLOADING_FRAME_RATE = 15;
	static const int ANIMLOADING_SPRITESHEET_COLUMNS = 6;
	static const int ANIMLOADING_SPRITESHEET_CELL_DIMENSION = 315;
	static const int ANIMLOADING_SPRITESHEET_STRIDE = ANIMLOADING_SPRITESHEET_CELL_DIMENSION * ANIMLOADING_SPRITESHEET_COLUMNS;

public:
	/** Initializes the class */
	static void Initialize();

	/** Cleans up the class */
	static void Shutdown();

	/** Return the name of the Style Set */
	static FName GetStyleSetName();

	/** Returns the shared pointer to the slate style set */
	static TSharedPtr<class ISlateStyle> Get()
	{
		return StyleSet;
	}

private:
	/** Returns a complete file path */
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

	/** Pointer to the style set */
	static TSharedPtr<class FSlateStyleSet> StyleSet;
};
