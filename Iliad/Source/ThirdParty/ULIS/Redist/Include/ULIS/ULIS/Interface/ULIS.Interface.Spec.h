// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Spec.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FSpecContext class.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Spec/ULIS.Spec.Spec.h"

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FSpecContext
/// @brief      The FSpecContext class provides a context for gathering informations on block specifications at runtime.
class FSpecContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         static  FSpec  BlockSpecAtIndex( uint32 iIndex )
    /// @brief      Query block specification object.
    /// @detail     Get block FSpec specification object at index iIndex from registry.
    /// @param      iIndex      The index to gather from the type registry.
    /// @return     FSpec object representing the specification information for a given block type.
    static  FSpec  BlockSpecAtIndex( uint32 iIndex );


    /// @fn         static  FSpec  BlockSpecFromHash( uint32 iHash )
    /// @brief      Query block specification object.
    /// @detail     Get block FSpec specification object with the provided hash. If the hash doesn't match any existing type, a default FSpec is returned.
    /// @param      iHash       The hash to query.
    /// @return     FSpec object representing the specification information for a given block type.
    static  FSpec  BlockSpecFromHash( uint32 iHash );


    /// @fn         static  void  PrintSpecs()
    /// @brief      Print all specs for all existing block types to std out.
    static  void  PrintSpecs();


    /// @fn         static  void  PrintSpecs()
    /// @brief      Print short specs ( hash and string ) for all existing block types to std out.
    static  void  PrintShortSpecs();

};

} // namespace ULIS

