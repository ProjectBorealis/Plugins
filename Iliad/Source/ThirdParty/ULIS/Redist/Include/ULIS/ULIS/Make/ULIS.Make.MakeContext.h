// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Make.MakeContext.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Make/ULIS.Make.BlockCopier.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Thread/ULIS.Thread.ParallelFor.h"
#include <atomic>

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TMakeContext
template< uint32 _SH >
class TMakeContext
{
public:
    // Copy
    static void CopyBlockInto( const TBlock< _SH >* iSrc, TBlock< _SH >* iDst, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        CopyBlockRectInto( iSrc, iDst, FRect( 0, 0, iSrc->Width(), iSrc->Height() ), iPerformanceOptions);
    }

    static void CopyBlockRectInto( const TBlock< _SH >* iSrc, TBlock< _SH >* iDst, const FRect& iRect, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        FRect src_bb = FRect( 0, 0, iSrc->Width(), iSrc->Height() );
        FRect rect_bb = FRect( iRect.x, iRect.y, FMath::Min( iDst->Width(), iRect.w ), FMath::Min( iDst->Height(), iRect.h ) );
        FRect inter_bb  = src_bb & rect_bb;
        if( inter_bb.Area() <= 0 ) return;
        FPoint shift( -iRect.x, -iRect.y );

        TBlockCopier< _SH >::Run( iSrc, iDst, inter_bb, shift, iPerformanceOptions);
    }

    static void CopyBlockRectInto( const TBlock< _SH >* iSrc, TBlock< _SH >* iDst, const FRect& iSrcRect, const FPoint& iDstPos, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        FRect src_bb = FRect( 0, 0, iSrc->Width(), iSrc->Height() );
        FRect rect_bb = FRect( iSrcRect.x, iSrcRect.y, FMath::Min( iDst->Width(), iSrcRect.w ), FMath::Min( iDst->Height(), iSrcRect.h ) );
        FRect inter_bb  = src_bb & rect_bb;
        if( inter_bb.Area() <= 0 ) return;
        FPoint shift( -iSrcRect.x + iDstPos.x, -iSrcRect.y + iDstPos.y );

        TBlockCopier< _SH >::Run( iSrc, iDst, inter_bb, shift, iPerformanceOptions);
    }

    static  FRect  GetTrimmedTransparencyRect( const TBlock< _SH >* iSrc, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    {
        using tPixelType = typename TBlock< _SH >::tPixelType;
        using tPixelValue = typename TBlock< _SH >::tPixelValue;
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using tPixelBase = TPixelBase< _SH >;
        using info = TBlockInfo< _SH >;

        if( !tPixelBase::HasAlpha() )
            return  FRect( 0, 0, iSrc->Width(), iSrc->Height() );

        std::atomic_int left( INT_MAX );
        std::atomic_int top( INT_MAX );
        std::atomic_int right( 0 );
        std::atomic_int bot( 0 );
        const int minx = 0;
        const int miny = 0;
        const int maxx = iSrc->Width();
        const int maxy = iSrc->Height();
        ParallelFor( (int32)maxy
                   , [&]( int32 iLine ) {
                        for( int i = minx; i < maxx; ++i ) {
                            if( iSrc->PixelProxy( i, iLine ).GetAlpha() > tPixelType( 0 ) ) {
                                if( iLine < top.load() )    top.store( iLine );
                                if( i < left.load() )       left.store( i );
                                if( iLine > bot.load() )    bot.store( iLine );
                                if( i > right.load() )      right.store( i );
                            }
                        }
                   }
                   , iPerformanceOptions );
        return  FRect( left, top, ( right - left ) + 1, ( bot - top ) + 1 );
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS