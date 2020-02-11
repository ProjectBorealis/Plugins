// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.FX.FXContext.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/FX/Noise/ULIS.FX.Noise.ValueNoise.h"
#include "ULIS/FX/Noise/ULIS.FX.Noise.WhiteNoise.h"
#include "ULIS/FX/Noise/ULIS.FX.Noise.BrownianNoise.h"
#include "ULIS/FX/Noise/ULIS.FX.Noise.VoronoiNoise.h"
#include "ULIS/FX/Convolution/ULIS.FX.Convolution.Kernel.h"
#include "ULIS/FX/Convolution/ULIS.FX.Convolution.BlockConvolutioner.h"

namespace ULIS {
/////////////////////////////////////////////////////
// TFXContext
template< uint32 _SH >
class TFXContext
{
public:
    static void WhiteNoise( TBlock< _SH >* iBlock, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        assert( iBlock );
        TWhiteNoiseGenerator< _SH >::Run( iBlock, iSeed, iPerformanceOptions );
    }

    static void ValueNoise( TBlock< _SH >* iBlock, float iFrequency, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        assert( iBlock );
        TValueNoiseGenerator< _SH >::Run( iBlock, iFrequency, iSeed, iPerformanceOptions );
    }

    static  void  BrownianNoise( TBlock< _SH >* iBlock, float iFrequency = 0.22f, float iFrequencyMult = 1.8f, float iAmplitudeMult = 0.35f, uint8 iNumLayers = 5, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    {
        assert( iBlock );
        TBrownianNoiseGenerator< _SH >::Run( iBlock, iFrequency, iFrequencyMult, iAmplitudeMult, iNumLayers, iSeed, iPerformanceOptions );
    }

    static void VoronoiNoise( TBlock< _SH >* iBlock, uint32 iCount, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        assert( iBlock );
        TVoronoiNoiseGenerator< _SH >::Run( iBlock, iCount, iSeed, iPerformanceOptions );
    }

    static void Convolution( TBlock< _SH >* iSrcBlock, TBlock< _SH >* iDstBlock, const FKernel& iKernel, bool iConvolveAlpha = true, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    {
        assert( iSrcBlock );
        assert( iDstBlock );
        assert( iDstBlock->Width() == iSrcBlock->Width() );
        assert( iDstBlock->Height() == iSrcBlock->Height() );
        TBlockConvolutioner< _SH >::Run( iSrcBlock, iDstBlock, iKernel, iConvolveAlpha, iPerformanceOptions );
    }

};

} // namespace ULIS

