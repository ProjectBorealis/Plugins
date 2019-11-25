/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "MultiPackerEditorTypes.generated.h"

UCLASS()
class UMultiPackerEditorTypes : public UObject
{
	GENERATED_UCLASS_BODY()

	static const FString SPinMaterialNode;
	static const FString SPinTextureNode;
	static const FString SPinOutputNode;
	static const FString PinCategory_MultipleNodes;
	static const FString PinCategory_SingleNode;
	static const FString PinCategory_RGB;
	static const FString PinCategory_Red;
	static const FString PinCategory_Green;
	static const FString PinCategory_Blue;
	static const FString PinCategory_Alpha;
	static const FString PinCategory_RGBA;
};
