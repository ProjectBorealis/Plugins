/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#include "MultiPackerRuntimeMatNode.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ImageUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "MultiPackerRuntimeMatNode"

UMultiPackerRuntimeMatNode::UMultiPackerRuntimeMatNode()
{
	ChangeBackground(false);
}

UMultiPackerRuntimeMatNode::~UMultiPackerRuntimeMatNode()
{
}

UMultiPackerRuntimeGraph* UMultiPackerRuntimeMatNode::GetGraph() const
{
	return Cast<UMultiPackerRuntimeGraph>(GetOuter());
}

#if WITH_EDITOR  
void UMultiPackerRuntimeMatNode::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerRuntimeMatNode, MaterialSize)))
	{
		Height = Width = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(MaterialSize);
	}
}
#endif

void UMultiPackerRuntimeMatNode::ChangeBackground(bool error)
{
	BackgroundColor = error ? FLinearColor(1.0f, 0.0f, 0.0f, 1.0f) : FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

FLinearColor UMultiPackerRuntimeMatNode::GetBackground() const
{
	return BackgroundColor;
}

#undef LOCTEXT_NAMESPACE
