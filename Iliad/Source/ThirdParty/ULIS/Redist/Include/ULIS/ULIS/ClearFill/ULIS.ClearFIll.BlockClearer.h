// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.ClearFIll.BlockClearer.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TBlockClearer_Default_ScanLine_Memcpy
template< uint32 _SH >
class TBlockClearer_Default_ScanLine_Memcpy
{
public:
    static void ProcessScanLine( TBlock< _SH >*                     iBlock
                               , const int                          iLine
                               , const int                          iX1
                               , const int                          iX2 )
    {
        int         dep     = iBlock->Depth();
        uint8_t*    dst     = iBlock->PixelPtr( iX1, iLine );

        for( int i = iX1; i < iX2; ++i )
        {
            memset( dst, 0, dep );
            dst = dst + dep;
        }
    }

    static inline void Run( TBlock< _SH >*                      iBlock
                          , const FRect&                        iROI )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iBlock, y, x1, x2 );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockClearer_Default_ScanLine
template< uint32 _SH >
class TBlockClearer_Default_ScanLine
{
public:
    static void ProcessScanLine( TBlock< _SH >*                     iBlock
                               , const int                          iLine
                               , const int                          iX1
                               , const int                          iX2 )
    {
        int dep = iBlock->Depth();
        for( int x = iX1; x < iX2; ++x )
            memset( iBlock->PixelPtr( x, iLine ), 0, dep );
    }

    static inline void Run( TBlock< _SH >*                      iBlock
                          , const FRect&                        iROI )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iBlock, y, x1, x2 );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockClearer_Default_MonoThread
template< uint32 _SH >
class TBlockClearer_Default_MonoThread
{
public:
    static inline void Run( TBlock< _SH >*                      iBlock
                          , const FRect&                        iROI )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        int dep     = iBlock->Depth();
        for( int y = y1; y < y2; ++y )
            for( int x = x1; x < x2; ++x )
                memset( iBlock->PixelPtr( x, y ), 0, dep );
    }
};

/////////////////////////////////////////////////////
// TBlockClearer_Default
template< uint32 _SH >
class TBlockClearer_Default
{
public:
    static inline void Run( TBlock< _SH >*                      iBlock
                          , const FRect&                        iROI
                          , const FPerformanceOptions&                   iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            TBlockClearer_Default_ScanLine< _SH >::Run( iBlock, iROI );
        }
        else
        {
            TBlockClearer_Default_MonoThread< _SH >::Run( iBlock, iROI );
        }
    }
};


/////////////////////////////////////////////////////
// TBlockClearer_Imp
template< uint32 _SH >
class TBlockClearer_Imp
{
public:
    static inline void Run( TBlock< _SH >*                      iBlock
                          , const FRect&                        iROI
                          , const FPerformanceOptions&                   iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockClearer_Default< _SH >::Run( iBlock, iROI, iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// TBlockClearer
template< uint32 _SH >
class TBlockClearer
{
public:
    static inline void Run( TBlock< _SH >*                      iBlock
                          , const FRect&                        iROI
                          , const FPerformanceOptions&                   iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockClearer_Imp< _SH >::Run( iBlock, iROI, iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS