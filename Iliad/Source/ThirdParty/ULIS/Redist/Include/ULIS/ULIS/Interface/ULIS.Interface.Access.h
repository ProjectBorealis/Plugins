// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Access.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the definition for the classes PixelValueAutoCastChecked and PixelProxyAutoCastChecked
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Data/ULIS.Data.Block.h"

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      PixelValueAutoCastChecked
/// @brief      The PixelValueAutoCastChecked class provides a mean of auto-casting PixelValue.
/// @details    By overloading the call operator(), a PixelValue can be retrieved automatically without explicit cast from the caller.
///             The cast is checked with an assert which will fail if you try to assign the wrong type.
class PixelValueAutoCastChecked
{
private:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------- Private Data Members
    const IBlock* mBlock;   ///< Pointer to the IBlock needed to cast.
    int mX;                 ///< The X coordinate to get the value from.
    int mY;                 ///< The Y coordinate to get the value from.


public:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------- Construction / Destruction
    /// @fn         PixelValueAutoCastChecked( const IBlock* iBlock, int iX, int iY )
    /// @brief      Constructor from block and coordinates.
    /// @details    Initializes all members with the supplied values.
    /// @param      iBlock      The block to cast from.
    /// @param      iX          The X coordinate.
    /// @param      iY          The Y coordinate.
    PixelValueAutoCastChecked( const IBlock* iBlock, int iX, int iY )
        : mBlock( iBlock )
        , mX( iX )
        , mY( iY )
    {}


public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Operator Overload
    /// @fn         operator T()
    /// @brief      Call operator overload.
    /// @return     The value cast to the caller appropriate type ( likely in an assignment operation ).
    template< typename T > operator T()
    {
        assert( mBlock->Id() == T::TypeId() );
        return  ((::ULIS::TBlock< T::TypeId() >*)mBlock)->PixelValue( mX, mY );
    }

};


/////////////////////////////////////////////////////
/// @class      PixelProxyAutoCastChecked
/// @brief      The PixelProxyAutoCastChecked class provides a mean of auto-casting PixelProxy.
/// @details    By overloading the call operator(), a PixelProxy can be retrieved automatically without explicit cast from the caller.
///             The cast is checked with an assert which will fail if you try to assign the wrong type.
class PixelProxyAutoCastChecked
{
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------- Private Data Members
    const IBlock* mBlock;   ///< Pointer to the IBlock needed to cast.
    int mX;                 ///< The X coordinate to get the value from.
    int mY;                 ///< The Y coordinate to get the value from.


public:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------- Construction / Destruction
    /// @fn         PixelProxyAutoCastChecked( const IBlock* iBlock, int iX, int iY )
    /// @brief      Constructor from block and coordinates.
    /// @details    Initializes all members with the supplied values.
    /// @param      iBlock      The block to cast from.
    /// @param      iX          The X coordinate.
    /// @param      iY          The Y coordinate.
    PixelProxyAutoCastChecked( const IBlock* iBlock, int iX, int iY )
        : mBlock( iBlock )
        , mX( iX )
        , mY( iY )
    {}


public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Operator Overload
    /// @fn         operator T()
    /// @brief      Call operator overload.
    /// @return     The value cast to the caller appropriate type ( likely in an assignment operation ).
    template< typename T > operator T()
    {
        assert( mBlock->Id() == T::TypeId() );
        return  ((::ULIS::TBlock< T::TypeId() >*)mBlock)->PixelProxy( mX, mY );
    }

};

} // namespace ULIS

