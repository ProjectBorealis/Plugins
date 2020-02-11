// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Make.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FMakeContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Maths/ULIS.Maths.Geometry.h"
#include <string>

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FMakeContext
/// @brief      The FMakeContext class provides a context for all Copy and Make operations on Blocks.
class FMakeContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         static  IBlock*  MakeBlock( int iWidth, int iHeight, uint32_t iFormat, const std::string& iProfileTag = "default" )
    /// @brief      Make a block with dynamic type deduction at runtime.
    /// @details    Use an existing hash from profile registry or available from the ULIS::Format namespace for the iFormat param.
    /// @param      iWidth          The width of the block to allocate.
    /// @param      iHeight         The height of the block to allocate.
    /// @param      iFormat         The hash of the type.
    /// @param      iProfileTag     The name of the requested profile, leave default if unneeded.
    /// @return     A new allocated IBlock with specified format, width, height and profile.
    static  IBlock*  MakeBlock( int iWidth
                              , int iHeight
                              , uint32_t iFormat
                              , const std::string& iProfileTag = "default" );


    /// @fn         static  IBlock*  MakeBlockFromExternalData( int iWidth, int iHeight, uint8* iData, uint32_t iFormat, const std::string& iProfileTag = "default" )
    /// @brief      Make a block with dynamic type deduction at runtime, from existing raw data.
    /// @details    The block doesn't take ownership of the raw data, so the data shouldn't be deleted during the block lifetime.
    /// @param      iWidth          The width of the block to allocate.
    /// @param      iHeight         The height of the block to allocate.
    /// @param      iData           The external data to read from.
    /// @param      iFormat         The hash of the type.
    /// @param      iProfileTag     The name of the requested profile, leave default if unneeded.
    /// @return     A new allocated IBlock with specified format, width, height and profile, with the specified data.
    static  IBlock*  MakeBlockFromExternalData( int iWidth
                                              , int iHeight
                                              , uint8* iData
                                              , uint32_t iFormat
                                              , const std::string& iProfileTag = "default" );


    /// @fn         static  IBlock*  MakeBlockFromExternalDataTakeOwnership( int iWidth, int iHeight, uint8* iData, uint32_t iFormat, const std::string& iProfileTag = "default" )
    /// @brief      Make a block with dynamic type deduction at runtime, from existing raw data, and take ownership of the raw data pointer.
    /// @details    The block takes ownership of the raw data, so the data is living in the block now, the source pointer shouldn't be deleted.
    /// @param      iWidth          The width of the block to allocate.
    /// @param      iHeight         The height of the block to allocate.
    /// @param      iData           The external data to read from.
    /// @param      iFormat         The hash of the type.
    /// @param      iProfileTag     The name of the requested profile, leave default if unneeded.
    /// @return     A new allocated IBlock with specified format, width, height and profile, with the specified data.
    static  IBlock*  MakeBlockFromExternalDataTakeOwnership( int iWidth
                                                           , int iHeight
                                                           , uint8* iData
                                                           , uint32_t iFormat
                                                           , const std::string& iProfileTag = "default" );


    /// @fn         static  IBlock*  MakeBlockFromDataPerformCopy( int iWidth, int iHeight, uint8* iData, uint32_t iFormat, const std::string& iProfileTag = "default", const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    /// @brief      Make a block with dynamic type deduction at runtime, from existing raw data.
    /// @details    The block performs a full copy of the data, so the data can be safely destroyed afterwards.
    /// @param      iWidth                  The width of the block to allocate.
    /// @param      iHeight                 The height of the block to allocate.
    /// @param      iData                   The external data to copy from.
    /// @param      iFormat                 The hash of the type.
    /// @param      iProfileTag             The name of the requested profile, leave default if unneeded.
    /// @param      iPerformanceOptions     The performance preferences.
    /// @return     A new allocated IBlock with specified format, width, height and profile, with the specified data.
    static  IBlock*  MakeBlockFromDataPerformCopy( int iWidth
                                                 , int iHeight
                                                 , uint8* iData
                                                 , uint32_t iFormat
                                                 , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions()
                                                 , const std::string& iProfileTag = "default" );


    /// @fn         static  IBlock*  CopyBlock( IBlock* iBlock, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    /// @brief      Make a copy of the input block with same format, same layout, same profile.
    /// @details    A new block with same format is allocated, then a copy is performed.
    /// @param      iBlock                  The source.
    /// @param      iPerformanceOptions     The performance preferences.
    /// @return     A new allocated IBlock copied from the source.
    static  IBlock*  CopyBlock( IBlock* iBlock
                              , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() );


    /// @fn         static  void  CopyBlockInto( IBlock* iSrc, IBlock* iDst, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    /// @brief      Make a copy of the source block into the destination block.
    /// @details    A copy is performed into the destination block. Blocks should have the same format and size.
    /// @param      iSrc                    The source.
    /// @param      iDst                    The destination.
    /// @param      iPerformanceOptions     The performance preferences.
    static  void  CopyBlockInto( IBlock* iSrc
                               , IBlock* iDst
                               , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() );


    /// @fn         static  IBlock*  CopyBlockRect( IBlock* iBlock, const FRect& iRect, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    /// @brief      Make a copy of the input block rect with same format, same layout, same profile.
    /// @details    A new block with same format is allocated, then a copy is performed.
    /// @param      iBlock                  The source.
    /// @param      iRect                   The area to copy.
    /// @param      iPerformanceOptions     The performance preferences.
    /// @return     A new allocated IBlock copied from the source.
    static  IBlock*  CopyBlockRect( IBlock* iBlock
                                  , const FRect& iRect
                                  , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() );


    /// @fn         static  void  CopyBlockRectInto( IBlock* iSrc, IBlock* iDst, const FRect& iRect, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    /// @brief      Make a copy of the source block into the destination block, copying the specified rect only.
    /// @details    A copy is performed into the destination block.
    /// @param      iSrc                    The source.
    /// @param      iDst                    The destination.
    /// @param      iRect                   The area to copy.
    /// @param      iPerformanceOptions     The performance preferences.
    static  void  CopyBlockRectInto( IBlock* iSrc
                                   , IBlock* iDst
                                   , const FRect& iRect
                                   , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() );


    /// @fn         static  void  CopyBlockRectInto( IBlock* iSrc, IBlock* iDst, const FRect& iRect, const FPoint& iDstPos, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    /// @brief      Make a copy of the source block into the destination block, copying the specified rect only at specified pos.
    /// @details    A copy is performed into the destination block.
    /// @param      iSrc                    The source.
    /// @param      iDst                    The destination.
    /// @param      iSrcRect                The area to copy.
    /// @param      iDstPos                 The position to copy to.
    /// @param      iPerformanceOptions     The performance preferences.
    static  void  CopyBlockRectInto( IBlock* iSrc
                                   , IBlock* iDst
                                   , const FRect& iSrcRect
                                   , const FPoint& iDstPos
                                   , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() );


    /// @fn         static  FRect  GetTrimmedTransparencyRect( const IBlock* iSrc, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    /// @brief      Get the rect corresponding to the minimal rectangular area in the block that is non transparent.
    /// @details    THis effectively leaves empty spaces out.
    /// @param      iSrc                    The source.
    /// @param      iPerformanceOptions     The performance preferences.
    static  FRect  GetTrimmedTransparencyRect( const IBlock* iSrc
                                             , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() );

};

} // namespace ULIS

