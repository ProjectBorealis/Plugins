// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.Optimization.RGBA8.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Blend/ULIS.Blend.BlendingModes.h"
#include "ULIS/Blend/ULIS.Blend.BlockBlender.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalCPUConfig.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"
#include "ULIS/Blend/Optimization/ULIS.Blend.Optimization.BlendFuncSSE.h"
#include "ULIS/Blend/Optimization/ULIS.Blend.Optimization.BlendAlphaSSE.h"
#include <immintrin.h>

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

template< eBlendingMode _BM >
struct Composer_RGBA8_SSE
{
    static
    __m128
    BasicCompositing( __m128 Cb, __m128 Cs, __m128 ab, __m128 var ) {
        __m128 compute = BlendFuncSSE< _BM >::Compute( Cb, Cs );
        return  _mm_div_ps( _mm_add_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), var ), Cb ), _mm_mul_ps( var, _mm_div_ps( _mm_add_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), ab ), Cs ), _mm_mul_ps( ab, compute ) ), _mm_set_ps1( 255.f ) ) ) ), _mm_set_ps1( 255.f ) );
    }
};

template<>
struct Composer_RGBA8_SSE< eBlendingMode::kErase >
{
    static
    __m128
    BasicCompositing( __m128 Cb, __m128 Cs, __m128 ab, __m128 var ) {
        return  Cb;
    }
};

/////////////////////////////////////////////////////
// TPixelBlender_RGBA8_SSE
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM
        , bool          _NS >
struct TPixelBlender_RGBA8_SSE
{
     // Type Info
    using tPixelType                = typename TBlock< _SH >::tPixelType;
    using tPixelValue               = TPixelValue< _SH >;
    using tPixelProxy               = TPixelProxy< _SH >;
    using tPixelBase                = TPixelBase< _SH >;
    using tPixelInfo                = TPixelInfo< _SH >;
    using tBlockInfo                = TBlockInfo< _SH >;
    static constexpr const uint8 mAIndex = tPixelInfo::RedirectedIndex( 3 );
    static inline void ProcessPixel( tPixelType* iBackPixelPtr, tPixelType* iTopPixelPtr, float iOpacity )
    {
        __m128 backElementsf    = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_loadu_si128( (const __m128i*)iBackPixelPtr ) ) );
        __m128 topElementsf     = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_loadu_si128( (const __m128i*)iTopPixelPtr ) ) );
        __m128 backAlphaf = _mm_set_ps1( float( *( iBackPixelPtr + mAIndex ) ) );
        __m128 topAlphaf = _mm_div_ps( _mm_mul_ps( _mm_set_ps1( float( *( iTopPixelPtr + mAIndex ) ) ), _mm_set_ps1( iOpacity ) ), _mm_set_ps1( 255.f ) );
        __m128 alphaComp = BlendAlphaSSE< eAlphaMode::kNormal >::Compute( backAlphaf, topAlphaf );
        __m128 alphaResultf = BlendAlphaSSE< _AM >::Compute( backAlphaf, topAlphaf );
        __m128 vcmp = _mm_cmpeq_ps( alphaComp, _mm_setzero_ps() );
        int mask = _mm_movemask_ps( vcmp );
        bool result = ( mask == 0xf );
        __m128 var = result ? _mm_setzero_ps() : _mm_div_ps( _mm_mul_ps( topAlphaf, _mm_set_ps1( 255.f ) ), alphaComp );
        /*
        __m128 compute = BlendFuncSSE< _BM >::Compute( backElementsf, topElementsf );
        __m128 elementsResult = _mm_div_ps( _mm_add_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), var ), backElementsf ), _mm_mul_ps( var, _mm_div_ps( _mm_add_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), backAlphaf ), topElementsf ), _mm_mul_ps( backAlphaf, compute ) ), _mm_set_ps1( 255.f ) ) ) ), _mm_set_ps1( 255.f ) );
        */
        __m128 elementsResult = Composer_RGBA8_SSE< _BM >::BasicCompositing( backElementsf, topElementsf, backAlphaf, var );
        __m128i y = _mm_cvtps_epi32( elementsResult );                  // Convert them to 32-bit ints
        y = _mm_packus_epi32(y, y);                                     // Pack down to 16 bits
        y = _mm_packus_epi16(y, y);                                     // Pack down to 8 bits
        *(uint32*)iBackPixelPtr = (uint32)_mm_cvtsi128_si32(y);          // Store the lower 32 bits
        __m128i alpha_result = _mm_cvtps_epi32( alphaResultf );         // Convert them to 32-bit ints
        alpha_result = _mm_packus_epi32(alpha_result, alpha_result);    // Pack down to 16 bits
        alpha_result = _mm_packus_epi16(alpha_result, alpha_result);    // Pack down to 8 bits
        uint32 alpha = (uint32)_mm_cvtsi128_si32(alpha_result);         // Store the lower 32 bits
        memcpy( iBackPixelPtr + mAIndex, &alpha, 1 );
    }
};

