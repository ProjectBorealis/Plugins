/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

#include "LayerDatabase/SLayerStyleEditor.h"
#include "Widgets/SPanel.h"
#include <SlateMaterialBrush.h>
#include "Widgets/Images/SImage.h"
#include "Templates/SharedPointer.h"
#include "EditorStyleSet.h"
#include <Widgets/Input/SNumericEntryBox.h>
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "SLayerStyleEditor"

class SMaterialEditorUIPreviewZoomer : public SPanel
{
public:

	class FMaterialPreviewPanelSlot : public TSupportsOneChildMixin<FMaterialPreviewPanelSlot>
	{
	public:
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 19)
	FMaterialPreviewPanelSlot()
		: TSupportsOneChildMixin<FMaterialPreviewPanelSlot>()
	{
	}
};
#endif
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 19)
FMaterialPreviewPanelSlot(SWidget* InOwner)
	: TSupportsOneChildMixin<FMaterialPreviewPanelSlot>(InOwner)
{
}
};
#endif
	SLATE_BEGIN_ARGS(SLayerStyleEditor) {}
	SLATE_END_ARGS()

		SMaterialEditorUIPreviewZoomer()
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION > 19)
		: ChildSlot(this)
#endif
	{
	}

	void Construct(const FArguments& InArgs, UMaterialInterface* InPreviewMaterial);

	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	bool ZoomBy(const float Amount);
	float GetZoomLevel() const;

	void SetPreviewSize(const FVector2D PreviewSize);
	void SetPreviewMaterial(UMaterialInterface* InPreviewMaterial);
private:
	mutable FVector2D CachedSize;
	float ZoomLevel;

	FMaterialPreviewPanelSlot ChildSlot;
	TSharedPtr<FSlateMaterialBrush> PreviewBrush;
	TSharedPtr<SImage> ImageWidget;
};


void SMaterialEditorUIPreviewZoomer::Construct(const FArguments& InArgs, UMaterialInterface* InPreviewMaterial)
{
	PreviewBrush = MakeShareable(new FSlateMaterialBrush(*InPreviewMaterial, FVector2D(250, 250)));

	ChildSlot
		[
			SAssignNew(ImageWidget, SImage)
			.Image(PreviewBrush.Get())
		];

	ZoomLevel = 1.0f;
}

void SMaterialEditorUIPreviewZoomer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	CachedSize = AllottedGeometry.GetLocalSize();

	const TSharedRef<SWidget>& ChildWidget = ChildSlot.GetWidget();
	if (ChildWidget->GetVisibility() != EVisibility::Collapsed)
	{
		const FVector2D& WidgetDesiredSize = ChildWidget->GetDesiredSize();

		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(ChildWidget, FVector2D::ZeroVector, WidgetDesiredSize * ZoomLevel));
	}
}

FVector2D SMaterialEditorUIPreviewZoomer::ComputeDesiredSize(float) const
{
	FVector2D ThisDesiredSize = FVector2D::ZeroVector;

	const TSharedRef<SWidget>& ChildWidget = ChildSlot.GetWidget();
	if (ChildWidget->GetVisibility() != EVisibility::Collapsed)
	{
		ThisDesiredSize = ChildWidget->GetDesiredSize() * ZoomLevel;
	}

	return ThisDesiredSize;
}

FChildren* SMaterialEditorUIPreviewZoomer::GetChildren()
{
	return &ChildSlot;
}

FReply SMaterialEditorUIPreviewZoomer::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ZoomBy(MouseEvent.GetWheelDelta());

	return FReply::Handled();
}

int32 SMaterialEditorUIPreviewZoomer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	return LayerId;
}

bool SMaterialEditorUIPreviewZoomer::ZoomBy(const float Amount)
{
	static const float MinZoomLevel = 0.2f;
	static const float MaxZoomLevel = 4.0f;

	const float PrevZoomLevel = ZoomLevel;
	ZoomLevel = FMath::Clamp(ZoomLevel + (Amount * 0.05f), MinZoomLevel, MaxZoomLevel);
	return ZoomLevel != PrevZoomLevel;
}

float SMaterialEditorUIPreviewZoomer::GetZoomLevel() const
{
	return ZoomLevel;
}


void SMaterialEditorUIPreviewZoomer::SetPreviewSize(const FVector2D PreviewSize)
{
	PreviewBrush->ImageSize = PreviewSize;
}

void SMaterialEditorUIPreviewZoomer::SetPreviewMaterial(UMaterialInterface* InPreviewMaterial)
{
	PreviewBrush = MakeShareable(new FSlateMaterialBrush(*InPreviewMaterial, PreviewBrush->ImageSize));
	ImageWidget->SetImage(PreviewBrush.Get());
}

void SLayerStyleEditor::Construct(const FArguments& InArgs, UMaterialInterface* PreviewMaterial)
{

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(3.f)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PreviewSize", "Preview Size"))
		]
	+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(3.f)
		.MaxWidth(75)
		[
			SNew(SNumericEntryBox<int32>)
			.AllowSpin(true)
		.MinValue(1)
		.MaxSliderValue(4096)
		.OnValueChanged(this, &SLayerStyleEditor::OnPreviewXChanged)
		.OnValueCommitted(this, &SLayerStyleEditor::OnPreviewXCommitted)
		.Value(this, &SLayerStyleEditor::OnGetPreviewXValue)
		.MinDesiredValueWidth(75)
		.Label()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PreviewSize_X", "X"))
		]
		]
		]
	+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(3.f)
		.MaxWidth(75)
		[
			SNew(SNumericEntryBox<int32>)
			.AllowSpin(true)
		.MinValue(1)
		.MaxSliderValue(4096)
		.MinDesiredValueWidth(75)
		.OnValueChanged(this, &SLayerStyleEditor::OnPreviewYChanged)
		.OnValueCommitted(this, &SLayerStyleEditor::OnPreviewYCommitted)
		.Value(this, &SLayerStyleEditor::OnGetPreviewYValue)
		.Label()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PreviewSize_Y", "Y"))
		]
		]
		]
		]
		]
	+ SVerticalBox::Slot()
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderImage(FEditorStyle::GetBrush("BlackBrush"))
		[
			SAssignNew(PreviewZoomer, SMaterialEditorUIPreviewZoomer, PreviewMaterial)
		]
		]
		];

	PreviewSize = FIntPoint(250, 250);
	PreviewZoomer->SetPreviewSize(FVector2D(PreviewSize));
}

void SLayerStyleEditor::SetPreviewMaterial(UMaterialInterface* InMaterialInterface)
{
	PreviewZoomer->SetPreviewMaterial(InMaterialInterface);
}

void SLayerStyleEditor::OnPreviewXChanged(int32 NewValue)
{
	PreviewSize.X = NewValue;
	PreviewZoomer->SetPreviewSize(FVector2D(PreviewSize));

}

void SLayerStyleEditor::OnPreviewXCommitted(int32 NewValue, ETextCommit::Type)
{
	OnPreviewXChanged(NewValue);
}


void SLayerStyleEditor::OnPreviewYChanged(int32 NewValue)
{
	PreviewSize.Y = NewValue;
	PreviewZoomer->SetPreviewSize(FVector2D(PreviewSize));

}

void SLayerStyleEditor::OnPreviewYCommitted(int32 NewValue, ETextCommit::Type)
{
	OnPreviewYChanged(NewValue);
}

#undef LOCTEXT_NAMESPACE