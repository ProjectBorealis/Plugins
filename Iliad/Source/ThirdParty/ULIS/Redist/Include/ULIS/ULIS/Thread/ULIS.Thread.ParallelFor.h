// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Thread.ParallelFor.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"
#include "ULIS/Thread/ULIS.Thread.Pool.h"
#include <functional>

namespace ULIS {
/////////////////////////////////////////////////////
// ParallelFor
inline  void  ParallelFor_Imp( FThreadPool& iPool, int32 iNum, std::function< void( int32 ) >& iFun, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
{
    if( iPerformanceOptions.desired_workers > 1 && iPool.GetNumWorkers() > 1 )
    {
        for( int i = 0; i < iNum; ++i )
            iPool.ScheduleJob( iFun, i );
        iPool.WaitForCompletion();
    }
    else
    {
        for( int i = 0; i < iNum; ++i )
        {
            iFun( i );
        }
    }
}


inline  void  ParallelForPool( FThreadPool& iPool, int32 iNum, std::function< void( int32 ) > iFun, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
{
    ParallelFor_Imp( iPool, iNum, iFun, iPerformanceOptions );
}


inline  void  ParallelFor( int32 iNum, std::function< void( int32 ) > iFun, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
{
    ParallelFor_Imp( FGlobalThreadPool::Get(), iNum, iFun, iPerformanceOptions );
}


} // namespace ULIS

