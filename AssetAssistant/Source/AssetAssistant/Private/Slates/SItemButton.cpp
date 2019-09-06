// Copyright 2017 Tefel. All Rights Reserved.
#include "Slates/SItemButton.h"
#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"

void SItemButton::Construct(const FArguments& InArgs)
{
	SButton::FArguments ButtonArgs = SButton::FArguments()
		.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
		.OnClicked(InArgs._OnClicked)
		.ToolTipText(InArgs._Text)
		.ContentPadding(3.0f)
		.ForegroundColor(FSlateColor::UseForeground())
		.IsFocusable(InArgs._IsFocusable)
		[
			SNew(SImage)
			.Image(InArgs._Image)
			.ColorAndOpacity(FSlateColor::UseForeground())
		];

	SButton::Construct(ButtonArgs);
}