// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkAsyncTaskFunctor.h
#pragma once


/**
 * ASyncTask to invoke a lambda function
 */
class FSubstanceLiveLinkAutoDeleteAsyncTask : public FNonAbandonableTask
{
public:
	/** FAutoDeleteAsyncTask means this task will automatically destroy itself on completion */
	friend class FAutoDeleteAsyncTask<FSubstanceLiveLinkAutoDeleteAsyncTask>;

	/** Typedef for the callback type */
	typedef TFunction<void ()> SubstanceLiveLinkAsyncTaskFunction;

public:
	/**
	 * Constructor
	 * @param _Callback the function to invoke on the worker thread
	 */
	FSubstanceLiveLinkAutoDeleteAsyncTask(SubstanceLiveLinkAsyncTaskFunction _Callback)
		: Callback(_Callback)
	{
	}

	/**
	 * Called by Thread Pool to initiate work
	 */
	void DoWork()
	{
		Callback();
	}

	/**
	 * StatId required for analysis
	 * @returns the StatId object declaration
	 */
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSubstanceLiveLinkAutoDeleteAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

private:
	/** Callback function to invoke */
	SubstanceLiveLinkAsyncTaskFunction Callback;
};
