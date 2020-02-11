// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Blend.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FBlendingContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Blend/ULIS.Blend.BlendingModes.h"
#include "ULIS/Maths/ULIS.Maths.Geometry.h"

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FBlendingContext
/// @brief      The FBlendingContext class provides a context for all Blending operations on Blocks.
class FBlendingContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         Blend( IBlock* iBlockTop, IBlock* iBlockBack, eBlendingMode iMode, int ix = 0, int iy = 0, float iOpacity = 1.f, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool callInvalidCB = true )
    /// @brief      Blend two block together.
    /// @details    The block on top will be blended on the back block, according to the specified parameters.
    ///                 - Warning ! Both blocks should be the same underlying format in order for the function to succeed.
    ///                 - If the two blocks do not have the same type, the function will fail with an assertion in debug builds.
    ///                 - In a release build, nothing prevents the function to run but it will end up corrupting memory and doing all sorts of crazy things.
    /// @param      iBlockTop               The pointer to the \e IBlock on top ( remains untouched ).
    /// @param      iBlockBack              The pointer to the \e IBlock on back ( receives the blend ).
    /// @param      iX                      x coordinate of the position to blend in the back block.
    /// @param      iY                      y coordinate of the position to blend in the back block.
    /// @param      iBlendingMode           The blending mode ( see \e eBlendingMode ).
    /// @param      iAlphaMode              The alpha mode ( see \e eAlphaMode ).
    /// @param      iOpacity                The opacity used to perform the blend, beetween 0 and 1.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  Blend( IBlock*                    iBlockTop
                       , IBlock*                    iBlockBack
                       , int                        iX                  = 0
                       , int                        iY                  = 0
                       , eBlendingMode              iBlendingMode       = eBlendingMode::kNormal
                       , eAlphaMode                 iAlphaMode          = eAlphaMode::kNormal
                       , float                      iOpacity            = 1.f
                       , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions()
                       , bool                       iCallInvalidCB      = true );


    /// @fn         Blend( IBlock* iBlockTop, IBlock* iBlockBack, eBlendingMode iMode, const FRect& iArea, float iOpacity = 1.f, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @details    The block on top will be blended on the back block, according to the specified parameters.
    ///                 - Warning ! Both blocks should be the same underlying format in order for the function to succeed.
    ///                 - If the two blocks do not have the same type, the function will fail with an assertion in debug builds.
    ///                 - In a release build, nothing prevents the function to run but it will end up corrupting memory and doing all sorts of crazy things.
    /// @details    The block on top will be blended on the back block, according to the specified parameters.
    /// @param      iBlockTop               The pointer to the \e IBlock on top ( remains untouched ).
    /// @param      iBlockBack              The pointer to the \e IBlock on back ( receives the blend ).
    /// @param      iArea                   The area to blend in the back block.
    /// @param      iBlendingMode           The blending mode ( see \e eBlendingMode ).
    /// @param      iAlphaMode              The alpha mode ( see \e eAlphaMode ).
    /// @param      iOpacity                The opacity used to perform the blend, beetween 0 and 1.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  Blend( IBlock*                    iBlockTop
                       , IBlock*                    iBlockBack
                       , const FRect&               iArea
                       , eBlendingMode              iBlendingMode       = eBlendingMode::kNormal
                       , eAlphaMode                 iAlphaMode          = eAlphaMode::kNormal
                       , float                      iOpacity            = 1.f
                       , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions()
                       , bool                       iCallInvalidCB      = true );

};

} // namespace ULIS

