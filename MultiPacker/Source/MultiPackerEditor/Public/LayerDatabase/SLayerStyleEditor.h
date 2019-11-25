/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */
#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UMaterialInterface;
class SMaterialEditorUIPreviewZoomer;

/**
* A preview viewport used for 2D UI materials
*/
class SLayerStyleEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLayerStyleEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMaterialInterface* PreviewMaterial);
	void SetPreviewMaterial(UMaterialInterface* InMaterialInterface);

private:
	void OnPreviewXChanged(int32 NewValue);
	void OnPreviewXCommitted(int32 NewValue, ETextCommit::Type);
	void OnPreviewYChanged(int32 NewValue);
	void OnPreviewYCommitted(int32 NewValue, ETextCommit::Type);
	TOptional<int32> OnGetPreviewXValue() const { return PreviewSize.X; }
	TOptional<int32> OnGetPreviewYValue() const { return PreviewSize.Y; }
private:
	FIntPoint PreviewSize;
	TSharedPtr<class SMaterialEditorUIPreviewZoomer> PreviewZoomer;
};