// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.VectorSIMD.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the definition for the class SIMD wrapper classes and functions.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/ULIS.Config.h"
#include <immintrin.h>

namespace ULIS {
/////////////////////////////////////////////////////
/// @struct     FVectorSIMD128_8bit
/// @brief      [Deprecated] The FVectorSIMD128_8bit struct provides wrapper around SIMD SSSE intrinsics.
/// @details    The FVectorSIMD128_8bit class consists of a union member for holding the data, and some wrapper Getters / Setters and operators overload.
struct alignas( 16 ) FVectorSIMD128_8bit
{
public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------- Public Data Member
    /// @brief      Anonymous union member.
    /// @details    Members of the anonymous union can be accessed directly by their without naming the union.
    union {
        uint8   u8[16];     ///< Anonymous union member, representing an array of 16 uint8. The union is 128bits wide.
        uint16  u16[8];     ///< Anonymous union member, representing an array of 8 uint16. The union is 128bits wide.
        uint32  u32[4];     ///< Anonymous union member, representing an array of 4 uint32. The union is 128bits wide.
        __m128i m128i;      ///< Anonymous union member, representing a __m128i intrinsic data type. The union is 128bits wide.
    };


public:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------- Construction / Destruction
    /// @fn         FVectorSIMD128_8bit()
    /// @brief      Default Constructor.
    /// @details    Members remain uninitialized.
    ULIS_FORCEINLINE FVectorSIMD128_8bit()
    {}


public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------- Makers
    /// @fn         static FVectorSIMD128_8bit Zero()
    /// @brief      Maker, filling with zero.
    /// @details    Members are initialized and set to zero.
    /// @return     A FVectorSIMD128_8bit object filled with uint8 zeroes.
    ULIS_FORCEINLINE static FVectorSIMD128_8bit Zero() {
        FVectorSIMD128_8bit tmp;
        tmp.SetZero();
        return  tmp;
    }


    /// @fn         static FVectorSIMD128_8bit Max()
    /// @brief      Maker, filling with max-uint8 value( 255 ).
    /// @details    Members are initialized and set to 255.
    /// @return     A FVectorSIMD128_8bit object filled with uint8 255.
    ULIS_FORCEINLINE static FVectorSIMD128_8bit Max() {
        FVectorSIMD128_8bit tmp;
        tmp.Set1( 255 );
        return  tmp;
    }


public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Loaders / Setters
    /// @fn         Load( const uint8* iMem )
    /// @brief      Load uint8 data from memory.
    /// @details    Load with SIMD intrinsics from memory.
    /// @param      iMem    The memory to load, should be an array to uint8 at least 128 bits wide.
    ULIS_FORCEINLINE void Load( const uint8* iMem ) {
        m128i = _mm_loadu_si128( (const __m128i*)iMem );
    }


    /// @fn         SetZero()
    /// @brief      Store zeroes in the entire vector.
    /// @details    Using SIMD intrinsics.
    ULIS_FORCEINLINE void SetZero() {
        m128i = _mm_setzero_si128();
    }


    /// @fn         Set1( uint8 iValue )
    /// @brief      Store one value copied along the entire vector.
    /// @details    Using SIMD intrinsics.
    /// @param      iValue      the value to load and spread along the vector.
    ULIS_FORCEINLINE void Set1( uint8 iValue ) {
        m128i = _mm_set1_epi8( iValue );
    }


    /// @fn         Set2( uint8 e0, uint8 e1 )
    /// @brief      Store two values copied and spread along the entire vector.
    /// @details    Using SIMD intrinsics.
    ULIS_FORCEINLINE void Set2( uint8 e0, uint8 e1 ) {
        m128i = _mm_set_epi8( e1, e1, e1, e1, e1, e1, e1, e1, e0, e0, e0, e0, e0, e0, e0, e0 );
    }


    /// @fn         Set4( uint8 e0, uint8 e1, uint8 e2, uint8 e3 )
    /// @brief      Store four values copied and spread along the entire vector.
    /// @details    Using SIMD intrinsics.
    ULIS_FORCEINLINE void Set4( uint8 e0, uint8 e1, uint8 e2, uint8 e3 ) {
        m128i = _mm_set_epi8( e3, e3, e3, e3, e2, e2, e2, e2, e1, e1, e1, e1, e0, e0, e0, e0 );
    }


    /// @fn         Set8( uint8 e0, uint8 e1, uint8 e2, uint8 e3, uint8 e4, uint8 e5, uint8 e6, uint8 e7 )
    /// @brief      Store eight values copied and spread along the entire vector.
    /// @details    Using SIMD intrinsics.
    ULIS_FORCEINLINE void Set8( uint8 e0, uint8 e1, uint8 e2, uint8 e3, uint8 e4, uint8 e5, uint8 e6, uint8 e7 ) {
        m128i = _mm_set_epi8( e7, e7, e6, e6, e5, e5, e4, e4, e3, e3, e2, e2, e1, e1, e0, e0 );
    }


    /// @fn         Set16( uint8 e0, uint8 e1, uint8 e2,  uint8 e3,  uint8 e4,  uint8 e5,  uint8 e6,  uint8 e7, uint8 e8, uint8 e9, uint8 e10, uint8 e11, uint8 e12, uint8 e13, uint8 e14, uint8 e15 )
    /// @brief      Store sixteen values copied and spread along the entire vector.
    /// @details    Using SIMD intrinsics.
    ULIS_FORCEINLINE void Set16( uint8 e0, uint8 e1, uint8 e2,  uint8 e3,  uint8 e4,  uint8 e5,  uint8 e6,  uint8 e7
                    , uint8 e8, uint8 e9, uint8 e10, uint8 e11, uint8 e12, uint8 e13, uint8 e14, uint8 e15 ) {
        m128i = _mm_set_epi8( e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0 );
    }


public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------- Operators Implementation
    /// @fn         AdditionSafe( __m128i i )
    /// @brief      Add a __m128i intrinsic to this object using uint8 add, not checking for staturation ( you must ensure the addition doesn't overflow ).
    /// @details    Using SIMD intrinsics.
    /// @param      i       the value to add.
    /// @return     A FVectorSIMD128_8bit which is the result of the addition.
    ULIS_FORCEINLINE FVectorSIMD128_8bit AdditionSafe( __m128i i ) {
        FVectorSIMD128_8bit tmp;
        tmp.m128i = _mm_add_epi8( m128i, i );
        return  tmp;
    }


    /// @fn         AdditionSat( __m128i i )
    /// @brief      Add a __m128i intrinsic to this object using uint8 add, checking for staturation ( you do not need to ensure if the addition overflows ).
    /// @details    Using SIMD intrinsics.
    /// @param      i       the value to add.
    /// @return     A FVectorSIMD128_8bit which is the result of the addition.
    ULIS_FORCEINLINE FVectorSIMD128_8bit AdditionSat( __m128i i )  {
        FVectorSIMD128_8bit tmp;
        tmp.m128i = _mm_adds_epu8( m128i, i );
        return  tmp;
    }


    /// @fn         SubtractionSafe( __m128i i )
    /// @brief      Substracts a __m128i intrinsic to this object using uint8 substraction, not checking for staturation ( you must ensure the substraction doesn't overflow ).
    /// @details    Using SIMD intrinsics.
    /// @param      i       the value to substract.
    /// @return     A FVectorSIMD128_8bit which is the result of the substraction.
    ULIS_FORCEINLINE FVectorSIMD128_8bit SubtractionSafe( __m128i i ) {
        FVectorSIMD128_8bit tmp;
        tmp.m128i = _mm_sub_epi8( m128i, i );
        return  tmp;
    }


    /// @fn         SubtractionSat( __m128i i )
    /// @brief      Substracts a __m128i intrinsic to this object using uint8 substraction, checking for staturation ( you do not need to ensure if the substraction overflows ).
    /// @details    Using SIMD intrinsics.
    /// @param      i       the value to substract.
    /// @return     A FVectorSIMD128_8bit which is the result of the substraction.
    ULIS_FORCEINLINE FVectorSIMD128_8bit SubtractionSat( __m128i i )  {
        FVectorSIMD128_8bit tmp;
        tmp.m128i = _mm_subs_epu8( m128i, i );
        return  tmp;
    }


public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------ Operators Overloads
    /// @fn         operator+( uint8 i )
    /// @brief      Adds one uint8 i spread across the entire vector.
    /// @details    Using \e AdditionSat implementation.
    /// @param      i       the value to add.
    /// @return     A FVectorSIMD128_8bit which is the result of the addition.
    ULIS_FORCEINLINE FVectorSIMD128_8bit operator+( uint8 i ) { return  AdditionSat( _mm_set1_epi8( i ) ); }


    /// @fn         operator-( uint8 i )
    /// @brief      Substracts one uint8 i spread across the entire vector.
    /// @details    Using \e SubstractionSat implementation.
    /// @param      i       the value to substract.
    /// @return     A FVectorSIMD128_8bit which is the result of the substraction.
    ULIS_FORCEINLINE FVectorSIMD128_8bit operator-( uint8 i ) { return  SubtractionSat( _mm_set1_epi8( i ) ); }


    /// @fn         operator+( __m128i i )
    /// @brief      Adds one __m128i vector.
    /// @details    Using \e AdditionSat implementation.
    /// @param      i       the value of data type __m128i vector to add.
    /// @return     A FVectorSIMD128_8bit which is the result of the addition.
    ULIS_FORCEINLINE FVectorSIMD128_8bit operator+( __m128i i ) { return  AdditionSat( i ); }


    /// @fn         operator-( __m128i i )
    /// @brief      Substracts one __m128i vector.
    /// @details    Using \e SubstractionSat implementation.
    /// @param      i       the value of data type __m128i vector to substract.
    /// @return     A FVectorSIMD128_8bit which is the result of the substraction.
    ULIS_FORCEINLINE FVectorSIMD128_8bit operator-( __m128i i ) { return  SubtractionSat( i ); }


    /// @fn         operator+( FVectorSIMD128_8bit i )
    /// @brief      Adds one FVectorSIMD128_8bit.
    /// @details    Using \e AdditionSat implementation.
    /// @param      i       the value of type FVectorSIMD128_8bit to add.
    /// @return     A FVectorSIMD128_8bit which is the result of the addition.
    ULIS_FORCEINLINE FVectorSIMD128_8bit operator+( FVectorSIMD128_8bit i ) { return  AdditionSat( i.m128i ); }


