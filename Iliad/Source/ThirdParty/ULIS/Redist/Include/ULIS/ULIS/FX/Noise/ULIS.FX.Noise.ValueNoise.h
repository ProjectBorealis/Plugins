// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.FX.Noise.ValueNoise.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"
#include "ULIS/FX/Noise/ULIS.FX.Noise.NoiseUtils.h"

namespace ULIS {
/////////////////////////////////////////////////////
// TValueNoiseGenerator_Default_ScanLine
template< uint32 _SH >
class TValueNoiseGenerator_Default_ScanLine
{
public:
    static void ProcessScanLine( TBlock< _SH >*     iBlock
                               , const int          iLine
                               , const int          iX1
                               , const int          iX2
                               , const float        iFrequency
                               , const ValueNoise&  iGen )
    {
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using info = TBlockInfo< _SH >;

        for( int x = iX1; x < iX2; ++x )
        {
            tPixelProxy proxy = iBlock->PixelProxy( x, iLine );
            Vec2f vec = Vec2f( x, iLine ) * iFrequency;
            float floatvalue = iGen.eval( vec );
            typename tPixelProxy::tPixelType value = ConvType< float, typename tPixelProxy::tPixelType >( floatvalue );

            for( int i = 0; i < info::_nf._nc; ++i )
                proxy.SetComponent( i, value );

            proxy.SetAlpha( proxy.Max() );
        }
    }

    static inline void Run( TBlock< _SH >* iBlock, float iFrequency, int iSeed )
    {
        uint32 seed = iSeed < 0 ? time( NULL ) : iSeed;
        ValueNoise noise( seed );

        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iBlock->Width();
        const int y2 = iBlock->Height();
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iBlock, y, x1, x2, iFrequency, noise );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TValueNoiseGenerator_Default_MonoThread
template< uint32 _SH >
class TValueNoiseGenerator_Default_MonoThread
{
public:
    static void Run( TBlock< _SH >* iBlock, float iFrequency, int iSeed )
    {
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using info = TBlockInfo< _SH >;

        uint32 seed = iSeed < 0 ? time( NULL ) : iSeed;
        ValueNoise noise( seed );

        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iBlock->Width();
        const int y2 = iBlock->Height();
        for( int y = y1; y < y2; ++y )
        {
            for( int x = x1; x < x2; ++x )
            {
                tPixelProxy proxy = iBlock->PixelProxy( x, y );
                Vec2f vec = Vec2f( x, y ) * iFrequency;
                float floatvalue = noise.eval( vec );
                typename tPixelProxy::tPixelType value = ConvType< float, typename tPixelProxy::tPixelType >( floatvalue );

                for( int i = 0; i < info::_nf._nc; ++i )
                    proxy.SetComponent( i, value );

                proxy.SetAlpha( proxy.Max() );
            }
        }
    }
};


/////////////////////////////////////////////////////
// TValueNoiseGenerator
template< uint32 _SH >
class TValueNoiseGenerator
{
public:
    static inline void Run( TBlock< _SH >*              iBlock
                          , float iFrequency
                          , int iSeed
                          , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            TValueNoiseGenerator_Default_ScanLine< _SH >::Run( iBlock, iFrequency, iSeed );
        }
        else
        {
            TValueNoiseGenerator_Default_MonoThread< _SH >::Run( iBlock, iFrequency, iSeed );
        }
    }
};

} // namespace ULIS