/////////////////////////////////////////////////////
// TPixelBlender_RGBA8_SSE, Specialization for Non Separable blending modes
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM >
struct TPixelBlender_RGBA8_SSE< _SH
                              , _BM
                              , _AM
                              , true >
{
     // Type Info
    using tPixelType                = typename TBlock< _SH >::tPixelType;
    using tPixelValue               = TPixelValue< _SH >;
    using tPixelProxy               = TPixelProxy< _SH >;
    using tPixelBase                = TPixelBase< _SH >;
    using tPixelInfo                = TPixelInfo< _SH >;
    using tBlockInfo                = TBlockInfo< _SH >;
    static constexpr const uint8 mAIndex = tPixelInfo::RedirectedIndex( 3 );
    static inline void ProcessPixel( tPixelType* iBackPixelPtr, tPixelType* iTopPixelPtr, float iOpacity )
    {
        tPixelProxy back( iBackPixelPtr );
        tPixelProxy top( iTopPixelPtr );
        tPixelValue componentsResult;
        BlendFuncNS_RGBA8< _SH, _BM >::Compute( back, top, componentsResult );

        __m128 backElementsf    = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_loadu_si128( (const __m128i*)iBackPixelPtr ) ) );
        __m128 topElementsf     = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_loadu_si128( (const __m128i*)iTopPixelPtr ) ) );
        __m128 backAlphaf = _mm_set_ps1( float( *( iBackPixelPtr + mAIndex ) ) );
        __m128 topAlphaf = _mm_div_ps( _mm_mul_ps( _mm_set_ps1( float( *( iTopPixelPtr + mAIndex ) ) ), _mm_set_ps1( iOpacity ) ), _mm_set_ps1( 255.f ) );
        __m128 alphaComp = BlendAlphaSSE< eAlphaMode::kNormal >::Compute( backAlphaf, topAlphaf );
        __m128 alphaResultf = BlendAlphaSSE< _AM >::Compute( backAlphaf, topAlphaf );
        __m128 vcmp = _mm_cmpeq_ps( alphaComp, _mm_setzero_ps() );
        int mask = _mm_movemask_ps( vcmp );
        bool result = ( mask == 0xf );
        __m128 var = result ? _mm_setzero_ps() : _mm_div_ps( _mm_mul_ps( topAlphaf, _mm_set_ps1( 255.f ) ), alphaComp );
        __m128 compute = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_loadu_si128( (const __m128i*)componentsResult.Ptr() ) ) );
        __m128 elementsResult = _mm_div_ps( _mm_add_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), var ), backElementsf ), _mm_mul_ps( var, _mm_div_ps( _mm_add_ps( _mm_mul_ps( _mm_sub_ps( _mm_set_ps1( 255.f ), backAlphaf ), topElementsf ), _mm_mul_ps( backAlphaf, compute ) ), _mm_set_ps1( 255.f ) ) ) ), _mm_set_ps1( 255.f ) );

        __m128i y = _mm_cvtps_epi32( elementsResult );                  // Convert them to 32-bit ints
        y = _mm_packus_epi32(y, y);                                     // Pack down to 16 bits
        y = _mm_packus_epi16(y, y);                                     // Pack down to 8 bits
        *(uint32*)iBackPixelPtr = (uint32)_mm_cvtsi128_si32(y);          // Store the lower 32 bits
        __m128i alpha_result = _mm_cvtps_epi32( alphaResultf );         // Convert them to 32-bit ints
        alpha_result = _mm_packus_epi32(alpha_result, alpha_result);    // Pack down to 16 bits
        alpha_result = _mm_packus_epi16(alpha_result, alpha_result);    // Pack down to 8 bits
        uint32 alpha = (uint32)_mm_cvtsi128_si32(alpha_result);         // Store the lower 32 bits
        memcpy( iBackPixelPtr + mAIndex, &alpha, 1 );
    }
};

