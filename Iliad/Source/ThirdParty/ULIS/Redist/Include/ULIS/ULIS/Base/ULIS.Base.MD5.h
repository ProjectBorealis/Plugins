// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.MD5.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declaration for the MD5 class.
 */

#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include <string>

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FMD5
/// @brief      The FMD5 class provides a mean for computation of MD5 hash.
/// @details    Based on reference implementation of RFC 1321.
class FMD5
{
public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------- Construction
    /// @fn         FMD5()
    /// @brief      Default Constructor.
    /// @details    Calls \e Init, ready to \e Update.
    FMD5();


    /// @fn         FMD5( const std::string& iText )
    /// @brief      Constructor from std::string.
    /// @details    Calls \e Init and computes hash directly, ready to \e Hexdiget.
    /// @param      iText   The text to process.
    FMD5( const std::string& iText );


    /// @fn         FMD5( const uint8* iData, uint32 iLen )
    /// @brief      Constructor from data and length.
    /// @details    Calls \e Init and computes hash directly, ready to \e Hexdiget.
    /// @param      iData   The data to process.
    /// @param      iLen    The length as bytes to process.
    FMD5( const uint8* iData, uint32 iLen );


private:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------- Private API
    /// @fn         Init()
    /// @brief      Initializes the state of the MD5 computer.
    void  Init();


    /// @fn         Transform( const uint8 block[64] )
    /// @brief      The main deterministic transform process.
    /// @details    Called from \e Update.
    void  Transform( const uint8 block[64] );


public:
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Public API
    /// @fn         Update( const uint8 iBuf[], uint32 iLen )
    /// @brief      Update the data incrementally.
    /// @details    This can be called for the whole data at once, or as split batches, the resulting hash will be the same.
    /// @param      iBuf    The buffer to process.
    /// @param      iLen    The length as bytes of the buffer.
    void  Update( const uint8 iBuf[], uint32 iLen );


    /// @fn         Finalize()
    /// @brief      Finalize the hashing of the data.
    /// @details    This should be called after passing all the buffers to Update.
    /// @return     A reference to this finalized FMD5 object;
    FMD5&  Finalize();


    /// @fn         Hexdigest() const
    /// @brief      Query the Hex Digest as std::string.
    /// @return     A std::string representing the Hex hash.
    std::string  Hexdigest() const;

private:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------- Private Static Tools
    static  void  Decode( uint32 oOutput[], const uint8 iInput[], uint32 iLen );
    static  void  Encode( uint8 oOutput[], const uint32 iInput[], uint32 iLen );
    static  inline uint32 F( uint32 x, uint32 y, uint32 z );
    static  inline uint32 G( uint32 x, uint32 y, uint32 z );
    static  inline uint32 H( uint32 x, uint32 y, uint32 z  );
    static  inline uint32 I( uint32 x, uint32 y, uint32 z );
    static  inline uint32 Rotate( uint32 x, int n );
    static  inline void FF( uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac );
    static  inline void GG( uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac );
    static  inline void HH( uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac );
    static  inline void II( uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac );

private:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------- Private Data
    bool    mFinalized;
    uint8   mBuffer[64];
    uint32  mCount[2];
    uint32  mState[4];
    uint8   mDigest[16];
};

/////////////////////////////////////////////////////
// Accessibility utility functions
/// @fn         MD5( const std::string iStr )
/// @brief      Shorthand function, computes MD5 for std::string directly
/// @param      iStr    The text to hash.
/// @return     A std::string representing the MD5 hash.
std::string  MD5( const std::string iStr );


/// @fn         MD5( const uint8* iData, int iLen )
/// @brief      Shorthand function, computes MD5 for std::string directly
/// @param      iData   The data to hash.
/// @param      iLen    The size in bytes of the data to hash.
/// @return     A std::string representing the MD5 hash.
std::string  MD5( const uint8* iData, int iLen );

} // namespace ULIS

