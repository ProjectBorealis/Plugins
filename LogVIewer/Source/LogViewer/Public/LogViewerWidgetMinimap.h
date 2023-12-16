// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/SCompoundWidget.h"
#include "LogFilter.h"
#include "Misc/TextFilter.h"

class SLogViewerWidgetMinimap;
class SLogViewerWidgetMain;

class SLogViewerWidgetMinimap
	: public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SLogViewerWidgetMinimap)
		: _MainWidget()
		{}

		SLATE_ARGUMENT(SLogViewerWidgetMain*, MainWidget)
	SLATE_END_ARGS()
	~SLogViewerWidgetMinimap();

	const uint32 TextureWidth = 128;
	const uint32 TextureHeight = 1024;

	FSlateBrush Brush;
	void CreateTexture();
	void UpdateTexture(uint32 LogOffset);
	bool IsStickToTop(uint32 LogOffset) const;
	bool IsStickToBottom(uint32 LogOffset) const;

	const FColor BgColor = FColor(38, 38, 38);
	const FColor FontColor = FColor(206, 206, 206);
	const FColor ErrorColor = FColor(206, 0, 0);
	const FColor HighlightedColor = FColor(38, 149, 90);
	const FColor SelectedColor = FColor(223, 163, 9);
	const FColor WarningColor = FColor(229, 229, 5);
	const FColor BgScreenColor = FColor(90, 90, 90);

	int32 BaseLogPos = 0;

	UTexture2D* NewTexture;

	//FSlateUpdatableTexture* UpdatableTexture;

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	float Timer = 0.f;

	bool bDirty;

	void RequestUpdate() { bDirty = true; }
protected:
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	SLogViewerWidgetMain * MainWidget;

private:
	bool bMouseDown;
};
