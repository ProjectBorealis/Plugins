// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Thread.Pool.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include <iostream>
#include <deque>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <random>
#include <atomic>

namespace ULIS {
/////////////////////////////////////////////////////
// FThreadPool
class FThreadPool
{
public:
    // Construction / Destruction
    FThreadPool( unsigned int iCount = std::thread::hardware_concurrency() );
    ~FThreadPool();

public:
    // Public API
    template<class F>
    void ScheduleJob( F&& f )
    {
        std::unique_lock< std::mutex > lock( queue_mutex );
        tasks.push_back( std::forward< F >( f ) );
        cv_task.notify_one();
    }

    template<class F, typename ... Args >
    void ScheduleJob( F&& f, Args&& ... args )
    {
        std::unique_lock< std::mutex > lock( queue_mutex );
        tasks.push_back( std::bind( std::forward< F >( f ), args ... ) );
        cv_task.notify_one();
    }


    void                                    WaitForCompletion();
    unsigned int                            GetProcessed() const { return processed; }
    unsigned int                            GetNumWorkers() const { return workers.size(); }
    void                                    SetNumWorkers( unsigned int );
    unsigned int                            GetMaxWorkers() const { return  std::thread::hardware_concurrency(); }

private:
    // Private API
    void ThreadProcess();

private:
    // Private Data
    unsigned int                        busy;
    bool                                stop;
    std::atomic_uint                    processed;
    std::vector< std::thread >          workers;
    std::deque< std::function<void()> > tasks;
    std::mutex                          queue_mutex;
    std::condition_variable             cv_task;
    std::condition_variable             cv_finished;
};


} // namespace ULIS

