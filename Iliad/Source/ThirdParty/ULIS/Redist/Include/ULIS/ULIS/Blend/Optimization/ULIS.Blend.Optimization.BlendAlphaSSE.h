// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.Optimization.BlendAlphaSSE.h
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
template< eAlphaMode _AM >
struct BlendAlphaSSE {
    static inline
    __m128 Compute( const __m128& AlphaBack, const __m128& AlphaTop )
    {
        return  _mm_sub_ps( _mm_add_ps( AlphaBack, AlphaTop ), _mm_div_ps( _mm_mul_ps( AlphaBack, AlphaTop ), _mm_set_ps1( 255.f ) ) );
    }
};

#define ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( iMode )                         \
    template<> struct BlendAlphaSSE< eAlphaMode::k ## iMode > {                 \
    static inline __m128 Compute( const __m128& AlphaBack, const __m128& AlphaTop ) {
#define ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END }};


//return  _mm_max_ps( _mm_setzero_ps(), _mm_sub_ps( AlphaBack, AlphaTop ) );


// SPEC ALPHA
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Normal
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Normal )
    return  _mm_sub_ps( _mm_add_ps( AlphaBack, AlphaTop ), _mm_div_ps( _mm_mul_ps( AlphaBack, AlphaTop ), _mm_set_ps1( 255.f ) ) );
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END

//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------- Erase
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Erase )
    return  _mm_div_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), AlphaTop ), AlphaBack  ), _mm_set_ps1( 255.f ) );
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END

//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Top
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Top )
    return  AlphaTop;
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------- Back
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Back )
    return  AlphaBack;
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END

//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Sub
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Sub )
    return  _mm_max_ps( _mm_setzero_ps(), _mm_sub_ps( AlphaBack, AlphaTop ) );
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END

//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Add
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Add )
    return  _mm_min_ps( _mm_set1_ps( 255.f ), _mm_add_ps( AlphaBack, AlphaTop ) );
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Mul
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Mul )
    return  _mm_div_ps( _mm_mul_ps( AlphaBack, AlphaTop ), _mm_set_ps1( 255.f ) );
ULIS_SPEC_BLENDFUNC_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Min
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Min )
    return  _mm_min_ps( AlphaBack, AlphaTop );
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END
//--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------- Max
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_START( Max )
    return  _mm_max_ps( AlphaBack, AlphaTop );
ULIS_SPEC_BLENDALPHA_SSE_COMPUTE_END
/////////////////////////////////////////////////////
// Undefines
#undef tSpec
#undef ttPixelType
#undef ttNextPixelType
#undef ttMax
#undef ttDownscale
#undef ttUpscale

} // namespace ULIS