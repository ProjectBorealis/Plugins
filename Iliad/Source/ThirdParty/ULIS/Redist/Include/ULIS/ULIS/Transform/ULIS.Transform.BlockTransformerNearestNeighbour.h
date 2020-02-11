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
// TBlockTransformer_NearestNeighbour_ScanLine
template< uint32 _SH >
class TBlockTransformer_NearestNeighbour_ScanLine
{
public:
    static void ProcessScanLine( const TBlock< _SH >*   iSrcBlock
                   , TBlock< _SH >*         iDstBlock
                   , const glm::mat3&       iInverseTransform
                   , const int              iLine
                   , const int              iX1
                   , const int              iX2 )
    {
        const int maxx = iSrcBlock->Width();
        const int maxy = iSrcBlock->Height();
        const typename TBlock< _SH >::tPixelValue fallback = typename TBlock< _SH >::tPixelValue();
        for( int x = iX1; x < iX2; ++x )
        {
            glm::vec3 point_in_dst( x, iLine, 1.f );
            glm::vec2 point_in_src = ( iInverseTransform * point_in_dst );
            int src_x = floor( point_in_src.x );
            int src_y = floor( point_in_src.y );
            if( src_x < 0 || src_y < 0 || src_x >= maxx || src_y >= maxy )
                iDstBlock->SetPixelValue( x, iLine, fallback );
            else
                iDstBlock->SetPixelProxy( x, iLine, iSrcBlock->PixelProxy( src_x, src_y ) );
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
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iSrcBlock, iDstBlock, iInverseTransform, y, x1, x2 );
        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockTransformer_NearestNeighbour_MonoThread
template< uint32 _SH >
class TBlockTransformer_NearestNeighbour_MonoThread
{
public:
    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform )
    {
        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iDstBlock->Width();
        const int y2 = iDstBlock->Height();
        const int maxx = iSrcBlock->Width();
        const int maxy = iSrcBlock->Height();
        const typename TBlock< _SH >::tPixelValue fallback = typename TBlock< _SH >::tPixelValue();
        for( int y = y1; y < y2; ++y )
        {
            for( int x = x1; x < x2; ++x )
            {
                glm::vec3 point_in_dst( x, y, 1.f );
                glm::vec2 point_in_src = ( iInverseTransform * point_in_dst );
                int src_x = floor( point_in_src.x );
                int src_y = floor( point_in_src.y );
                if( src_x < 0 || src_y < 0 || src_x >= maxx || src_y >= maxy )
                {
                    iDstBlock->SetPixelValue( x, y, fallback );
                }
                else
                {
                    iDstBlock->SetPixelProxy( x, y, iSrcBlock->PixelProxy( src_x, src_y ) );
                }
            }
        }
    }
};


/////////////////////////////////////////////////////
// TBlockTransformer_NearestNeighbour
template< uint32 _SH >
class TBlockTransformer_NearestNeighbour
{
public:
    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform
                   , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 && FGlobalThreadPool::Get().GetNumWorkers() > 1 )
        {
            TBlockTransformer_NearestNeighbour_ScanLine< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform );
        }
        else
        {
            TBlockTransformer_NearestNeighbour_MonoThread< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform );
        }
    }
};

} // namespace ULIS

