// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Make.BlockCopier.h
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
// TBlockCopier_Default_ScanLine_Memcpy
template< uint32 _SH >
class TBlockCopier_Default_ScanLine_Memcpy
{
public:
    static void ProcessScanLine( const TBlock< _SH >*               iSrc
                               , TBlock< _SH >*                     iDst
                               , const int                          iLine
                               , const int                          iX1
                               , const int                          iX2
                               , const FPoint&                      iShift )
    {
        void*       src     = iSrc->PixelPtr( iX1, iLine );
        void*       dst     = iDst->PixelPtr( iX1 + iShift.x, iLine + iShift.y );
        int         dep     = iSrc->Depth();

        for( int i = iX1; i < iX2; ++i )
        {
            memcpy( dst, src, dep );
            src = static_cast<uint8*>(src) + dep;
            dst = static_cast<uint8*>(dst) + dep;
        }
    }

    static inline void Run( const TBlock< _SH >*                iSrc
                          , TBlock< _SH >*                      iDst
                          , const FRect&                        iROI
                          , const FPoint&                       iShift )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iSrc, iDst, y, x1, x2, iShift );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockCopier_Default_ScanLine
template< uint32 _SH >
class TBlockCopier_Default_ScanLine
{
public:
    static void ProcessScanLine( const TBlock< _SH >*               iSrc
                               , TBlock< _SH >*                     iDst
                               , const int                          iLine
                               , const int                          iX1
                               , const int                          iX2
                               , const FPoint&                      iShift )
    {
        for( int x = iX1; x < iX2; ++x )
            iDst->SetPixelProxy( x + iShift.x, iLine + iShift.y, iSrc->PixelProxy( x, iLine ) );
    }

    static inline void Run( const TBlock< _SH >*                iSrc
                          , TBlock< _SH >*                      iDst
                          , const FRect&                        iROI
                          , const FPoint&                       iShift )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iSrc, iDst, y, x1, x2, iShift );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockCopier_Default_MonoThread
template< uint32 _SH >
class TBlockCopier_Default_MonoThread
{
public:
    static inline void Run( const TBlock< _SH >*                iSrc
                          , TBlock< _SH >*                      iDst
                          , const FRect&                        iROI
                          , const FPoint&                       iShift )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        for( int y = y1; y < y2; ++y )
            for( int x = x1; x < x2; ++x )
                iDst->SetPixelProxy( x + iShift.x, y + iShift.y, iSrc->PixelProxy( x, y ) );
    }
};

/////////////////////////////////////////////////////
// TBlockCopier_Default
template< uint32 _SH >
class TBlockCopier_Default
{
public:
    static inline void Run( const TBlock< _SH >*                iSrc
                          , TBlock< _SH >*                      iDst
                          , const FRect&                        iROI
                          , const FPoint&                       iShift
                          , const FPerformanceOptions&                   iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            TBlockCopier_Default_ScanLine< _SH >::Run( iSrc, iDst, iROI, iShift );
        }
        else
        {
            TBlockCopier_Default_MonoThread< _SH >::Run( iSrc, iDst, iROI, iShift );
        }
    }
};


/////////////////////////////////////////////////////
// TBlockCopier_Imp
template< uint32 _SH >
class TBlockCopier_Imp
{
public:
    static inline void Run( const TBlock< _SH >*                iSrc
                          , TBlock< _SH >*                      iDst
                          , const FRect&                        iROI
                          , const FPoint&                       iShift
                          , const FPerformanceOptions&                   iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockCopier_Default< _SH >::Run( iSrc, iDst, iROI, iShift, iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// TBlockCopier
template< uint32 _SH >
class TBlockCopier
{
public:
    static inline void Run( const TBlock< _SH >*                iSrc
                          , TBlock< _SH >*                      iDst
                          , const FRect&                        iROI
                          , const FPoint&                       iShift
                          , const FPerformanceOptions&                   iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockCopier_Imp< _SH >::Run( iSrc, iDst, iROI, iShift, iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS
