// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.BlendAlpha.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Blend/ULIS.Blend.BlendingModes.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Maths/ULIS.Maths.Utility.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >
#define ttPixelType typename TBlock< _SH >::tPixelType
#define ttNextPixelType typename TBlock< _SH >::tNextPixelType
#define ttMax TBlock< _SH >::StaticFastMax()
#define ttDownscale ConvType< ttNextPixelType, ttPixelType >
#define ttUpscale ConvType< ttPixelType, ttNextPixelType >
#define ttNorm ConvType< ttPixelType, float >

/////////////////////////////////////////////////////
// BlendAlpha Functors
template< uint32 _SH, eAlphaMode _AM >
struct BlendAlpha {
    static inline
    typename TBlock< _SH >::tPixelType Compute( const typename TBlock< _SH >::tPixelType& Cb, const typename TBlock< _SH >::tPixelType& Cs )
    {
        return ttPixelType(0);
    }
};

#define ULIS_SPEC_BLENDALPHA_COMPUTE_START( iMode )                                                                                                             \
    template< uint32 _SH > struct BlendAlpha< _SH, eAlphaMode::k ## iMode > {                                                                                   \
    static inline typename TBlock< _SH >::tPixelType Compute( const typename TBlock< _SH >::tPixelType& Cb, const typename TBlock< _SH >::tPixelType& Cs ) {
#define ULIS_SPEC_BLENDALPHA_COMPUTE_END }};

//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Normal
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Normal )
    return  ( Cb + Cs ) - ConvType< typename TBlock< _SH >::tNextPixelType, typename TBlock< _SH >::tPixelType >( (typename TBlock< _SH >::tNextPixelType)( Cb * Cs ) );
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------- Erase
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Erase )
    return  ConvType< typename TBlock< _SH >::tNextPixelType, typename TBlock< _SH >::tPixelType >( (typename TBlock< _SH >::tNextPixelType)( ( ttMax - Cs ) * Cb ) );
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Top
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Top )
    return  Cs;
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------- Back
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Back )
    return  Cb;
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------- Sub
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Sub )
    return  Cb > Cs ? Cb - Cs : 0;
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Add
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Add )
    return  ttPixelType( ttMax - Cb ) > Cs ? Cb + Cs : ttMax;
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Mul
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Mul )
    return  ttDownscale( ttNextPixelType( Cb * Cs ) );
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Min
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Min )
    return  FMath::Min( Cb, Cs );
ULIS_SPEC_BLENDALPHA_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Max
ULIS_SPEC_BLENDALPHA_COMPUTE_START( Max )
    return  FMath::Max( Cb, Cs );
ULIS_SPEC_BLENDALPHA_COMPUTE_END

/////////////////////////////////////////////////////
// Undefines
#undef tSpec
#undef ttPixelType
#undef ttNextPixelType
#undef ttMax
#undef ttDownscale
#undef ttUpscale

} // namespace ULIS