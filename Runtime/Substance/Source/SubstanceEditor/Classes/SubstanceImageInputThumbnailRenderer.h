// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceImageInputThumbnailRenderer.h

#pragma once
#include "ThumbnailRendering/ThumbnailRenderer.h"
#include "SubstanceImageInputThumbnailRenderer.generated.h"

UCLASS()
class USubstanceImageInputThumbnailRenderer : public UThumbnailRenderer
{
	GENERATED_UCLASS_BODY()

	/** UThumbnailRenderer Object - Renders the Image Input Thumbnail */
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas) override;
};
