// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.ClearFIll.ClearFillContext.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/ClearFill/ULIS.ClearFIll.BlockClearer.h"
#include "ULIS/ClearFill/ULIS.ClearFIll.BlockFiller.h"
#include "ULIS/Color/ULIS.Color.CColor.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Thread/ULIS.Thread.ParallelFor.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TClearFillContext
template< uint32 _SH >
class TClearFillContext
{
public:
    // Fill

    static void Fill( TBlock< _SH >* iBlock, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool callInvalidCB = true )
    {
        FillRect( iBlock, iColor, FRect( 0, 0, iBlock->Width(), iBlock->Height() ), iPerformanceOptions, callInvalidCB );
    }

    static void FillPreserveAlpha( TBlock< _SH >* iBlock, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool callInvalidCB = true )
    {
        using tPixelType = typename TBlock< _SH >::tPixelType;
        using tPixelValue = typename TBlock< _SH >::tPixelValue;
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using tPixelBase = TPixelBase< _SH >;
        using info = TBlockInfo< _SH >;

        tPixelValue val = iBlock->PixelValueForColor( iColor );

        ::ULIS::ParallelFor( iBlock->Height()
                           , [&]( int iLine ) {
                                for( int i = 0; i < iBlock->Width(); ++i )
                                {
                                    tPixelProxy prox = iBlock->PixelProxy( i, iLine );
                                    tPixelType alpha = prox.GetAlpha();
                                    prox = val;
                                    prox.SetAlpha( alpha );
                                }
                            } );
    }

    static void FillRect( TBlock< _SH >* iBlock, const CColor& iColor, const FRect& iArea, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool callInvalidCB = true )
    {
        FRect block_bb  = FRect( 0, 0, iBlock->Width(), iBlock->Height() );
        FRect rect_bb   = FRect( iArea.x, iArea.y, iArea.w, iArea.h );
        FRect inter_bb  = block_bb & rect_bb;
        bool intersects = inter_bb.Area() > 0;
        if( !intersects ) return;

        TBlockFiller< _SH >::Run( iBlock, TBlock<_SH>::PixelValueForColor( iColor ), inter_bb, iPerformanceOptions);

        if( !callInvalidCB )
            return;

        iBlock->Invalidate( inter_bb );
    }

public:
    // Clear
    static void Clear( TBlock< _SH >* iBlock, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool callInvalidCB = true )
    {
        ClearRect( iBlock, FRect( 0, 0, iBlock->Width(), iBlock->Height() ), iPerformanceOptions, callInvalidCB );
    }

    static void ClearRect( TBlock< _SH >* iBlock, const FRect& iArea, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool callInvalidCB = true )
    {
        FRect block_bb  = FRect( 0, 0, iBlock->Width(), iBlock->Height() );
        FRect rect_bb   = FRect( iArea.x, iArea.y, iArea.w, iArea.h );
        FRect inter_bb  = block_bb & rect_bb;
        bool intersects = inter_bb.Area() > 0;
        if( !intersects ) return;

        TBlockClearer< _SH >::Run( iBlock, inter_bb, iPerformanceOptions);

        if( !callInvalidCB )
            return;

        iBlock->Invalidate( inter_bb );
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS