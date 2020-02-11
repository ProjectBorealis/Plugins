// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Transform.Optimization.RGBA8.h
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
#include "ULIS/Transform/ULIS.Transform.ResamplingMethods.h"
#include "ULIS/Transform/ULIS.Transform.BlockTransformer.h"
#include "ULIS/Thread/ULIS.Thread.ParallelFor.h"
#include <immintrin.h>

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TBlockTransformer_RGBA8_Linear
template< uint32 _SH >
class TBlockTransformer_RGBA8_Linear
{
public:
    static void ProcessScanLine( const TBlock< _SH >*       iSrcBlock
                               , TBlock< _SH >*             iDstBlock
                               , const glm::mat3&           iInverseTransform
                               , const int                  iLine
                               , const int                  iWidth
                               , const glm::vec2&           iSrdDeltaX )
    {
        // Type Info
        using tPixelType                = typename TBlock< _SH >::tPixelType;
        using tPixelValue               = TPixelValue< _SH >;
        using tPixelProxy               = TPixelProxy< _SH >;
        using tPixelBase                = TPixelBase< _SH >;
        using tPixelInfo                = TPixelInfo< _SH >;
        using tBlockInfo                = TBlockInfo< _SH >;
        const int srcWidth              = iSrcBlock->Width();
        const int srcHeight             = iSrcBlock->Height();
        const int srcBytesPerScanLine   = iSrcBlock->BytesPerScanLine();
        glm::vec2 pointInSrc            = glm::vec2( iInverseTransform * glm::vec3( 0, iLine, 1.f ) ) - glm::vec2( 0.5, 0.5 );
        const tPixelType* beginSrc      = iSrcBlock->DataPtr();
        const tPixelType* endSrc        = beginSrc + iSrcBlock->BytesTotal();
        tPixelType* dstPixelPtr         = iDstBlock->ScanlinePtr( iLine );
        uint8 alpha_index               = (uint8)tPixelInfo::RedirectedIndex( 3 );

        // Loop
        for( int x = 0; x < iWidth; ++x )
        {
            // Neighbouring coordinates
            int     left        = FMath::Floor( pointInSrc.x );
            int     up          = FMath::Floor( pointInSrc.y );
            int     right       = left  + 1;
            int     bot         = up    + 1;

            // Fractional parts, linear interpolators tx ty
            float   tx          = pointInSrc.x - float( left );
            float   ty          = pointInSrc.y - float( up );

            auto lerp = [&]( const __m128& iElementsA
                           , const __m128& iAlphaA
                           , const __m128& iElementsB
                           , const __m128& iAlphaB
                           , const __m128& iT
                           , const __m128& iD
                           , __m128* oElements
                           , __m128* oAlpha )
            {
                __m128 inverseT = _mm_sub_ps( _mm_set_ps1( 1.f ), iT );
                __m128 termA = _mm_mul_ps( iAlphaA, iD );
                __m128 termB = _mm_mul_ps( iAlphaB, iT );
                *oAlpha = _mm_add_ps( termA, termB );

                __m128 vcmp = _mm_cmpeq_ps( *oAlpha, _mm_setzero_ps());
                int mask = _mm_movemask_ps (vcmp);
                bool result = (mask == 0xf);
                if( result )
                    *oElements = _mm_setzero_ps();
                else
                    *oElements = _mm_div_ps( _mm_add_ps( _mm_mul_ps( iElementsA, termA ), _mm_mul_ps( iElementsB, termB ) ), *oAlpha );
            };

            const tPixelType* c00p = iSrcBlock->PixelPtr( left, up );
            const tPixelType* c10p = c00p + iSrcBlock->Depth();
            const tPixelType* c11p = c10p + srcBytesPerScanLine;
            const tPixelType* c01p = c00p + srcBytesPerScanLine;
            __m128 c00f, c00_alphaf;
            __m128 c10f, c10_alphaf;
            __m128 c11f, c11_alphaf;
            __m128 c01f, c01_alphaf;
            #define ULIS_TMP_LOAD_SSE( iX, iY, iVal, iAlpha, iPtr )                                                 \
                if( iX >= 0 && iY >= 0 && iX < srcWidth && iY < srcHeight ) {                                       \
                    iVal    = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_loadu_si128( (const __m128i*)iPtr ) ) );      \
                    iAlpha  = _mm_set_ps1( float( *(iPtr + alpha_index) ) );                                        \
                } else {                                                                                            \
                    iVal    = _mm_setzero_ps();                                                                     \
                    iAlpha  = _mm_setzero_ps();                                                                     \
                }
            ULIS_TMP_LOAD_SSE( left, up, c00f, c00_alphaf, c00p )
            ULIS_TMP_LOAD_SSE( right, up, c10f, c10_alphaf, c10p )
            ULIS_TMP_LOAD_SSE( right, bot, c11f, c11_alphaf, c11p )
            ULIS_TMP_LOAD_SSE( left, bot, c01f, c01_alphaf, c01p )
            __m128 txf  = _mm_set_ps1( tx );
            __m128 txfi = _mm_sub_ps( _mm_set_ps1( 1.f ), txf );
            __m128 tyf  = _mm_set_ps1( ty );
            __m128 tyfi = _mm_sub_ps( _mm_set_ps1( 1.f ), tyf );

            __m128 a_elementsf;
            __m128 a_alphaf;
            __m128 b_elementsf;
            __m128 b_alphaf;
            __m128 c_elementsf;
            __m128 c_alphaf;
            lerp( c00f, c00_alphaf, c10f, c10_alphaf, txf, txfi, &a_elementsf, &a_alphaf );
            lerp( c01f, c01_alphaf, c11f, c11_alphaf, txf, txfi, &b_elementsf, &b_alphaf );
            lerp( a_elementsf, a_alphaf, b_elementsf, b_alphaf, tyf, tyfi, &c_elementsf, &c_alphaf );

            __m128i y = _mm_cvtps_epi32( c_elementsf );                     // Convert them to 32-bit ints
            y = _mm_packus_epi32(y, y);                                     // Pack down to 16 bits
            y = _mm_packus_epi16(y, y);                                     // Pack down to 8 bits
            *(uint32*)dstPixelPtr = (uint32)_mm_cvtsi128_si32(y);           // Store the lower 32 bits
            __m128i alpha_result = _mm_cvtps_epi32( c_alphaf );             // Convert them to 32-bit ints
            alpha_result = _mm_packus_epi32(alpha_result, alpha_result);    // Pack down to 16 bits
            alpha_result = _mm_packus_epi16(alpha_result, alpha_result);    // Pack down to 8 bits
            uint32 alpha = (uint32)_mm_cvtsi128_si32(alpha_result);         // Store the lower 32 bits
            memcpy( dstPixelPtr + alpha_index, &alpha, 1 );

            // Increment Walker in src
            pointInSrc += iSrdDeltaX;
            dstPixelPtr += tBlockInfo::_nf._pd;
        }
    }

    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform )
    {
        const int w = iDstBlock->Width();
        const int h = iDstBlock->Height();
        glm::vec2 src_dx = glm::vec2( iInverseTransform * glm::vec3( 1.f, 0.f, 0.f ) );
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = 0; y < h; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iSrcBlock, iDstBlock, iInverseTransform, y, w, src_dx );
        global_pool.WaitForCompletion();
    }
};

/////////////////////////////////////////////////////
// TBlockTransformer_RGBA8
template< uint32 _SH >
class TBlockTransformer_RGBA8
{
public:
    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform
                   , eResamplingMethod           iResamplingMethod = eResamplingMethod::kBilinear
                   , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        switch( iResamplingMethod )
        {
            case eResamplingMethod::kNearestNeighbour:
            {
                TBlockTransformer_NearestNeighbour< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform, iPerformanceOptions );
                break;
            }

            case eResamplingMethod::kBilinear:
            {
                TBlockTransformer_RGBA8_Linear< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform );
                break;
            }

            default: ULIS_CRASH_DELIBERATE;
        }
    }
};

/////////////////////////////////////////////////////
// TBlockTransformer_Imp Specialization
template< uint32 _SH >
class TBlockTransformer_Imp< _SH
                           , e_tp::kuint8
                           , e_cm::kRGB
                           , e_ea::khasAlpha >
{
public:
    static inline void Run( const TBlock< _SH >*        iSrcBlock
                          , TBlock< _SH >*              iDstBlock
                          , const glm::mat3&            iInverseTransform
                          , eResamplingMethod           iResamplingMethod = eResamplingMethod::kBilinear
                          , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockTransformer_RGBA8< _SH >::Run( iSrcBlock
                                           , iDstBlock
                                           , iInverseTransform
                                           , iResamplingMethod
                                           , iPerformanceOptions );
    }
};


/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS

