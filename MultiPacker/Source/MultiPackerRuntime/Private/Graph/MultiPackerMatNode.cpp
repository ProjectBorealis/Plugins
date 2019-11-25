/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "Graph/MultiPackerMatNode.h"
#include "Graph/MultiPacker.h"
#include "Materials/MaterialInterface.h"

#define LOCTEXT_NAMESPACE "MultiPackerMatNode"

UMultiPackerMatNode::UMultiPackerMatNode()
{
	ChangeBackground(false);
	ErrorSize = RectangleSize ? CompareSizes(UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY), UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY)) : false;
}

UMultiPacker* UMultiPackerMatNode::GetGraph() const
{
	return Cast<UMultiPacker>(GetOuter());
}

#if WITH_EDITOR  
void UMultiPackerMatNode::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UMultiPackerMatNode, SetSizeByParameter)))
	{
		SizeVertical = SizeHorizontal = UMultiPackerBaseEnums::GetTextureSizeOutputEnum(SetSizeByParameter);
		ErrorSize = RectangleSize ? CompareSizes(UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY), UMultiPackerBaseEnums::GetTextureSizeOutputEnum(GetGraph()->OutputSizeY)) : false;
	}
}
#endif

bool UMultiPackerMatNode::CanProcess()
{
	if (MaterialBaseInput)
		bMatAsset = true;
	bCanProcess = !ErrorSize && !ErrorTiles && bMatAsset;
	return bCanProcess;
}

bool UMultiPackerMatNode::CompareSizes(int32 Vertical, int32 Horizontal)
{
	ErrorSize = false;
	if (SizeVertical / TilesVertical > Vertical)
		ErrorSize = true;
	if (SizeHorizontal / TilesHorizontal > Horizontal)
		ErrorSize = true;
	return ErrorSize;
}

void UMultiPackerMatNode::SetErrorTiles(bool error)
{
	ErrorTiles = error;
}

void UMultiPackerMatNode::ChangeBackground(bool error)
{
	BackgroundColor = error ? FLinearColor(1.0f, 0.0f, 0.0f, 1.0f) : FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

FLinearColor UMultiPackerMatNode::GetBackground() const
{
	return BackgroundColor;
}

#undef LOCTEXT_NAMESPACE
