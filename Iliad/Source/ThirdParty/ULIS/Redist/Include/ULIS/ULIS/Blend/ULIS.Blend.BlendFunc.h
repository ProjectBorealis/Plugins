// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.BlendFunc.h
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
#include "ULIS/Conv/ULIS.Conv.ConversionContext.h"

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
// BlendFunc Functors
template< uint32 _SH, eBlendingMode _BM >
struct BlendFunc {
    static inline
    typename TBlock< _SH >::tPixelType Compute( const typename TBlock< _SH >::tPixelType& Cb, const typename TBlock< _SH >::tPixelType& Cs )
    {
        return ttPixelType(0);
    }
};

#define ULIS_SPEC_BLENDFUNC_COMPUTE_START( iMode )                                                                                                          \
    template< uint32 _SH > struct BlendFunc< _SH, eBlendingMode::k ## iMode > {                                                                             \
    static inline typename TBlock< _SH >::tPixelType Compute( const typename TBlock< _SH >::tPixelType& Cb, const typename TBlock< _SH >::tPixelType& Cs ) {
#define ULIS_SPEC_BLENDFUNC_COMPUTE_END }};

//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Normal
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Normal )
    return Cs;
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------- Erase
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Erase )
    return Cb;
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Behind
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Behind )
    return Cb;
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Darken
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Darken )
    return  FMath::Min( Cb, Cs );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------- Multiply
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Multiply )
    return  ttDownscale( ttNextPixelType( Cb * Cs ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- ColorBurn
ULIS_SPEC_BLENDFUNC_COMPUTE_START( ColorBurn )
    return  Cs == ttPixelType( 0 ) ? Cs : ttMax - ttPixelType( FMath::Min( ttNextPixelType( ttMax ), ttNextPixelType( ttUpscale( ttMax - Cb ) / Cs ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- LinearBurn
ULIS_SPEC_BLENDFUNC_COMPUTE_START( LinearBurn )
    return  Cs + Cb < ttMax ? ttPixelType( 0 ) : Cs + Cb - ttMax;
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------ Lighten
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Lighten )
    return  FMath::Max( Cb, Cs );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Screen
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Screen )
    return  Cb + Cs - ttDownscale( Cb * Cs );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- ColorDodge
ULIS_SPEC_BLENDFUNC_COMPUTE_START( ColorDodge )
    return  Cs == ttMax ? Cs : ttPixelType( FMath::Min( ttNextPixelType( ttMax ), ttNextPixelType( ttUpscale( Cb ) / ( ttMax - Cs ) ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- LinearDodge
ULIS_SPEC_BLENDFUNC_COMPUTE_START( LinearDodge )
    return  ttPixelType( FMath::Min( ttNextPixelType( ttMax ), ttNextPixelType( Cb + Cs ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------ Overlay
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Overlay )
    return  ttNorm( Cb ) < 0.5f ? ttDownscale( 2 * Cs * Cb ) : ttMax - ttDownscale( 2 * ( ttMax - Cs ) * ( ttMax - Cb ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- SoftLight
ULIS_SPEC_BLENDFUNC_COMPUTE_START( SoftLight )
    ttPixelType q = ttDownscale( ttNextPixelType( Cb * Cb ) );
    ttNextPixelType d = 2 * Cs;
    return  q + ttDownscale( d * Cb ) - ttDownscale( d * q );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- HardLight
ULIS_SPEC_BLENDFUNC_COMPUTE_START( HardLight )
    return  BlendFunc< _SH, eBlendingMode::kOverlay >::Compute( Cs, Cb );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- VividLight
ULIS_SPEC_BLENDFUNC_COMPUTE_START( VividLight )
    return  ttNorm( Cs ) < 0.5f ? BlendFunc< _SH, eBlendingMode::kColorBurn >::Compute( Cb, 2 * Cs ) : BlendFunc< _SH, eBlendingMode::kColorDodge >::Compute( Cb, 2 * ( Cs - ( ttMax / ttPixelType( 2 ) ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- LinearLight
ULIS_SPEC_BLENDFUNC_COMPUTE_START( LinearLight )
    return  ttNorm( Cs ) < 0.5f ? BlendFunc< _SH, eBlendingMode::kLinearBurn >::Compute( Cb, 2 * Cs ) : BlendFunc< _SH, eBlendingMode::kLinearDodge >::Compute( Cb, 2 * ( Cs - ( ttMax / ttPixelType( 2 ) ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------- PinLight
ULIS_SPEC_BLENDFUNC_COMPUTE_START( PinLight )
    return  ttNorm( Cs ) < 0.5f ? BlendFunc< _SH, eBlendingMode::kDarken >::Compute( Cb, 2 * Cs ) : BlendFunc< _SH, eBlendingMode::kLighten >::Compute( Cb, 2 * ( Cs - ( ttMax / ttPixelType( 2 ) ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------ HardMix
ULIS_SPEC_BLENDFUNC_COMPUTE_START( HardMix )
    return  ttNorm( BlendFunc< _SH, eBlendingMode::kVividLight >::Compute( Cb, Cs ) ) < 0.5f ? 0 : 255;
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Difference
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Difference )
    return  ttPixelType( abs( double( Cb - Cs ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- Exclusion
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Exclusion )
    return  Cb + Cs - 2 * ttDownscale( Cb * Cs );
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- Substract
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Substract )
    return  Cb > Cs ? Cb - Cs : 0;
ULIS_SPEC_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Divide
ULIS_SPEC_BLENDFUNC_COMPUTE_START( Divide )
    return  Cs == ttPixelType( 0 ) ? ttMax : ttPixelType( FMath::Min( ttNextPixelType( ttMax ) , ttNextPixelType( ttUpscale( Cb ) / Cs ) ) );
ULIS_SPEC_BLENDFUNC_COMPUTE_END

/////////////////////////////////////////////////////
// Non Separable BlendFunc Functors
template< uint32 _SH, eBlendingMode _BM >
struct BlendFuncNS {
    static inline
    void  Compute( const typename TBlock< _SH >::tPixelProxy& Cb
                 , const typename TBlock< _SH >::tPixelProxy& Cs
                 , typename TBlock< _SH >::tPixelValue& Cr
                 , TConversionContext::TReusableConverter< _SH, TDefaultModelFormat< e_cm::kHSL >() >* iForwardConnector
                 , TConversionContext::TReusableConverter< TDefaultModelFormat< e_cm::kHSL >(), _SH >* iBackwardConnector )
    {
    }
};

#define ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( iMode )                                                             \
    template< uint32 _SH > struct BlendFuncNS< _SH, eBlendingMode::k ## iMode > {                                           \
    static inline                                                                                                           \
    void  Compute( const typename TBlock< _SH >::tPixelProxy& Cb                                                            \
                 , const typename TBlock< _SH >::tPixelProxy& Cs                                                            \
                 , typename TBlock< _SH >::tPixelValue& Cr                                                                  \
                 , TConversionContext::TReusableConverter< _SH, TDefaultModelFormat< e_cm::kHSL >() >* iForwardConnector    \
                 , TConversionContext::TReusableConverter< TDefaultModelFormat< e_cm::kHSL >(), _SH >* iBackwardConnector ) {
#define ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END }};
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Hue
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( Hue )
    FValueHSLAf Bhsl;
    FValueHSLAf Shsl;
    FValueHSLAf Rhsl;
    iForwardConnector->Convert( Cb, Bhsl );
    iForwardConnector->Convert( Cs, Shsl );
    Rhsl.SetH( Shsl.H() );
    Rhsl.SetS( Bhsl.S() );
    Rhsl.SetL( Bhsl.L() );
    iBackwardConnector->Convert( Rhsl, Cr );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Saturation
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( Saturation )
    FValueHSLAf Bhsl;
    FValueHSLAf Shsl;
    FValueHSLAf Rhsl;
    iForwardConnector->Convert( Cb, Bhsl );
    iForwardConnector->Convert( Cs, Shsl );
    Rhsl.SetH( Bhsl.H() );
    Rhsl.SetS( Shsl.S() );
    Rhsl.SetL( Bhsl.L() );
    iBackwardConnector->Convert( Rhsl, Cr );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------- Color
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( Color )
    FValueHSLAf Bhsl;
    FValueHSLAf Shsl;
    FValueHSLAf Rhsl;
    iForwardConnector->Convert( Cb, Bhsl );
    iForwardConnector->Convert( Cs, Shsl );
    Rhsl.SetH( Shsl.H() );
    Rhsl.SetS( Shsl.S() );
    Rhsl.SetL( Bhsl.L() );
    iBackwardConnector->Convert( Rhsl, Cr );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Luminosity
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( Luminosity )
    FValueHSLAf Bhsl;
    FValueHSLAf Shsl;
    FValueHSLAf Rhsl;
    iForwardConnector->Convert( Cb, Bhsl );
    iForwardConnector->Convert( Cs, Shsl );
    Rhsl.SetH( Bhsl.H() );
    Rhsl.SetS( Bhsl.S() );
    Rhsl.SetL( Shsl.L() );
    iBackwardConnector->Convert( Rhsl, Cr );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- DarkerColor
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( DarkerColor )
    FValueHSLAf Bhsl;
    FValueHSLAf Shsl;
    iForwardConnector->Convert( Cb, Bhsl );
    iForwardConnector->Convert( Cs, Shsl );
    Cr.TPixelBase< _SH >::operator=( Bhsl.L() < Shsl.L() ? Cb : Cs );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------- LighterColor
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_START( LighterColor )
    FValueHSLAf Bhsl;
    FValueHSLAf Shsl;
    iForwardConnector->Convert( Cb, Bhsl );
    iForwardConnector->Convert( Cs, Shsl );
    Cr.TPixelBase< _SH >::operator=( Bhsl.L() > Shsl.L() ? Cb : Cs );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_COMPUTE_END

/////////////////////////////////////////////////////
// Undefines
#undef tSpec
#undef ttPixelType
#undef ttNextPixelType
#undef ttMax
#undef ttDownscale
#undef ttUpscale
} // namespace ULIS

