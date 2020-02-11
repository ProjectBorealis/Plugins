// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Transform.BlockTransformer.h
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

namespace ULIS {
/////////////////////////////////////////////////////
// TBlockTransformer_Linear_ScanLine
template< uint32 _SH >
class TBlockTransformer_Linear_ScanLine
{
public:
    static void ProcessScanLine( const TBlock< _SH >*       iSrcBlock
                               , TBlock< _SH >*             iDstBlock
                               , const glm::mat3&           iInverseTransform
                               , const int                  iLine
                               , const int                  iX1
                               , const int                  iX2
                               , const glm::vec2&           iSrdDeltaX )
    {
        using tPixelType = typename TBlock< _SH >::tPixelType;
        using tPixelValue = typename TBlock< _SH >::tPixelValue;
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using tPixelBase = TPixelBase< _SH >;
        using info = TBlockInfo< _SH >;

        const int maxx = iSrcBlock->Width();
        const int maxy = iSrcBlock->Height();
        tPixelValue fallback = tPixelValue();
        fallback.SetAlpha( 0 );

        glm::vec2 pointInSrc = glm::vec2( iInverseTransform * glm::vec3( iX1, iLine, 1.f ) ) - glm::vec2( 0.5, 0.5 );

        for( int x = iX1; x < iX2; ++x )
        {
            int     left        = FMath::Floor( pointInSrc.x );
            int     up          = FMath::Floor( pointInSrc.y );
            int     right       = left + 1;
            int     bot         = up + 1;
            float   tx          = pointInSrc.x - float( left );
            float   ty          = pointInSrc.y - float( up );

            auto lerp = [&]( const tPixelValue& iA, const tPixelValue& iB, float t ) {
                tPixelValue ret;
                float al_a = ConvType< tPixelType, float >( iA.GetAlpha() );
                float al_b = ConvType< tPixelType, float >( iB.GetAlpha() );
                float al_c = ( al_a * ( 1.f - t ) + al_b * ( t ) );
                ret.SetAlpha( ConvType< float, tPixelType >( al_c ) );
                for( int i = 0; i < info::_nf._nc; ++i ) {
                    tPixelType el_a = iA.GetComponent( i ) * al_a;
                    tPixelType el_b = iB.GetComponent( i ) * al_b;
                    tPixelType el_r = static_cast< tPixelType >( ( el_a * ( 1.f - t ) + el_b * ( t ) ) / al_c );
                    ret.SetComponent( i, el_r );
                }
                return  ret;
            };

            /*
            const tPixelValue c00   = isInside ? iSrcBlock->PixelValue( left, up )      : fallback;
            const tPixelValue c10   = isInside ? iSrcBlock->PixelValue( right, up )     : fallback;
            const tPixelValue c11   = isInside ? iSrcBlock->PixelValue( right, bot )    : fallback;
            const tPixelValue c01   = isInside ? iSrcBlock->PixelValue( left, bot )     : fallback;
            */
            #define TEMP( iX, iY ) ( iX < 0 || iY < 0 || iX >= maxx || iY >= maxy ) ? fallback : iSrcBlock->PixelValue( iX, iY );
            const tPixelValue c00   = TEMP( left, up );
            const tPixelValue c10   = TEMP( right, up );
            const tPixelValue c11   = TEMP( right, bot );
            const tPixelValue c01   = TEMP( left, bot );
            const tPixelValue a = lerp( c00, c10, tx );
            const tPixelValue b = lerp( c01, c11, tx );
            const tPixelValue c = lerp( a, b, ty );
            iDstBlock->SetPixelValue( x, iLine, c );

            pointInSrc += iSrdDeltaX;
        }
    }

    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform )
    {
        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iDstBlock->Width();
        const int y2 = iDstBlock->Height();

        glm::vec2 src_dx = glm::vec2( iInverseTransform * glm::vec3( 1.f, 0.f, 0.f ) );

        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iSrcBlock, iDstBlock, iInverseTransform, y, x1, x2, src_dx );
        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockTransformer_Linear_MonoThread
template< uint32 _SH >
class TBlockTransformer_Linear_MonoThread
{
public:
    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform )
    {
        using tPixelType = typename TBlock< _SH >::tPixelType;
        using tPixelValue = typename TBlock< _SH >::tPixelValue;
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using tPixelBase = TPixelBase< _SH >;
        using info = TBlockInfo< _SH >;

        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iDstBlock->Width();
        const int y2 = iDstBlock->Height();
        const int maxx = iSrcBlock->Width();
        const int maxy = iSrcBlock->Height();
        tPixelValue fallback = tPixelValue();
        fallback.SetAlpha( 0 );
        for( int y = y1; y < y2; ++y )
        {
            for( int x = x1; x < x2; ++x )
            {
                glm::vec2 point_in_src = glm::vec2( iInverseTransform * glm::vec3( x, y, 1.f ) ) - glm::vec2( 0.5, 0.5 );
                int left    = floor( point_in_src.x );
                int up      = floor( point_in_src.y );
                int right   = left + 1;
                int bot     = up + 1;
                float tx = point_in_src.x - float( left );
                float ty = point_in_src.y - float( up );

                auto lerp = [&]( const tPixelBase& iA, const tPixelBase& iB, float t ) {
                    tPixelValue ret;
                    float al_a = ConvType< tPixelType, float >( iA.GetAlpha() );
                    float al_b = ConvType< tPixelType, float >( iB.GetAlpha() );
                    float al_c = ( al_a * ( 1.f - t ) + al_b * ( t ) );
                    ret.SetAlpha( ConvType< float, tPixelType >( al_c ) );
                    for( int i = 0; i < info::_nf._nc; ++i ) {
                        tPixelType el_a = iA.GetComponent( i ) * al_a;
                        tPixelType el_b = iB.GetComponent( i ) * al_b;
                        tPixelType el_r = static_cast< tPixelType >( ( el_a * ( 1.f - t ) + el_b * ( t ) ) / al_c );
                        ret.SetComponent( i, el_r );
                    }
                    return  ret;
                };
                #define TEMP( iX, iY ) ( iX < 0 || iY < 0 || iX >= maxx || iY >= maxy ) ? fallback : iSrcBlock->PixelValue( iX, iY );
                const tPixelValue c00   = TEMP( left, up );
                const tPixelValue c10   = TEMP( right, up );
                const tPixelValue c11   = TEMP( right, bot );
                const tPixelValue c01   = TEMP( left, bot );
                const tPixelValue a = lerp( c00, c10, tx );
                const tPixelValue b = lerp( c01, c11, tx );
                const tPixelValue c = lerp( a, b, ty );
                iDstBlock->SetPixelValue( x, y, c );
            }
        }
    }
};


/////////////////////////////////////////////////////
// TBlockTransformer_Linear
template< uint32 _SH >
class TBlockTransformer_Linear
{
public:
    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform
                   , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 && FGlobalThreadPool::Get().GetNumWorkers() > 1 )
        {
            TBlockTransformer_Linear_ScanLine< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform );
        }
        else
        {
            TBlockTransformer_Linear_MonoThread< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform );
        }
    }
};

} // namespace ULIS