/////////////////////////////////////////////////////
// TPixelBlender_RGBA8_SSE, Specialization for Dissolve blending mode
template< uint32        _SH
        , eAlphaMode    _AM >
struct TPixelBlender_RGBA8_SSE< _SH
                              , eBlendingMode::kDissolve
                              , _AM
                              , true >
{
    // Type Info
    using tPixelType                = typename TBlock< _SH >::tPixelType;
    using tPixelValue               = TPixelValue< _SH >;
    using tPixelProxy               = TPixelProxy< _SH >;
    using tPixelBase                = TPixelBase< _SH >;
    using tPixelInfo                = TPixelInfo< _SH >;
    using tBlockInfo                = TBlockInfo< _SH >;
    static constexpr const uint8 mAIndex = tPixelInfo::RedirectedIndex( 3 );
    static inline void ProcessPixel( tPixelType* iBackPixelPtr, tPixelType* iTopPixelPtr, float iOpacity )
    {
        tPixelProxy back( iBackPixelPtr );
        tPixelProxy top( iTopPixelPtr );

        float threshold = ConvType< typename TBlock< _SH >::tNextPixelType, float >( (typename TBlock< _SH >::tNextPixelType)( top.GetAlpha() * iOpacity ) );
        float toss = 0.f;
        {
            float sum = uint64( iBackPixelPtr ) * uint64( iTopPixelPtr ) + ( uint64( iBackPixelPtr ) + uint64( iTopPixelPtr ) );
            toss = float( ( int( 8253729 * ( ( sin( sum ) + cos( sum ) + 2.0f ) / 4.0f ) ) + 2396403 ) % 65537 ) / 65537.f;
        }

        if( toss < threshold )
        {
            const typename TBlock< _SH >::tPixelType alphaBack   = back.GetAlpha();
            const typename TBlock< _SH >::tPixelType alphaResult = BlendAlpha< _SH, _AM >::Compute( alphaBack, TBlock< _SH >::StaticMax() );
            back.TPixelBase< _SH >::operator=( top );
            back.SetAlpha( alphaResult );
        }
    }
};

/////////////////////////////////////////////////////
// TBlockBlender_RGBA8_SSE
template< uint32 _SH, eBlendingMode _BM, eAlphaMode _AM, bool _NS >
class TBlockBlender_RGBA8_SSE
{
     // Type Info
    using tPixelType                = typename TBlock< _SH >::tPixelType;
    using tPixelValue               = TPixelValue< _SH >;
    using tPixelProxy               = TPixelProxy< _SH >;
    using tPixelBase                = TPixelBase< _SH >;
    using tPixelInfo                = TPixelInfo< _SH >;
    using tBlockInfo                = TBlockInfo< _SH >;

public:
    static void ProcessScanLineSSE( TBlock< _SH >*                     iBlockTop
                                  , TBlock< _SH >*                     iBlockBack
                                  , typename TBlock< _SH >::tPixelType iOpacity
                                  , const int                          iLine
                                  , const int                          iX1
                                  , const int                          iX2
                                  , const FPoint&                      iShift )
    {
        // Base ptrs for scanlines
        tPixelType* backPixelPtr    = iBlockBack->PixelPtr( iX1, iLine );
        tPixelType* topPixelPtr     = iBlockTop->PixelPtr( iX1 + iShift.x, iLine + iShift.y );
        float opacity               = (float)iOpacity;
        const int op = iX2 - iX1;

        for( int i = 0; i < op; ++i )
        {
            TPixelBlender_RGBA8_SSE< _SH, _BM, _AM, _NS >::ProcessPixel( backPixelPtr, topPixelPtr, opacity );
            backPixelPtr    += tBlockInfo::_nf._pd;
            topPixelPtr     += tBlockInfo::_nf._pd;
        }
    }

    static void ProcessBlockSSE( TBlock< _SH >*                     iBlockTop
                               , TBlock< _SH >*                     iBlockBack
                               , typename TBlock< _SH >::tPixelType iOpacity
                               , const FRect&                       iROI
                               , const FPoint&                      iShift )
    {
        // Geom
        const int x1        = iROI.x;
        const int y1        = iROI.y;
        const int x2        = x1 + iROI.w;
        const int y2        = y1 + iROI.h;

        // Base ptrs for scanlines
        tPixelType* backPixelPtr;//     = iBlockBack->PixelPtr( iX1, iLine );
        tPixelType* topPixelPtr; //     = iBlockTop->PixelPtr( iX1 + iShift.x, iLine + iShift.y );
        float opacity               = (float)iOpacity;
        const int opx = x2 - x1;

        for( int j = y1; j < y2; ++j )
        {
            backPixelPtr    = iBlockBack->PixelPtr( x1, j );
            topPixelPtr     = iBlockTop->PixelPtr( x1 + iShift.x, j + iShift.y );
            for( int i = 0; i < opx; ++i )
            {
                TPixelBlender_RGBA8_SSE< _SH, _BM, _AM, _NS >::ProcessPixel( backPixelPtr, topPixelPtr, opacity );
                backPixelPtr    += tBlockInfo::_nf._pd;
                topPixelPtr     += tBlockInfo::_nf._pd;
            }
        }
    }

    static void Run( TBlock< _SH >*                     iBlockTop
                   , TBlock< _SH >*                     iBlockBack
                   , typename TBlock< _SH >::tPixelType iOpacity
                   , const FRect&                       iROI
                   , const FPoint&                      iShift
                   , const FPerformanceOptions&         iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            const int x1 = iROI.x;
            const int y1 = iROI.y;
            const int x2 = x1 + iROI.w;
            const int y2 = y1 + iROI.h;
            FThreadPool& global_pool = FGlobalThreadPool::Get();
            for( int y = y1; y < y2; ++y )
                global_pool.ScheduleJob( TBlockBlender_RGBA8_SSE< _SH, _BM, _AM, _NS >::ProcessScanLineSSE, iBlockTop, iBlockBack, iOpacity, y, x1, x2, iShift );
            global_pool.WaitForCompletion();
        }
        else
        {
            ProcessBlockSSE( iBlockTop, iBlockBack, iOpacity, iROI, iShift );
        }
    }
};

/////////////////////////////////////////////////////
// TBlockBlender_Imp
template< uint32        _SH     // Format
        , eBlendingMode _BM     // Blending Mode
        , eAlphaMode    _AM     // Alpha Mode
        , bool          _NS     // Non Separable
        , uint32        _LH     // Layout
        , e_nm          _NM     // Normalized
        , bool          _DM >   // Decimal
class TBlockBlender_Imp< _SH,                // Format
                         _BM,                // Blending Mode
                         _AM,                // Alpha Mode
                         _NS,                // Non Separable
                         e_tp::kuint8,       // uint8
                         e_cm::kRGB,         // RGB
                         e_ea::khasAlpha,    // Alpha
                         _LH,                // Layout
                         _NM,                // Normalized
                         _DM >               // Decimal
{
public:
    static void Run( TBlock< _SH >*                     iBlockTop
                   , TBlock< _SH >*                     iBlockBack
                   , typename TBlock< _SH >::tPixelType iOpacity
                   , const FRect&                       iROI
                   , const FPoint&                      iShift
                   , const FPerformanceOptions&         iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockBlender_RGBA8_SSE< _SH, _BM, _AM, _NS >::Run( iBlockTop
                                                          , iBlockBack
                                                          , iOpacity
                                                          , iROI
                                                          , iShift
                                                          , iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS