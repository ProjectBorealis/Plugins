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
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "Multithreading/DonNavigationWorker.h"
#include "../DonAINavigationPrivatePCH.h"

#include "DonNavigationManager.h"

FDonNavigationWorker::FDonNavigationWorker()
{

}

FDonNavigationWorker::FDonNavigationWorker(ADonNavigationManager* Manager, int32 MaxPathSolverIterations, int32 MaxCollisionSolverIterations) 
				     : Manager(Manager), 
					   MaxPathSolverIterations(MaxPathSolverIterations),
					   MaxCollisionSolverIterations(MaxCollisionSolverIterations)
{	
	Thread = FRunnableThread::Create(this, TEXT("DonNavigationWorker"), 0U, TPri_BelowNormal);
}

FDonNavigationWorker::~FDonNavigationWorker()
{
	delete Thread;

	Thread = NULL;
}

void FDonNavigationWorker::ShutDown()
{
	Stop();

	Thread->WaitForCompletion();
}

bool FDonNavigationWorker::Init() 
{
	if (Manager) 
	{
		UE_LOG(DoNNavigationLog, Log, TEXT("FDonNavigationWorker thread started"));
		
		return true;
	}
	return false;
}

uint32 FDonNavigationWorker::Run()
{
	FPlatformProcess::Sleep(0.03f);

	while (StopTaskCounter.GetValue() == 0)
	{
		//Manager->ReceiveAsyncAbortRequests();
		Manager->ReceiveAsyncNavigationTasks();		
		Manager->ReceiveAsyncCollisionTasks();

		SolveNavigationTasks();

		//FPlatformProcess::Sleep(0.2f);
	}
	return 0;
}

void FDonNavigationWorker::Stop() 
{
	StopTaskCounter.Increment();
}

void FDonNavigationWorker::SolveNavigationTasks()
{
	Manager->TickScheduledPathfindingTasks_Safe(0.f, MaxPathSolverIterations);

	Manager->TickScheduledCollisionTasks_Safe(0.f, MaxCollisionSolverIterations);
}