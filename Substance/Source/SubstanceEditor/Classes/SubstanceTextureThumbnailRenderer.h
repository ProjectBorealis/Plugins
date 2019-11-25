// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceTextureThumbnailRenderer.h

#pragma once
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "SubstanceTextureThumbnailRenderer.generated.h"

UCLASS()
class USubstanceTextureThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_UCLASS_BODY()

	/** UThumbnailRenderer Object */
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas) override;
};
