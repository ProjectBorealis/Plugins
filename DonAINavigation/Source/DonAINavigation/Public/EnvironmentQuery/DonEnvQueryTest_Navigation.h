// The MIT License(MIT)
//
// Copyright(c) 2015 Venugopalan Sreedharan
//
// Contributors: This EQS code was kindly contributed by Vladimir Ivanov (Github @ArCorvus)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Venu's Note:- This EQS code was kindly contributed by Vladimir Ivanov (Github @ArCorvus)

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "DonEnvQueryTest_Navigation.generated.h"

/**
 * 
 */
UCLASS()
class DONAINAVIGATION_API UDonEnvQueryTest_Navigation : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UDonEnvQueryTest_Navigation(const FObjectInitializer& ObjectInitializer);
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;


public:
	/**  */
	//UPROPERTY(EditDefaultsOnly, Category = Trace)
	//TSubclassOf<UEnvQueryContext> Context;

	/** Search for random location nearby */
	UPROPERTY(EditDefaultsOnly, Category = Test, meta = (InlineEditConditionToggle))
	uint32 bSearchRandomLocation : 1;

	/** Number of Attempts to find random location nearby if Item's location is not valid */
	UPROPERTY(EditDefaultsOnly, Category = Test, meta = (EditCondition = "bSearchRandomLocation"))
	int32 RandomLocationMaxAttempts;

	/** Find Random location radius */
	UPROPERTY(EditDefaultsOnly, Category = Test, meta = (EditCondition = "bSearchRandomLocation"))
	float RandomLocationRadius;

private:
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;	
	
};
