#pragma once
#include "Widgets/Input/SButton.h"

class SItemButton : public SButton
{
public:
	SLATE_BEGIN_ARGS(SItemButton)
		: _Text()
		, _Image(FEditorStyle::GetBrush("Default"))
		, _IsFocusable(true)
	{}
		SLATE_ARGUMENT(FText, Text)
		SLATE_ARGUMENT(const FSlateBrush*, Image)
		SLATE_EVENT(FOnClicked, OnClicked)

		/** Sometimes a button should only be mouse-clickable and never keyboard focusable. */
		SLATE_ARGUMENT(bool, IsFocusable)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);
};
