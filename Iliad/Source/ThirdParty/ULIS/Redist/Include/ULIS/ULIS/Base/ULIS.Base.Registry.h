// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.Registry.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the definition for the TRegistry struct.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <utility>

namespace ULIS {
/////////////////////////////////////////////////////
/// @struct     TRegistry
/// @brief      The TRegistry struct provides a compile-time uint32 index registry.
/// @details    It is acts like a vector of uint32, meant to hold CRC32 hashes, providing some kind of type registry and soft runtime type analysis for blocks.
template< int _S >
struct TRegistry
{
public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------ Public Data Members
    const uint32 array[_S] = { 0 };   ///< The actual array member holding the uint32 CRC32 hashes.


public:
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Public API
    /// @fn         operator[]( int i )  const
    /// @brief      Overload of operator[] for accessing the elements of the underlying array.
    /// @details    No range checking is done, which can lead to errors, always check with \e Size() first, and don't input negative numbers.
    /// @return     An uint32 representing the value at the specified index.
    constexpr  uint32  operator[]  ( int iIndex )  const  { return  array[ iIndex ]; }


    /// @fn         Size()  const
    /// @brief      Query the size.
    /// @return     An int representing the size of the underlying array
    constexpr  int  Size()  const  { return  _S; }


    /// @fn         TRegistry< _S + 1 >  PushBack( uint32 iValue )  const
    /// @brief      Push a value at the end of the underlying array.
    /// @details    PushBack does not reallocate or grow memory since this is happening at compile time, instead it creates a copy of the same static storage,
    ///             grown with the new value. See \e PushBack_imp for details on the implementation.
    /// @return     A new TRegistry object grown with the new value pushed at the end.
    constexpr  TRegistry< _S + 1 >  PushBack( uint32 iValue )  const  { return PushBack_imp< _S + 1 >( iValue, std::make_integer_sequence< int, _S >() ); }


private:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- Private API
    /// @fn         PushBack_imp( uint32 i, std::integer_sequence< T, Nums... > )  const
    /// @brief      Template implementation for PushBack
    /// @details    PushBack_imp does not reallocate or grow memory since this is happening at compile time, instead it creates a copy of the same static
    ///             storage, grown with the new value. Implementation is done using a trick with parameter pack expansion and initializer-list-like construction.
    /// @return     A new TRegistry object grown with the new value pushed at the end.
    template< int _N
            , typename T
            , T... Nums >
    constexpr
    TRegistry< _N >
    PushBack_imp( uint32 i, std::integer_sequence< T, Nums... > )  const {
        return { array[Nums] ..., i };
    }

};


/////////////////////////////////////////////////////
// Utilities for registry manipulations
#define ULIS_PREVIOUS( i )                      BOOST_PP_SUB( i, 1 )
#define ULIS_CAT( iLhs, iRhs )                  BOOST_PP_CAT( iLhs, iRhs )
#define ULIS_CREATE_REG( iName, i )             namespace generated { constexpr ::ULIS::TRegistry< 1 > ULIS_CAT( iName,  __COUNTER__ ) = { i }; }
#define ULIS_APPEND_REG_IMP( iName, i, c )      namespace generated { constexpr auto ULIS_CAT( iName,  c ) = ULIS_CAT( iName, ULIS_PREVIOUS( c ) ).PushBack( i ); }
#define ULIS_APPEND_REG( iName, i )             ULIS_APPEND_REG_IMP( iName, i, __COUNTER__ )
#define ULIS_ASSIGN_REG( iName )                generated:: ULIS_CAT( iName, ULIS_PREVIOUS( __COUNTER__ ) )
#define ULIS_REG_TYPE                           constexpr auto
#define ULIS_REPEAT                             BOOST_PP_REPEAT

} // namespace ULIS

