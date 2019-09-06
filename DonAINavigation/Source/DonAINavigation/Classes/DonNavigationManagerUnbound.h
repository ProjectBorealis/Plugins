// The MIT License(MIT)
//
// Copyright(c) 2015 Venugopalan Sreedharan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "DonNavigationManager.h"

#include "DonNavigationManagerUnbound.generated.h"

/*
* Infinite Worlds! This is the unbound version of the Navigation Manager.
* Supports unlimited map sizes. Nothing is cached, everything is looked up on-demand and for procedural games it fully eliminates the burden of having to manage dynamic collision updates.
* It is obviously slower than the Finite World equivalent but will benefit projects with huge maps or highly dynamic/frequently changing/procedural collision geometry.
*/
UCLASS()
class DONAINAVIGATION_API ADonNavigationManagerUnbound : public ADonNavigationManager
{
	GENERATED_BODY()

public:
	ADonNavigationManagerUnbound(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

protected:
	virtual void TickNavigationSolver(FDonNavigationQueryTask& task) override;
	virtual bool PrepareSolution(FDonNavigationQueryTask& Task) override;

	TArray<FVector> NeighborsAsVectors(FVector Location);
	void ExpandFrontierTowardsTarget(FDonNavigationQueryTask& Task, FVector Current, FVector Neighbor);	
};
