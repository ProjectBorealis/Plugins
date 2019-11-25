/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "AssetThumbnail.h"

class FMultiPackerEditorThumbnail : public FAssetThumbnailPool
{
public:
	FMultiPackerEditorThumbnail(int NumObjectsInPool) : FAssetThumbnailPool(NumObjectsInPool) {}
	static TSharedPtr<FMultiPackerEditorThumbnail> Get() { return Instance; }
	static void Create()
	{
		Instance = MakeShareable(new FMultiPackerEditorThumbnail(256));
	}
private:
	static TSharedPtr<FMultiPackerEditorThumbnail> Instance;
};
