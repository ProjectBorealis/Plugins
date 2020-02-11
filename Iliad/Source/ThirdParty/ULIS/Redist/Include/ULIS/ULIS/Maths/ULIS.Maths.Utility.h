// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Maths.Utility.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include <cmath>
#include <glm/vec2.hpp>


namespace ULIS {
/////////////////////////////////////////////////////
// FMath
struct FMath
{
    // Do not change this value !
    static constexpr float kEpsilonf = .000001;
    static constexpr double kPId = 3.1415926535897932;
    static constexpr float  kPIf = 3.14159265;

    template< typename T >
    static inline T Min( T iA, T iB ) {
        return  iA < iB ? iA : iB;
    }

    template< typename T >
    static inline T Max( T iA, T iB ) {
        return  iA > iB ? iA : iB;
    }

    template< typename T >
    static inline T Min3( T iA, T iB, T iC ) {
        return  Min( iA, Min( iB, iC ) );
    }

    template< typename T >
    static inline T Max3( T iA, T iB, T iC ) {
        return  Max( iA, Max( iB, iC ) );
    }

    template< typename T >
    static inline T Clamp( T iValue, T iMin, T iMax ) {
        return  Max( iMin, Min( iValue, iMax ) );
    }

    template< typename T >
    static inline T Abs( T iA ) {
        return ( iA < 0 ? -iA : iA );
    }

    template< typename T >
    static inline T Sign( T iValue ) {
        return  iValue < T(0) ? T(-1) : T(1);
    }

    static inline double RadToDeg( double iRad )
    {
        return ( iRad * 180 / FMath::kPId );
    }

    static inline double DegToRad( double iDeg )
    {
        return ( iDeg * FMath::kPId / 180 );
    }

    static inline int IntegerPartOfNumber( float iNumber )
    {
        return (int)iNumber;
    }

    float FloatingPartOfNumber( float iNumber )
    {
        if ( iNumber > 0 )
            return iNumber - IntegerPartOfNumber( iNumber );
        else
            return iNumber - ( IntegerPartOfNumber( iNumber ) + 1 );
    }

    static inline int RoundNumber( float iNumber )
    {
        return  (int)( iNumber + 0.5f );
    }

    template< typename T >
    static  inline T Ceil( T iValue )
    {
        return  ceil( iValue );
    }

    template< typename T >
    static  inline T Floor( T iValue )
    {
        return  floor( iValue );
    }

    template< typename T >
    static  inline T RoundAwayFromZero( T iValue )
    {
        return  iValue < 0 ? (T)floor( iValue ) : (T)ceil( iValue );
    }

    template< typename T >
    static  inline T RoundTowardsZero( T iValue )
    {
        return  iValue < 0 ? (T)ceil( iValue ) : (T)floor( iValue );
    }

    static inline bool EpsilonComp( float iA, float iB )
    {
        return  Abs( iA - iB ) <= kEpsilonf;
    }

}; // struct FMath


} // namespace ULIS