    /// @fn         operator-( FVectorSIMD128_8bit i )
    /// @brief      Substracts one FVectorSIMD128_8bit.
    /// @details    Using \e SubstractionSat implementation.
    /// @param      i       the value of type FVectorSIMD128_8bit to substract.
    /// @return     A FVectorSIMD128_8bit which is the result of the substraction.
    ULIS_FORCEINLINE FVectorSIMD128_8bit operator-( FVectorSIMD128_8bit i ) { return  SubtractionSat( i.m128i ); }


};


/////////////////////////////////////////////////////
/// @struct     FVectorSIMD128_Dual8bit
/// @brief      [Deprecated] The FVectorSIMD128_Dual8bit struct provides wrapper around two FVectorSIMD128_8bit.
/// @details    The FVectorSIMD128_Dual8bit is used for intermediate results when multiplying or adding multiple FVectorSIMD128_8bit.
struct alignas( 16 ) FVectorSIMD128_Dual8bit
{
public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------- Public Data Member
    FVectorSIMD128_8bit lo;     ///< The low 128 bits as a FVectorSIMD128_8bit object.
    FVectorSIMD128_8bit hi;     ///< The high 128 bits as a FVectorSIMD128_8bit object.


public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------ Operators Overloads
    /// @fn         operator+( FVectorSIMD128_Dual8bit i )
    /// @brief      Adds one FVectorSIMD128_Dual8bit.
    /// @details    Using \e saturation, it adds both underlying FVectorSIMD128_8bit objects respectively.
    /// @param      i       the value of type FVectorSIMD128_Dual8bit to add.
    /// @return     A FVectorSIMD128_Dual8bit which is the result of the addition.
    ULIS_FORCEINLINE FVectorSIMD128_Dual8bit operator+( FVectorSIMD128_Dual8bit i ) {
        FVectorSIMD128_Dual8bit tmp;
        tmp.lo.m128i = _mm_adds_epu16( lo.m128i, i.lo.m128i );
        tmp.hi.m128i = _mm_adds_epu16( hi.m128i, i.hi.m128i );
        return  tmp;
    }


    /// @fn         operator-( FVectorSIMD128_Dual8bit i )
    /// @brief      Substracts one FVectorSIMD128_Dual8bit.
    /// @details    Using \e saturation, it substracts both underlying FVectorSIMD128_8bit objects respectively.
    /// @param      i       the value of type FVectorSIMD128_Dual8bit to substract.
    /// @return     A FVectorSIMD128_Dual8bit which is the result of the substraction.
    ULIS_FORCEINLINE FVectorSIMD128_Dual8bit operator-( FVectorSIMD128_Dual8bit i ) {
        FVectorSIMD128_Dual8bit tmp;
        tmp.lo.m128i = _mm_subs_epu16( lo.m128i, i.lo.m128i );
        tmp.hi.m128i = _mm_subs_epu16( hi.m128i, i.hi.m128i );
        return  tmp;
    }


};


namespace SIMD {
/////////////////////////////////////////////////////
/// @fn         Spread( const FVectorSIMD128_8bit& i )
/// @brief      Spreads a FVectorSIMD128_8bit in a FVectorSIMD128_Dual8bit.
/// @details    This will interleave the input uint8 values with zeroes.
/// @param      i   The input FVectorSIMD128_8bit object.
/// @return     A FVectorSIMD128_Dual8bit object with the input values interleaved with zeroes.
ULIS_FORCEINLINE FVectorSIMD128_Dual8bit Spread( const FVectorSIMD128_8bit& i )
{
    FVectorSIMD128_Dual8bit tmp;
    __m128i zero    = _mm_setzero_si128();
    tmp.lo.m128i = _mm_unpacklo_epi8( i.m128i, zero);
    tmp.hi.m128i = _mm_unpackhi_epi8( i.m128i, zero);
    return  tmp;
}


/// @fn         Contract( const FVectorSIMD128_Dual8bit& i )
/// @brief      Contracts a FVectorSIMD128_Dual8bit into a FVectorSIMD128_8bit.
/// @details    This will sample interleaved values to combine them into the result.
/// @param      i   The input FVectorSIMD128_Dual8bit object.
/// @return     A FVectorSIMD128_8bit object built from the interleaved inpute values.
ULIS_FORCEINLINE FVectorSIMD128_8bit Contract( const FVectorSIMD128_Dual8bit& i )
{
    FVectorSIMD128_8bit tmp;
    __m128i zero    = _mm_setzero_si128();
    __m128i maskLo  = _mm_set_epi8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 14, 12, 10, 8, 6, 4, 2, 0);
    __m128i maskHi  = _mm_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    tmp.m128i       = _mm_or_si128(_mm_shuffle_epi8(i.lo.m128i, maskLo), _mm_shuffle_epi8(i.hi.m128i, maskHi));
    return  tmp;
}


/// @fn         UpScale( const FVectorSIMD128_8bit& i )
/// @brief      Upscales a FVectorSIMD128_8bit to a FVectorSIMD128_Dual8bit
/// @details    This will multiply the 8bits values of the input object with 255, leading to two 16bits vectors.
/// @param      i   The input FVectorSIMD128_8bit object to upscale.
/// @return     A FVectorSIMD128_Dual8bit object representing two 16bits vectors.
ULIS_FORCEINLINE FVectorSIMD128_Dual8bit UpScale( const FVectorSIMD128_8bit& i )
{
    FVectorSIMD128_Dual8bit tmp;
    __m128i zero    = _mm_setzero_si128();
    __m128i factor  = _mm_set1_epi16( 0x101 );
    tmp.lo.m128i = _mm_mullo_epi16( _mm_unpacklo_epi8( i.m128i, zero), factor );
    tmp.hi.m128i = _mm_mullo_epi16( _mm_unpackhi_epi8( i.m128i, zero), factor );
    return  tmp;
}


/// @fn         DownScale( const FVectorSIMD128_Dual8bit& i )
/// @brief      Downscales a FVectorSIMD128_Dual8bit to a FVectorSIMD128_8bit
/// @details    This will shift down the 16bits values of the input object to 8bits, leading to one 8bit vector.
/// @param      i   The input FVectorSIMD128_Dual8bit object to downscale.
/// @return     A FVectorSIMD128_8bit object representing one 8bits vector.
ULIS_FORCEINLINE FVectorSIMD128_8bit DownScale( const FVectorSIMD128_Dual8bit& i )
{
    FVectorSIMD128_8bit tmp;
    __m128i ones = _mm_set1_epi16( 1 );
    __m128i lo = _mm_srli_epi16( _mm_adds_epu16( _mm_adds_epu16( i.lo.m128i, ones ), _mm_srli_epi16( i.lo.m128i, 8 ) ), 8 );
    __m128i hi = _mm_srli_epi16( _mm_adds_epu16( _mm_adds_epu16( i.hi.m128i, ones ), _mm_srli_epi16( i.hi.m128i, 8 ) ), 8 );
    __m128i maskLo  = _mm_set_epi8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 14, 12, 10, 8, 6, 4, 2, 0);
    __m128i maskHi  = _mm_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    tmp.m128i = _mm_or_si128(_mm_shuffle_epi8(lo, maskLo), _mm_shuffle_epi8(hi, maskHi));
    return  tmp;
}


