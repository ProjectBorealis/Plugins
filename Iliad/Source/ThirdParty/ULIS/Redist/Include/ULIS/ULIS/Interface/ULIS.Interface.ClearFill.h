// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.ClearFill.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FClearFillContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Color/ULIS.Color.CColor.h"
#include "ULIS/Maths/ULIS.Maths.Geometry.h"

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FClearFillContext
/// @brief      The FClearFillContext class provides a context for all Clear and Fill operations on Blocks.
class FClearFillContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         Fill( IBlock* iBlock, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Fill a block with the specified CColor.
    /// @details    The block will be entirely filled with a plain color with alpha.
    /// @param      iBlock                  The pointer to the \e IBlock to fill top.
    /// @param      iColor                  The CColor to obtain the color from. A colorspace conversion can occur if filling a block in Lab, for example.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  Fill( IBlock* iBlock
                      , const CColor& iColor
                      , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                      , bool iCallInvalidCB = true );


    /// @fn         FillPreserveAlpha( IBlock* iBlock, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Fill a block with the specified CColor, but preserves the alpha.
    /// @details    The block will be entirely filled with a color but the alpha keeps the same.
    /// @param      iBlock                  The pointer to the \e IBlock to fill top.
    /// @param      iColor                  The CColor to obtain the color from. A colorspace conversion can occur if filling a block in Lab, for example.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  FillPreserveAlpha( IBlock* iBlock
                                   , const CColor& iColor
                                   , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                   , bool iCallInvalidCB = true );


    /// @fn         FillRect( IBlock* iBlock, const CColor& iColor, const FRect& iRect, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Fill a rect in a block with the specified CColor.
    /// @details    The block will be filled in the specifier rect area with a plain color with alpha. If the area is greater than the block, the area is cropped.
    /// @param      iBlock                  The pointer to the \e IBlock to fill top.
    /// @param      iColor                  The CColor to obtain the color from. A colorspace conversion can occur if filling a block in Lab, for example.
    /// @param      iRect                   The Area to fill.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  FillRect( IBlock* iBlock
                          , const CColor& iColor
                          , const FRect& iRect
                          , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                          , bool iCallInvalidCB = true );


    /// @fn         Clear( IBlock* iBlock, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Clear a block.
    /// @details    The block will be entirely clear with transparent black ( all zero ).
    /// @param      iBlock                  The pointer to the \e IBlock to fill top.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  Clear( IBlock* iBlock
                       , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                       , bool iCallInvalidCB = true );


    /// @fn         ClearRect( IBlock* iBlock, const FRect& iRect, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Clear a rect in a block.
    /// @details    The block will be clear with transparent black ( all zero ). If the area is greater than the block, the area is cropped.
    /// @param      iBlock                  The pointer to the \e IBlock to fill top.
    /// @param      iRect                   The Area to fill.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  ClearRect( IBlock* iBlock
                           , const FRect& iRect
                           , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                           , bool iCallInvalidCB = true );

};

} // namespace ULIS

