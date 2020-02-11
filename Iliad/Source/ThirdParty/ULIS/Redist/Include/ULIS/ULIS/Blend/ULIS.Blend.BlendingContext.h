// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.BlendingContext.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Base/ULIS.Base.WeakPRNG.h"
#include "ULIS/Blend/ULIS.Blend.BlendingModes.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Blend/ULIS.Blend.BlockBlender.h"

// Load optimizations
#include "ULIS/Blend/Optimization/ULIS.Blend.Optimization.RGBA8.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TBlendingContext
template< uint32 _SH >
class TBlendingContext
{
private:
    template< eBlendingMode _BM >
    static void Blend_Imp( TBlock< _SH >*                      iBlockTop
                         , TBlock< _SH >*                      iBlockBack
                         , eAlphaMode                          iAlphaMode
                         , typename TBlock< _SH >::tPixelType  iOpacity
                         , const FRect&                        iROI
                         , const FPoint&                       iShift
                         , const FPerformanceOptions&          iPerformanceOptions )
    {
        #define ULIS_SWITCH_OP( iAM )  TBlockBlender< _SH, _BM, iAM >::Run( iBlockTop, iBlockBack, iOpacity, iROI, iShift, iPerformanceOptions )
        ULIS_FOR_ALL_ALPHA_MODES_DO( iAlphaMode, ULIS_SWITCH_OP )
        #undef ULIS_SWITCH_OP
    }

public:
    static void Blend( TBlock< _SH >*               iBlockTop
                     , TBlock< _SH >*               iBlockBack
                     , int                          iX                  = 0
                     , int                          iY                  = 0
                     , eBlendingMode                iBlendingMode       = eBlendingMode::kNormal
                     , eAlphaMode                   iAlphaMode          = eAlphaMode::kNormal
                     , float                        iOpacity            = 1.f
                     , const FPerformanceOptions&   iPerformanceOptions = FPerformanceOptions()
                     , bool                         callInvalidCB       = true )
    {
        assert( iOpacity > 0.f && iOpacity <= 1.f );
        FRect back_bb   = { 0, 0,     iBlockBack->Width(),    iBlockBack->Height()    };
        FRect top_bb    =  { iX, iY,   iBlockTop->Width(),     iBlockTop->Height()     };
        FRect inter_bb  = back_bb & top_bb;
        if( inter_bb.Area() <= 0 ) return;
        FPoint shift( -iX, -iY );

        #define ULIS_SWITCH_OP( iBM )  TBlendingContext< _SH >::Blend_Imp< iBM >( iBlockTop, iBlockBack, iAlphaMode, ConvType< float, typename TBlock< _SH >::tPixelType >( iOpacity ), inter_bb, shift, iPerformanceOptions )
        ULIS_FOR_ALL_BLENDING_MODES_DO( iBlendingMode, ULIS_SWITCH_OP )
        #undef ULIS_SWITCH_OP

        if( callInvalidCB )
            iBlockBack->Invalidate( inter_bb );
    }

    static void Blend( TBlock< _SH >*               iBlockTop
                     , TBlock< _SH >*               iBlockBack
                     , const FRect&                 iArea
                     , eBlendingMode                iBlendingMode       = eBlendingMode::kNormal
                     , eAlphaMode                   iAlphaMode          = eAlphaMode::kNormal
                     , float                        iOpacity            = 1.f
                     , const FPerformanceOptions&   iPerformanceOptions = FPerformanceOptions()
                     , bool                         callInvalidCB       = true )
    {
        assert( iOpacity > 0.f && iOpacity <= 1.f );
        FRect back_bb   = FRect( 0, 0, iBlockBack->Width(), iBlockBack->Height() );
        FRect top_bb    = FRect( iArea.x, iArea.y, iArea.w, iArea.h );
        FRect inter_bb  = back_bb & top_bb;
        bool intersects = inter_bb.Area() > 0;
        if( !intersects ) return;
        FPoint shift( 0, 0 );

        #define ULIS_SWITCH_OP( iBM )  TBlendingContext< _SH >::Blend_Imp< iBM >( iBlockTop, iBlockBack, iAlphaMode, ConvType< float, typename TBlock< _SH >::tPixelType >( iOpacity ), inter_bb, shift, iPerformanceOptions )
        ULIS_FOR_ALL_BLENDING_MODES_DO( iBlendingMode, ULIS_SWITCH_OP )
        #undef ULIS_SWITCH_OP

        if( callInvalidCB )
            iBlockBack->Invalidate( inter_bb );
    }
};


/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS