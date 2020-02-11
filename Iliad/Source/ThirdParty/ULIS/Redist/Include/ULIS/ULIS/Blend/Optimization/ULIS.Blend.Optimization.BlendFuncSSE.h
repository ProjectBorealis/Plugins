// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.Optimization.BlendFuncSSE.h
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
#include <immintrin.h>

namespace ULIS {
/////////////////////////////////////////////////////
// BlendFuncSSE Functors
template< eBlendingMode _BM >
struct BlendFuncSSE {
    static inline
    __m128 Compute( const __m128& Cb, const __m128& Cs )
    {
        return  _mm_setzero_ps();
    }
};

#define ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( iMode )                          \
    template<> struct BlendFuncSSE< eBlendingMode::k ## iMode > {               \
    static inline __m128 Compute( const __m128& Cb, const __m128& Cs ) {
#define ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END }};

// SPEC BLEND
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Normal
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Normal )
    return Cs;
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------- Erase
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Erase )
    return Cb;
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Behind
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Behind )
    return Cb;
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Darken
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Darken )
    return  _mm_min_ps( Cb, Cs );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------- Multiply
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Multiply )
    return  _mm_div_ps( _mm_mul_ps( Cb, Cs ), _mm_set_ps1( 255.f ) );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- ColorBurn
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( ColorBurn )
    __m128 vcmp     = _mm_cmpeq_ps( Cs, _mm_setzero_ps() );
    __m128 max255f  = _mm_set_ps1( 255.f );
    __m128 computed = _mm_sub_ps( max255f, _mm_min_ps( max255f, _mm_div_ps( _mm_mul_ps( max255f, _mm_sub_ps( max255f, Cb ) ), Cs ) ) );
    return  _mm_blendv_ps( computed, Cs, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- LinearBurn
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( LinearBurn )
    __m128 add      = _mm_add_ps( Cs, Cb );
    __m128 max255f  = _mm_set_ps1( 255.f );
    __m128 vcmp     = _mm_cmplt_ps( add, max255f );
    __m128 computed = _mm_sub_ps( add, max255f );
    return  _mm_blendv_ps( computed, _mm_setzero_ps(), vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END

//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------ Lighten
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Lighten )
    return  _mm_max_ps( Cb, Cs );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Screen
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Screen )
    return  _mm_sub_ps( _mm_add_ps( Cb, Cs ), _mm_div_ps( _mm_mul_ps( Cb, Cs ), _mm_set_ps1( 255.f ) ) );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- ColorDodge
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( ColorDodge )
    __m128 max255f  = _mm_set_ps1( 255.f );
    __m128 vcmp     = _mm_cmpeq_ps( Cs, max255f );
    __m128 computed = _mm_min_ps( max255f, _mm_div_ps( _mm_mul_ps( max255f, Cb ), _mm_sub_ps( max255f, Cs ) ) );
    return  _mm_blendv_ps( computed, Cs, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- LinearDodge
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( LinearDodge )
    return  _mm_min_ps( _mm_set_ps1( 255.f ), _mm_add_ps( Cb, Cs ) );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------ Overlay
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Overlay )
    __m128 max255f      = _mm_set_ps1( 255.f );
    __m128 twof         = _mm_set_ps1( 2.f );
    __m128 vcmp         = _mm_cmplt_ps( Cb, _mm_set_ps1( 128.f ) );
    __m128 computedA    = _mm_div_ps( _mm_mul_ps( twof, _mm_mul_ps( Cs, Cb ) ), max255f );
    __m128 computedB    = _mm_sub_ps( max255f, _mm_div_ps( _mm_mul_ps( twof, _mm_mul_ps( _mm_sub_ps( max255f, Cs ), _mm_sub_ps( max255f, Cb ) ) ), max255f ) );
    return  _mm_blendv_ps( computedB, computedA, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- SoftLight
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( SoftLight )
    __m128 q = _mm_div_ps( _mm_mul_ps( Cb, Cs ), _mm_set_ps1( 255.f ) );
    __m128 d = _mm_mul_ps( _mm_set_ps1( 2.f ), Cs );
    return  _mm_sub_ps( _mm_add_ps( q, _mm_div_ps( _mm_mul_ps( d, Cb ), _mm_set_ps1( 255.f ) ) ), _mm_div_ps( _mm_mul_ps( d, q ), _mm_set_ps1( 255.f ) ) );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- HardLight
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( HardLight )
    return  BlendFuncSSE< eBlendingMode::kOverlay >::Compute( Cs, Cb );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- VividLight
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( VividLight )
    __m128 twof         = _mm_set_ps1( 2.f );
    __m128 vcmp         = _mm_cmplt_ps( Cs, _mm_set_ps1( 128.f ) );
    __m128 computedA    = BlendFuncSSE< eBlendingMode::kColorBurn >::Compute( Cb, _mm_mul_ps( twof, Cs ) );
    __m128 computedB    = BlendFuncSSE< eBlendingMode::kColorDodge >::Compute( Cb, _mm_mul_ps( twof, _mm_sub_ps( Cs, _mm_set_ps1( 127.f ) ) ) );
    return  _mm_blendv_ps( computedB, computedA, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- LinearLight
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( LinearLight )
    __m128 twof         = _mm_set_ps1( 2.f );
    __m128 vcmp         = _mm_cmplt_ps( Cs, _mm_set_ps1( 128.f ) );
    __m128 computedA    = BlendFuncSSE< eBlendingMode::kLinearBurn >::Compute( Cb, _mm_mul_ps( twof, Cs ) );
    __m128 computedB    = BlendFuncSSE< eBlendingMode::kLinearDodge >::Compute( Cb, _mm_mul_ps( twof, _mm_sub_ps( Cs, _mm_set_ps1( 127.f ) ) ) );
    return  _mm_blendv_ps( computedB, computedA, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------- PinLight
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( PinLight )
    __m128 twof         = _mm_set_ps1( 2.f );
    __m128 vcmp         = _mm_cmplt_ps( Cs, _mm_set_ps1( 128.f ) );
    __m128 computedA    = BlendFuncSSE< eBlendingMode::kDarken >::Compute( Cb, _mm_mul_ps( twof, Cs ) );
    __m128 computedB    = BlendFuncSSE< eBlendingMode::kLighten >::Compute( Cb, _mm_mul_ps( twof, _mm_sub_ps( Cs, _mm_set_ps1( 127.f ) ) ) );
    return  _mm_blendv_ps( computedB, computedA, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------ HardMix
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( HardMix )
    __m128 computed = BlendFuncSSE< eBlendingMode::kVividLight >::Compute( Cb, Cs );
    __m128 vcmp     = _mm_cmplt_ps( computed, _mm_set_ps1( 128.f ) );
    return  _mm_blendv_ps( _mm_set_ps1( 255.f ), _mm_setzero_ps(), vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Difference
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Difference )
    __m128 diff = _mm_sub_ps( Cb, Cs );
    return  _mm_max_ps( _mm_sub_ps( _mm_setzero_ps(), diff ), diff );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- Exclusion
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Exclusion )
    return  _mm_add_ps( Cb, _mm_sub_ps( Cs, _mm_mul_ps( _mm_set_ps1( 2.f ), _mm_div_ps( _mm_mul_ps( Cb, Cs ), _mm_set_ps1( 255.f ) ) ) ) );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- Substract
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Substract )
    __m128 vcmp = _mm_cmpgt_ps( Cb, Cs );
    __m128 diff = _mm_sub_ps( Cb, Cs );
    return  _mm_blendv_ps( _mm_setzero_ps(), diff, vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Divide
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_START( Divide )
    __m128 vcmp         = _mm_cmpeq_ps( Cs, _mm_setzero_ps() );
    __m128 max255f      = _mm_set_ps1( 255.f );
    __m128 computed     = _mm_min_ps( max255f, _mm_div_ps( _mm_mul_ps( max255f, Cb ), Cs ) );
    return  _mm_blendv_ps( computed, _mm_setzero_ps(), vcmp );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END

/////////////////////////////////////////////////////
// Non Separable BlendFunc Functors
template< uint32 _SH, eBlendingMode _BM >
struct BlendFuncNS_RGBA8 {
    static inline
    void  Compute( const typename TBlock< _SH >::tPixelProxy& Cb
                 , const typename TBlock< _SH >::tPixelProxy& Cs
                 , typename TBlock< _SH >::tPixelValue& Cr )
    {
    }
};

#define ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( iMode )                                                       \
    template< uint32 _SH > struct BlendFuncNS_RGBA8< _SH, eBlendingMode::k ## iMode > {                                     \
    static inline                                                                                                           \
    void  Compute( const typename TBlock< _SH >::tPixelProxy& Cb                                                            \
                 , const typename TBlock< _SH >::tPixelProxy& Cs                                                            \
                 , typename TBlock< _SH >::tPixelValue& Cr ) {
#define ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END }};
/////////////////////////////////////////////////////
// HSL blending modes functions
template< uint32 _SH >
static inline uint8
Lum( const typename TBlock< _SH >::tPixelValue& C )
{
    return  uint8( 0.3f * C.R() + 0.59f * C.G() + 0.11f * C.B() );
}


static inline float
Lum( float R, float G, float B )
{
    return  float( 0.3f * R + 0.59f * G + 0.11f * B );
}

template< uint32 _SH >
static typename TBlock< _SH >::tPixelValue
SetLum( const typename TBlock< _SH >::tPixelValue& C, uint8 L )
{
    float d = L - Lum< _SH >( C );
    float R = C.R() + d;
    float G = C.G() + d;
    float B = C.B() + d;
    float backR = R;
    float backG = G;
    float backB = B;
    float l = Lum( R, G, B );
    float n = FMath::Min3( R, G, B );
    float x = FMath::Max3( R, G, B );
    if( n < 0.0f )
    {
        float ln = l - n;
        R = l + ( ( ( R - l ) * l ) / ( ln ) );
        G = l + ( ( ( G - l ) * l ) / ( ln ) );
        B = l + ( ( ( B - l ) * l ) / ( ln ) );
    }

    if( x > 255.0f )
    {
        float xl = x - l;
        float fl255 = 255.f - l;
        R = l + ( ( ( R - l ) * ( fl255 ) ) / ( xl ) );
        G = l + ( ( ( G - l ) * ( fl255 ) ) / ( xl ) );
        B = l + ( ( ( B - l ) * ( fl255 ) ) / ( xl ) );
    }

    typename TBlock< _SH >::tPixelValue ret;
    ret.SetR( (uint8)R );
    ret.SetG( (uint8)G );
    ret.SetB( (uint8)B );
    return  ret;
}

template< uint32 _SH >
static inline uint8
Sat( const typename TBlock< _SH >::tPixelValue& C )
{
    return  FMath::Max3( C.R(), C.G(), C.B() ) - FMath::Min3( C.R(), C.G(), C.B() );
}

template< uint32 _SH >
static typename TBlock< _SH >::tPixelValue
SetSat( const typename TBlock< _SH >::tPixelValue& C, uint8 s )
{
    uint8 maxIndex = C.R() > C.G() ? ( C.R() > C.B() ? 0 : 2 ) : ( C.G() > C.B() ? 1 : 2 );
    uint8 minIndex = C.R() < C.G() ? ( C.R() < C.B() ? 0 : 2 ) : ( C.G() < C.B() ? 1 : 2 );
    uint8 midIndex = 3 - maxIndex - minIndex;
    float Cmax = C.GetComponent( maxIndex );
    float Cmin = C.GetComponent( minIndex );
    float Cmid = C.GetComponent( midIndex );
    if( Cmax > Cmin )
    {
        Cmid = ( ( ( Cmid - Cmin ) * s ) / ( Cmax - Cmin ) );
        Cmax = s;
    }
    else
    {
        Cmid = Cmax = 0.f;
    }
    Cmin = 0.f;

    typename TBlock< _SH >::tPixelValue ret;
    ret.SetComponent( maxIndex, Cmax );
    ret.SetComponent( minIndex, Cmin );
    ret.SetComponent( midIndex, Cmid );
    return  ret;
}

//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Hue
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( Hue )
    Cr.TPixelBase< _SH >::operator=( SetLum< _SH >( SetSat< _SH >( Cs, Sat< _SH >( Cb ) ), Lum< _SH >( Cb ) ) );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Saturation
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( Saturation )
    Cr.TPixelBase< _SH >::operator=( SetLum< _SH >( SetSat< _SH >( Cb, Sat< _SH >( Cs ) ), Lum< _SH >( Cb ) ) );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------- Color
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( Color )
    Cr.TPixelBase< _SH >::operator=( SetLum< _SH >( Cs, Lum< _SH >( Cb ) ) );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Luminosity
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( Luminosity )
    Cr.TPixelBase< _SH >::operator=( SetLum< _SH >( Cb, Lum< _SH >( Cs ) ) );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- DarkerColor
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( DarkerColor )
    Cr.TPixelBase< _SH >::operator=( Lum< _SH >( Cb ) < Lum< _SH >( Cs ) ? Cb : Cs );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------- LighterColor
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_START( LighterColor )
    Cr.TPixelBase< _SH >::operator=( Lum< _SH >( Cb ) > Lum< _SH >( Cs ) ? Cb : Cs );
ULIS_SPEC_NONSEPARABLE_BLENDFUNC_RGBA8_COMPUTE_END
/////////////////////////////////////////////////////
// Undefines
#undef tSpec
#undef ttPixelType
#undef ttNextPixelType
#undef ttMax
#undef ttDownscale
#undef ttUpscale

} // namespace ULIS