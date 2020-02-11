// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.FX.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FFXContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/FX/Convolution/ULIS.FX.Convolution.Kernel.h"

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FFXContext
/// @brief      The FFXContext class provides a context for all FX operations on Blocks.
class FFXContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    static  void  WhiteNoise( IBlock* iBlock, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true );
    static  void  ValueNoise( IBlock* iBlock, float iFrequency, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true );
    static  void  BrownianNoise( IBlock* iBlock, float iFrequency = 0.22f, float iFrequencyMult = 1.8f, float iAmplitudeMult = 0.35f, uint8 iNumLayers = 5, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true );
    static  void  VoronoiNoise( IBlock* iBlock, uint32 iCount, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true );
    static  void  Clouds( IBlock* iBlock, int iSeed = -1, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true );
    static  void  Convolution( IBlock* iSrc, IBlock* iDst, const FKernel& iKernel, bool iConvolveAlpha = true, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() );
};

} // namespace ULIS

