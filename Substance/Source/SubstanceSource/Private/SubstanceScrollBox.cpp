// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceScrollBox.cpp

#include "SubstanceScrollBox.h"
#include "SubstanceSourcePrivatePCH.h"

FReply SSubstanceScrollBox::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector2D SimulatedScrollDelta = MouseEvent.GetGestureDelta() * ScrollSpeedMultiplier;

	//#NOTE::The only relevant variable below for the parent function is the gesture delta but
	//we are still copying over the data the can be accessed from the base mouse event.
	//This is more or less a hack but as we can't recreate this function due to many of the
	//needed elements being private within the parent class, we are just recreating the mouse event
	//simulating more mouse scroll than what actually occurred.
	FPointerEvent ModifiedMouseEvent(
	    MouseEvent.GetScreenSpacePosition(),
	    MouseEvent.GetLastScreenSpacePosition(),
	    TSet<FKey>(),
	    FModifierKeysState(),
	    MouseEvent.GetGestureType(),
	    SimulatedScrollDelta,
	    MouseEvent.IsDirectionInvertedFromDevice()
	);

	return SScrollBox::OnMouseWheel(MyGeometry, ModifiedMouseEvent);
}

void SSubstanceScrollBox::SetScrollSpeedMultiplier(float speed)
{
	ScrollSpeedMultiplier = speed;
}
