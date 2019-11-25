// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceScrollBox.cpp

#pragma once
#include "SubstanceSourceModule.h"
#include "Widgets/Layout/SScrollBox.h"

class SSubstanceScrollBox : public SScrollBox
{
public:
	/** Overriding mouse wheel to control scroll speed */
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Sets the scroll speed post creation */
	void SetScrollSpeedMultiplier(float speed);

protected:
	/** The orientation of the scrolling and stacking in the box. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scroll")
	float ScrollSpeedMultiplier;

};