/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

/* Based on the work of Jukka Jylanki released as Public Domain*/
//To solve the warning c4828 the diaeresis are out on the surname of Jukka//
/* https://github.com/juj/RectangleBinPack */
#pragma once

#include "BinPack/MaxRectsBinPack.h"

UMaxRectsBinPack::UMaxRectsBinPack()
	:binWidth(0),
	binHeight(0)
{
}

UMaxRectsBinPack::~UMaxRectsBinPack()
{
}

bool UMaxRectsBinPack::IsContainedIn(const FRectSizePadding &a, const FRectSizePadding &b)
{
	return a.x >= b.x && a.y >= b.y
		&& a.x + a.width <= b.x + b.width
		&& a.y + a.height <= b.y + b.height;
}

UMaxRectsBinPack::UMaxRectsBinPack(int width, int height)
{
	Init(width, height);
}

void UMaxRectsBinPack::Init(int width, int height)
{
	binWidth = width;
	binHeight = height;

	FRectSizePadding n;
	n.x = 0;
	n.y = 0;
	n.width = width;
	n.height = height;

	usedRectangles.Reset();

	freeRectangles.Reset();
	freeRectangles.Add(n);
}

void UMaxRectsBinPack::Insert(TArray<FRectSize> &rects, TArray<FRectSizePadding> &dst, EFreeRectChoiceHeuristic method)
{
	dst.Reset();

	while (rects.Num() > 0)
	{
		int bestScore1 = binWidth;
		int bestScore2 = binHeight;
		int bestRectIndex = -1;
		FRectSizePadding bestNode;

		for (int16 i = 0; i < (int16)rects.Num(); ++i)
		{
			int score1;
			int score2;
			FRectSizePadding newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);

			if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
			{
				bestScore1 = score1;
				bestScore2 = score2;
				bestNode = newNode;
				bestRectIndex = i;
			}
		}

		if (bestRectIndex == -1)
			return;

		PlaceRect(bestNode);
		rects.RemoveAt(bestRectIndex, 1, true);
	}
}

FRectSizePadding UMaxRectsBinPack::Insert(int32 width, int32 height, EFreeRectChoiceHeuristic method)
{
	FRectSizePadding newNode;
	// Unused in this function. We don't need to know the score after finding the position.
	int score1 = binWidth;
	int score2 = binHeight;
	switch (method)
	{
	case EFreeRectChoiceHeuristic::EMCE_Option1: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
	case EFreeRectChoiceHeuristic::EMCE_Option4: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
	case EFreeRectChoiceHeuristic::EMCE_Option5: newNode = FindPositionForNewNodeContactPoint(width, height, score1); break;
	case EFreeRectChoiceHeuristic::EMCE_Option2: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
	case EFreeRectChoiceHeuristic::EMCE_Option3: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
	}

	if (newNode.height == 0)
		return newNode;

	size_t numRectanglesToProcess = freeRectangles.Num();
	for (size_t i = 0; i < numRectanglesToProcess; ++i)
	{
		if (SplitFreeNode(freeRectangles[i], newNode))
		{
			freeRectangles.RemoveAt(i, 1, true);
			--i;
			--numRectanglesToProcess;
		}
	}

	PruneFreeList();

	usedRectangles.Add(newNode);
	return newNode;
}

FRectSizePadding UMaxRectsBinPack::ScoreRect(int width, int height, EFreeRectChoiceHeuristic method, int &score1, int &score2) const
{
	FRectSizePadding newNode;
	score1 = binWidth;
	score2 = binHeight;
	switch (method)
	{
	case EFreeRectChoiceHeuristic::EMCE_Option1: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
	case EFreeRectChoiceHeuristic::EMCE_Option4: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
	case EFreeRectChoiceHeuristic::EMCE_Option5: newNode = FindPositionForNewNodeContactPoint(width, height, score1);
		score1 = -score1; // Reverse since we are minimizing, but for contact point score bigger is better.
		break;
	case EFreeRectChoiceHeuristic::EMCE_Option2: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
	case EFreeRectChoiceHeuristic::EMCE_Option3: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
	}

	// Cannot fit the current rectangle.
	if (newNode.height == 0)
	{
		score1 = binWidth;
		score2 = binHeight;
	}

	return newNode;
}

/// Computes the ratio of used surface area.
float UMaxRectsBinPack::Occupancy() const
{
	unsigned long usedSurfaceArea = 0;
	for (int16 i = 0; i < (int16)usedRectangles.Num(); ++i)
		usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

	return (float)usedSurfaceArea / (binWidth * binHeight);
}

void UMaxRectsBinPack::PlaceRect(const FRectSizePadding &node)
{
	size_t numRectanglesToProcess = freeRectangles.Num();
	for (size_t i = 0; i < numRectanglesToProcess; ++i)
	{
		if (SplitFreeNode(freeRectangles[i], node))
		{
			freeRectangles.RemoveAt(i, 1, true);
			--i;
			--numRectanglesToProcess;
		}
	}

	PruneFreeList();

	usedRectangles.Add(node);
}

int UMaxRectsBinPack::ContactPointScoreNode(int x, int y, int width, int height) const
{
	int score = 0;

	if (x == 0 || x + width == binWidth)
		score += height;
	if (y == 0 || y + height == binHeight)
		score += width;

	for (int16 i = 0; i < (int16)usedRectangles.Num(); ++i)
	{
		if (usedRectangles[i].x == x + width || usedRectangles[i].x + usedRectangles[i].width == x)
		{
			if (!(usedRectangles[i].y + usedRectangles[i].height < y || y + height < usedRectangles[i].y))
				score += FMath::Min(usedRectangles[i].y + usedRectangles[i].height, y + height) - FMath::Max((int)usedRectangles[i].y, y);
		}
		if (usedRectangles[i].y == y + height || usedRectangles[i].y + usedRectangles[i].height == y)
		{
			if (!(usedRectangles[i].x + usedRectangles[i].width < x || x + width < usedRectangles[i].x))
				score += FMath::Min(usedRectangles[i].x + usedRectangles[i].width, x + width) - FMath::Max((int)usedRectangles[i].x, x);
		}
	}
	return score;
}

