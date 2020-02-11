// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Conv.BlockTypeConverter.h
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

namespace ULIS {
/////////////////////////////////////////////////////
// TBlockTypeConverter_ScanLine
template< uint32 _SHSrc, uint32 _SHDst >
class TBlockTypeConverter_ScanLine
{
public:
    static void ProcessScanLine( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst, int iLine, int iX1, int iX2 )
    {
        using tPixelProxySrc = typename TBlock< _SHSrc >::tPixelProxy;
        using tPixelProxyDst = typename TBlock< _SHDst >::tPixelProxy;
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;

        for( int x = iX1; x < iX2; ++x )
        {
            tPixelProxySrc srcProxy = iBlockSrc->PixelProxy( x, iLine );
            tPixelProxyDst dstProxy = iBlockDst->PixelProxy( x, iLine );

            for( int i = 0; i < src_info::_nf._nc; ++i )
                dstProxy.SetComponent( i, ConvType< typename tPixelProxySrc::tPixelType, typename tPixelProxyDst::tPixelType >( srcProxy.GetComponent( i ) ) );

            dstProxy.SetAlpha( ConvType< typename tPixelProxySrc::tPixelType, typename tPixelProxyDst::tPixelType >( srcProxy.GetAlpha() ) );
        }
    }

    static void Run( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst )
    {
        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iBlockSrc->Width();
        const int y2 = iBlockSrc->Height();
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iBlockSrc, iBlockDst, y, x1, x2 );

        global_pool.WaitForCompletion();
    }
};

/////////////////////////////////////////////////////
// TBlockTypeConverter_MonoThread
template< uint32 _SHSrc, uint32 _SHDst >
class TBlockTypeConverter_MonoThread
{
public:
    static void Run( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst )
    {
        using tPixelProxySrc = typename TBlock< _SHSrc >::tPixelProxy;
        using tPixelProxyDst = typename TBlock< _SHDst >::tPixelProxy;
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;

        for( int y = 0; y < iBlockSrc->Height(); ++y )
        {
            for( int x = 0; x < iBlockSrc->Width(); ++x )
            {
                tPixelProxySrc srcProxy = iBlockSrc->PixelProxy( x, y );
                tPixelProxyDst dstProxy = iBlockDst->PixelProxy( x, y );
                for( int i = 0; i < src_info::_nf._nc; ++i )
                    dstProxy.SetComponent( i, ConvType< typename tPixelProxySrc::tPixelType, typename tPixelProxyDst::tPixelType >( srcProxy.GetComponent( i ) ) );
                dstProxy.SetAlpha( ConvType< typename tPixelProxySrc::tPixelType, typename tPixelProxyDst::tPixelType >( srcProxy.GetAlpha() ) );
            }
        }
    }
};


/////////////////////////////////////////////////////
// TBlockTypeConverter_Imp
template< uint32 _SHSrc, uint32 _SHDst >
class TBlockTypeConverter_Imp
{
public:
    static inline void Run( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            TBlockTypeConverter_ScanLine< _SHSrc, _SHDst >::Run( iBlockSrc, iBlockDst );
        }
        else
        {
            TBlockTypeConverter_MonoThread< _SHSrc, _SHDst >::Run( iBlockSrc, iBlockDst );
        }
    }
};


/////////////////////////////////////////////////////
// TBlockTypeConverter
template< uint32 _SHSrc, uint32 _SHDst, int _MODEL_DIFF >
class TBlockTypeConverter
{
public:
    static inline void Run( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        static_assert( true, "Cannot convert type in different models" );
        assert( true );
    }
};

/////////////////////////////////////////////////////
// TBlockTypeConverter
template< uint32 _SHSrc, uint32 _SHDst >
class TBlockTypeConverter< _SHSrc, _SHDst, 0 >
{
public:
    static inline void Run( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockTypeConverter_Imp< _SHSrc, _SHDst >::Run( iBlockSrc, iBlockDst, iPerformanceOptions);
    }
};


/////////////////////////////////////////////////////
// TPixelTypeConverter
template< uint32 _SHSrc, uint32 _SHDst, int _MODEL_DIFF >
class TPixelTypeConverter
{
public:
    static inline void Apply( const TPixelBase< _SHSrc >& iSrc, TPixelBase< _SHDst >& iDst )
    {
    }
};

/////////////////////////////////////////////////////
// TPixelTypeConverter
template< uint32 _SHSrc, uint32 _SHDst >
class TPixelTypeConverter< _SHSrc, _SHDst, 0 >
{
public:
    static inline void Apply( const TPixelBase< _SHSrc >& iSrc, TPixelBase< _SHDst >& iDst )
    {
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;
        using tSrcType = typename TPixelBase< _SHSrc >::tPixelType;
        using tDstType = typename TPixelBase< _SHDst >::tPixelType;
        for( int i = 0; i < src_info::_nf._nc; ++i )
            iDst.SetComponent( i, ConvType< tSrcType, tDstType >( iSrc.GetComponent( i ) ) );
        iDst.SetAlpha( ConvType< tSrcType, tDstType >( iSrc.GetAlpha() ) );
    }
};


} // namespace ULIS

