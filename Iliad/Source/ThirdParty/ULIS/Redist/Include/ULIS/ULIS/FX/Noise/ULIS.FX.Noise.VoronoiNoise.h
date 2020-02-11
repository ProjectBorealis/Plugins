// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.FX.Noise.VoronoiNoise.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"

#include <cassert>
#include <vector>
#include <random>
#include <glm/vec2.hpp>
#include <glm/gtx/norm.hpp>

namespace ULIS {
/////////////////////////////////////////////////////
// TVoronoiNoiseGenerator_Default_ScanLine
template< uint32 _SH >
class TVoronoiNoiseGenerator_Default_ScanLine
{
public:
    static void ProcessScanLine( TBlock< _SH >* iBlock
                               , const int      iLine
                               , const int      iX1
                               , const int      iX2
                               , const float    iNormalizationFactor
                               , const std::vector< glm::vec2 >& iPoints )
    {
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using info = TBlockInfo< _SH >;

        for( int x = iX1; x < iX2; ++x )
        {
            int closest_index = 0;
            float max_distance_squared = INT_MAX;
            for( int i = 0; i < iPoints.size(); ++i )
            {
                float current_distance_squared = glm::distance2( iPoints[i], glm::vec2( x, iLine ) );
                if( current_distance_squared < max_distance_squared )
                {
                    max_distance_squared = current_distance_squared;
                    closest_index = i;
                }
            }

            tPixelProxy proxy = iBlock->PixelProxy( x, iLine );
            float floatvalue = FMath::Clamp( std::sqrt( max_distance_squared ) / iNormalizationFactor, 0.f, 1.f );
            typename tPixelProxy::tPixelType value = ConvType< float, typename tPixelProxy::tPixelType >( floatvalue );

            for( int i = 0; i < info::_nf._nc; ++i )
                proxy.SetComponent( i, value );

            proxy.SetAlpha( proxy.Max() );
        }
    }

    static inline void Run( TBlock< _SH >* iBlock, uint32 iCount, int iSeed )
    {
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using info = TBlockInfo< _SH >;

        uint32 seed = iSeed < 0 ? time( NULL ) : iSeed;
        std::minstd_rand generator( seed );

        const int fw = iBlock->Width() - 2;
        const int fh = iBlock->Height() - 2;
        std::vector< glm::vec2 > points( iCount );
        for( int i = 0; i < iCount; ++i )
            points[i] = glm::vec2( generator() % fw + 1, generator() % fh + 1 );

        float sqrcount = std::sqrt( (float)iCount );
        float cellsizew = 1 * ( fw / sqrcount );
        float cellsizeh = 1 * ( fh / sqrcount );
        float normalisation_factor = std::sqrt( cellsizew * cellsizew + cellsizeh * cellsizeh );

        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iBlock->Width();
        const int y2 = iBlock->Height();
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iBlock, y, x1, x2, normalisation_factor, points );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TVoronoiNoiseGenerator_Default_MonoThread
template< uint32 _SH >
class TVoronoiNoiseGenerator_Default_MonoThread
{
public:
    static void Run( TBlock< _SH >* iBlock, uint32 iCount, int iSeed )
    {
        using tPixelProxy = typename TBlock< _SH >::tPixelProxy;
        using info = TBlockInfo< _SH >;

        uint32 seed = iSeed < 0 ? time( NULL ) : iSeed;
        std::minstd_rand generator( seed );

        const int fw = iBlock->Width() - 2;
        const int fh = iBlock->Height() - 2;
        std::vector< glm::vec2 > points( iCount );
        for( int i = 0; i < iCount; ++i )
            points[i] = glm::vec2( generator() % fw + 1, generator() % fh + 1 );

        float sqrcount = std::sqrt( (float)iCount );
        float cellsizew = 1 * ( fw / sqrcount );
        float cellsizeh = 1 * ( fh / sqrcount );
        float normalisation_factor = std::sqrt( cellsizew * cellsizew + cellsizeh * cellsizeh );

        const int x1 = 0;
        const int y1 = 0;
        const int x2 = iBlock->Width();
        const int y2 = iBlock->Height();
        for( int y = y1; y < y2; ++y )
        {
            for( int x = x1; x < x2; ++x )
            {
                int closest_index = 0;
                float max_distance_squared = INT_MAX;
                for( int i = 0; i < iCount; ++i )
                {
                    float current_distance_squared = glm::distance2( points[i], glm::vec2( x, y ) );
                    if( current_distance_squared < max_distance_squared )
                    {
                        max_distance_squared = current_distance_squared;
                        closest_index = i;
                    }
                }

                tPixelProxy proxy = iBlock->PixelProxy( x, y );
                float floatvalue = FMath::Clamp( std::sqrt( max_distance_squared ) / normalisation_factor, 0.f, 1.f );
                typename tPixelProxy::tPixelType value = ConvType< float, typename tPixelProxy::tPixelType >( floatvalue );

                for( int i = 0; i < info::_nf._nc; ++i )
                    proxy.SetComponent( i, value );

                proxy.SetAlpha( proxy.Max() );
            }
        }
    }
};


/////////////////////////////////////////////////////
// TVoronoiNoiseGenerator
template< uint32 _SH >
class TVoronoiNoiseGenerator
{
public:
    static inline void Run( TBlock< _SH >*              iBlock
                          , uint32 iCount
                          , int iSeed
                          , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            TVoronoiNoiseGenerator_Default_ScanLine< _SH >::Run( iBlock, iCount, iSeed );
        }
        else
        {
            TVoronoiNoiseGenerator_Default_MonoThread< _SH >::Run( iBlock, iCount, iSeed );
        }
    }
};

} // namespace ULIS