FRectSizePadding UMaxRectsBinPack::FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const
{
	FRectSizePadding bestNode;

	bestY = binWidth;
	bestX = binHeight;

	for (int16 i = 0; i < (int16)freeRectangles.Num(); ++i)
	{
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int topSideY = freeRectangles[i].y + height;
			if (topSideY <= bestY || (topSideY == bestY && freeRectangles[i].x <= bestX))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestY = topSideY;
				bestX = freeRectangles[i].x;
			}
		}
	}
	return bestNode;
}

FRectSizePadding UMaxRectsBinPack::FindPositionForNewNodeBestShortSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const
{
	FRectSizePadding bestNode;

	bestShortSideFit = binWidth;
	bestLongSideFit = binHeight;

	for (int16 i = 0; i < (int16)freeRectangles.Num(); ++i)
	{
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - width);
			int leftoverVert = abs(freeRectangles[i].height - height);
			int shortSideFit = FMath::Min(leftoverHoriz, leftoverVert);
			int longSideFit = FMath::Max(leftoverHoriz, leftoverVert);

			if (shortSideFit <= bestShortSideFit || (shortSideFit == bestShortSideFit && longSideFit <= bestLongSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestLongSideFit = longSideFit;
			}
		}
	}
	return bestNode;
}

FRectSizePadding UMaxRectsBinPack::FindPositionForNewNodeBestAreaFit(int32 width, int32 height, int &bestAreaFit, int &bestShortSideFit) const
{
	FRectSizePadding bestNode;

	bestAreaFit = binWidth;
	bestShortSideFit = binHeight;

	for (int16 i = 0; i < (int16)freeRectangles.Num(); ++i)
	{
		int rectaglecheck = i;
		int areaFit = freeRectangles[i].width * freeRectangles[i].height - width * height;

		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - width);
			int leftoverVert = abs(freeRectangles[i].height - height);
			int shortSideFit = FMath::Min(leftoverHoriz, leftoverVert);

			bestAreaFit = freeRectangles[i].width * freeRectangles[i].height;
			if (areaFit <= bestAreaFit && areaFit >= 0 || (areaFit == bestAreaFit && shortSideFit <= bestShortSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;
			}
		}
	}

	return bestNode;
}

FRectSizePadding UMaxRectsBinPack::FindPositionForNewNodeBestLongSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const
{
	FRectSizePadding bestNode;

	bestShortSideFit = binWidth;
	bestLongSideFit = binHeight;

	for (int16 i = 0; i < (int16)freeRectangles.Num(); ++i)
	{
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - width);
			int leftoverVert = abs(freeRectangles[i].height - height);
			int shortSideFit = FMath::Min(leftoverHoriz, leftoverVert);
			int longSideFit = FMath::Max(leftoverHoriz, leftoverVert);

			if (longSideFit <= bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit <= bestShortSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestLongSideFit = longSideFit;
			}
		}
	}
	return bestNode;
}

FRectSizePadding UMaxRectsBinPack::FindPositionForNewNodeContactPoint(int width, int height, int &bestContactScore) const
{
	FRectSizePadding bestNode;

	bestContactScore = -1;

	for (int16 i = 0; i < (int16)freeRectangles.Num(); ++i)
	{
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, width, height);
			if (score >= bestContactScore)
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestContactScore = score;
			}
		}
	}
	return bestNode;
}

bool UMaxRectsBinPack::SplitFreeNode(FRectSizePadding freeNode, const FRectSizePadding &usedNode)
{
	// Test with SAT if the rectangles even intersect.
	if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x ||
		usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
		return false;

	if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
	{
		// New node at the top side of the used node.
		if (usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height)
		{
			FRectSizePadding newNode = freeNode;
			newNode.height = usedNode.y - newNode.y;
			freeRectangles.Add(newNode);
		}

		// New node at the bottom side of the used node.
		if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
		{
			FRectSizePadding newNode = freeNode;
			newNode.y = usedNode.y + usedNode.height;
			newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
			freeRectangles.Add(newNode);
		}
	}

	if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
	{
		// New node at the left side of the used node.
		if (usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width)
		{
			FRectSizePadding newNode = freeNode;
			newNode.width = usedNode.x - newNode.x;
			freeRectangles.Add(newNode);
		}

		// New node at the right side of the used node.
		if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
		{
			FRectSizePadding newNode = freeNode;
			newNode.x = usedNode.x + usedNode.width;
			newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
			freeRectangles.Add(newNode);
		}
	}

	return true;
}

void UMaxRectsBinPack::PruneFreeList()
{
	/// Go through each pair and remove any rectangle that is redundant.
	for (int16 i = 0; i < (int16)freeRectangles.Num(); ++i)
		for (int16 j = i + 1; j < (int16)freeRectangles.Num(); ++j)
		{
			if (IsContainedIn(freeRectangles[i], freeRectangles[j]))
			{
				freeRectangles.RemoveAt(i, 1, true);
				--i;
				break;
			}
			if (IsContainedIn(freeRectangles[j], freeRectangles[i]))
			{
				freeRectangles.RemoveAt(j, 1, true);
				--j;
			}
		}
}