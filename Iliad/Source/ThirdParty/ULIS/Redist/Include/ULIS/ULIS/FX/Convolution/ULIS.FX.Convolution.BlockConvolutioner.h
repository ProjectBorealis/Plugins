// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.FX.Convolution.BlockConvolutioner.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declaration for the Block Kernel Convolutioner.
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/FX/Convolution/ULIS.FX.Convolution.Kernel.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"

namespace ULIS {

template< uint32 _SH >
static TPixelValue< _SH > FetchPixel( int iX, int iY, TBlock< _SH >* iBlock )
{
    if( iX < 0 || iY < 0 || iX >= iBlock->Width() || iY >= iBlock->Height() )
    {
        return  TPixelValue< _SH >();
    }
    else
    {
        return  iBlock->PixelValue( iX, iY );
    }
}

/////////////////////////////////////////////////////
// TBlockConvolutioner
template< uint32 _SH >
class TBlockConvolutioner
{
    using tPixelType                = typename TBlock< _SH >::tPixelType;
    using tPixelValue               = TPixelValue< _SH >;
    using tPixelProxy               = TPixelProxy< _SH >;
    using tPixelBase                = TPixelBase< _SH >;
    using tPixelInfo                = TPixelInfo< _SH >;
    using tBlockInfo                = TBlockInfo< _SH >;

    static void ProcessScanLine( TBlock< _SH >* iSrcBlock
                               , TBlock< _SH >* iDstBlock
                               , const FKernel* iKernel
                               , bool           iConvolveAlpha
                               , const int      iLine
                               , const int      iX1
                               , const int      iX2 )
    {
        const int max_channels = iConvolveAlpha ? tBlockInfo::_nf._rc : tBlockInfo::_nf._nc;
        float* sum = new float[max_channels];
        for( int x = iX1; x < iX2; ++x )
        {
            for( int k = 0; k < max_channels; ++k )
                sum[k] = 0.f;

            for( int i = 0; i < iKernel->Size().x; ++i )
            {
                for( int j = 0; j < iKernel->Size().y; ++j )
                {
                    int srcConvolX = x + i - iKernel->Anchor().x;
                    int srcConvolY = iLine + j - iKernel->Anchor().y;
                    tPixelValue srcValue = FetchPixel< _SH >( srcConvolX, srcConvolY, iSrcBlock );
                    float convolValue = iKernel->At( i, j );
                    for( int k = 0; k < max_channels; ++k )
                        sum[k] += srcValue.GetComponent( k ) * convolValue;
                }
            }
            tPixelProxy dstProxy = iDstBlock->PixelProxy( x, iLine );
            for( int k = 0; k < max_channels; ++k )
                dstProxy.SetComponent( k, tPixelType( FMath::Clamp( sum[k], 0.f, float( tPixelInfo::Max() ) ) ) );

            if( !iConvolveAlpha )
                dstProxy.SetAlpha( iSrcBlock->PixelProxy( x, iLine ).GetAlpha() );
        }                              
        delete[] sum;
    }

public:
    static inline void Run( TBlock< _SH >*              iSrcBlock
                          , TBlock< _SH >*              iDstBlock
                          , const FKernel&              iKernel
                          , bool                        iConvolveAlpha = true
                          , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iSrcBlock->Width();
        const int y2 = iSrcBlock->Height();

        /*
        for( int y = y1; y < y2; ++y )
            ProcessScanLine( iSrcBlock, iDstBlock, iKernel, iConvolveAlpha, y, x1, x2 );
        */

        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iSrcBlock, iDstBlock, &iKernel, iConvolveAlpha, y, x1, x2 );

        global_pool.WaitForCompletion();
    }
};


} // namespace ULIS