/// @fn         FVectorSIMD128_Dual8bit operator*( const FVectorSIMD128_8bit& iLhs, const FVectorSIMD128_8bit& iRhs )
/// @brief      Multiplies two FVectorSIMD128_8bit together.
/// @details    This will spread the 8bits to 16bits interleaved values, then perform 16bits multiplication, and pack the result in a FVectorSIMD128_Dual8bit.
/// @param      iLhs    The left hand side input FVectorSIMD128_8bit object to multiply.
/// @param      iRhs    The right hand side input FVectorSIMD128_8bit object to multiply.
/// @return     A FVectorSIMD128_Dual8bit object representing the 16bits vector result of the multiplication.
ULIS_FORCEINLINE FVectorSIMD128_Dual8bit operator*( const FVectorSIMD128_8bit& iLhs, const FVectorSIMD128_8bit& iRhs )
{
    FVectorSIMD128_Dual8bit tmp;
    __m128i zero = _mm_setzero_si128();
    tmp.lo.m128i = _mm_mullo_epi16(_mm_unpacklo_epi8( iLhs.m128i, zero), _mm_unpacklo_epi8( iRhs.m128i, zero));
    tmp.hi.m128i = _mm_mullo_epi16(_mm_unpackhi_epi8( iLhs.m128i, zero), _mm_unpackhi_epi8( iRhs.m128i, zero));
    return  tmp;
}


} // namespace SIMD
} // namespace ULIS

