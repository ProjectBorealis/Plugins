// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Thread.Queue.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>


namespace ULIS {
/////////////////////////////////////////////////////
// TSharedQueue
template <typename T>
class TSharedQueue
{
public:
    // Construction / Destruction
    TSharedQueue (){}
    ~TSharedQueue(){}

public:
    // Public API
    T&      Front();
    void    Pop_front();
    void    Push_back( const T& item );
    int     Size();
    bool    Empty();

private:
    // Private Data
    std::deque< T >             stdqueue;
    std::condition_variable     cond;
    std::mutex                  mutex;
}; 


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Public API
template <typename T>
T& TSharedQueue<T>::Front()
{
    std::unique_lock<std::mutex> mlock(mutex);
    while (stdqueue.empty())
    {
        cond.wait(mlock);
    }
    return stdqueue.front();
}


template <typename T>
void TSharedQueue<T>::Pop_front()
{
    std::unique_lock<std::mutex> mlock(mutex);
    while (stdqueue.empty())
    {
        cond.wait(mlock);
    }
    stdqueue.pop_front();
}


template <typename T>
void TSharedQueue<T>::Push_back(const T& item)
{
    std::unique_lock<std::mutex> mlock(mutex);
    stdqueue.push_back(item);
    mlock.unlock();
    cond.notify_one();
}


template <typename T>
int TSharedQueue<T>::Size()
{
    std::unique_lock<std::mutex> mlock(mutex);
    int size = stdqueue.size();
    mlock.unlock();
    return size;
}


template <typename T>
bool TSharedQueue<T>::Empty()
{
    std::unique_lock< std::mutex > mlock( mutex );
    bool empty = stdqueue.empty();
    mlock.unlock();
    return empty;
}


} // namespace ULIS

