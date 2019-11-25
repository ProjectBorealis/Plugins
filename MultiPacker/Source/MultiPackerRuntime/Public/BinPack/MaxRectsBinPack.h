/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

/* Based on the work of Jukka Jylanki released as Public Domain*/
//To solve the warning c4828 the diaeresis are out on the surname of Jukka//
#pragma once

#include "CoreMinimal.h"
#include "MultiPackerBaseEnums.h"
#include "MaxRectsBinPack.generated.h"

/** UMaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that
use this structure. */
UCLASS(hidecategories = object)
class MULTIPACKERRUNTIME_API UMaxRectsBinPack : public UObject
{
	GENERATED_BODY()

public:
	/// Instantiates a bin of size (0,0). Call Init to create a new bin.
	UMaxRectsBinPack();
	~UMaxRectsBinPack();

	/// Returns true if a is contained in b.
	bool IsContainedIn(const FRectSizePadding &a, const FRectSizePadding &b);

	/// Instantiates a bin of the given size.
	UMaxRectsBinPack(int width, int height);


	/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
	/// you need to restart with a new bin.
	void Init(int width, int height);

	/// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
	/// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
	/// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
	/// @param method The rectangle placement rule to use when packing.
	void Insert(TArray<FRectSize> &rects, TArray<FRectSizePadding> &dst, EFreeRectChoiceHeuristic method);
	/// Inserts a single rectangle into the bin, possibly rotated.
	FRectSizePadding Insert(int32 width, int32 height, EFreeRectChoiceHeuristic method);

	/// Computes the ratio of used surface area to the total bin area.
	float Occupancy() const;

private:
	int binWidth;
	int binHeight;

	TArray<FRectSizePadding> usedRectangles;
	TArray<FRectSizePadding> freeRectangles;

	/// Computes the placement score for placing the given rectangle with the given method.
	/// @param score1 [out] The primary placement score will be outputted here.
	/// @param score2 [out] The secondary placement score will be outputted here. This isu sed to break ties.
	/// @return This struct identifies where the rectangle would be placed if it were placed.
	FRectSizePadding ScoreRect(int width, int height, EFreeRectChoiceHeuristic method, int &score1, int &score2) const;
	
	/// Places the given rectangle into the bin.
	void PlaceRect(const FRectSizePadding &node);

	/// Computes the placement score for the -CP variant.
	int ContactPointScoreNode(int x, int y, int width, int height) const;

	FRectSizePadding FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const;
	FRectSizePadding FindPositionForNewNodeBestShortSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
	FRectSizePadding FindPositionForNewNodeBestLongSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
	FRectSizePadding FindPositionForNewNodeBestAreaFit(int32 width, int32 height, int &bestAreaFit, int &bestShortSideFit) const;
	FRectSizePadding FindPositionForNewNodeContactPoint(int width, int height, int &contactScore) const;
	
	/// @return True if the free node was split.
	bool SplitFreeNode(FRectSizePadding freeNode, const FRectSizePadding &usedNode);

	//int CommonIntervalLength(int i1start, int i1end, int i2start, int i2end);
	/// Goes through the free rectangle list and removes any redundant entries.
	void PruneFreeList();
};

