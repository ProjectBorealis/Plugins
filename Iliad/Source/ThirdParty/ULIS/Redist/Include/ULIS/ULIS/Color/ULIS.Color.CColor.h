// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Color.CColor.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Color/ULIS.Color.ModelSupport.h"

namespace ULIS {

/////////////////////////////////////////////////////
// CColor
class  CColor
{
public:
    // Construction
    CColor();
    CColor( int r, int g, int b, int alpha = 255 );
    CColor( eCColorModel );
    CColor( uint32 rgbHexValue );

public:
    // Comparison Operators
    bool  operator==( const  CColor& Other )  const;
    bool  operator!=( const  CColor& Other )  const;

public:
    // Instance Conversion API
    CColor  ToGrey()  const;
    CColor  ToRGB()  const;
    CColor  ToHSL()  const;
    CColor  ToHSV()  const;
    CColor  ToCMYK()  const;
    CColor  ToModel( eCColorModel )  const;

public:
    // Static Makers API
    static  CColor  FromRGBHexValue( uint32 rgbHexValue, int alpha = 255 );
    static  CColor  FromGrey( int g, int alpha = 255 );
    static  CColor  FromRGB( int r, int g, int b, int alpha = 255 );
    static  CColor  FromHSL( int h, int s, int l, int alpha = 255 );
    static  CColor  FromHSV( int h, int s, int v, int alpha = 255 );
    static  CColor  FromCMYK( int c, int m, int y, int k, int alpha = 255 );

    static  CColor  FromGreyF( float f, float alpha = 1.f );
    static  CColor  FromRGBF( float r, float g, float b, float alpha = 1.f );
    static  CColor  FromHSLF( float h, float s, float l, float alpha = 1.f );
    static  CColor  FromHSVF( float h, float s, float v, float alpha = 1.f );
    static  CColor  FromCMYKF( float c, float m, float y, float k, float alpha = 1.f );

    static bool IsSimilar( const CColor& A, const CColor& B, int threshold = 1 );

public:
    // Instance Representation Access API, Getters
    eCColorModel    Mode()  const  { return  mMode; }
    int             Alpha()             const;
    float           AlphaF()            const;

    // Hex
    uint32  RGBHexValue()       const;

    // Grey
    int     Grey()              const;
    float   GreyF()           const;

    // RGB
    int     Red()               const;
    int     Green()             const;
    int     Blue()              const;
    float   RedF()              const;
    float   GreenF()            const;
    float   BlueF()             const;

    // HSV Alias
    int     Hue()               const;
    int     Saturation()        const;
    float   HueF()              const;
    float   SaturationF()       const;

    // HSL
    int     HSLHue()            const;
    int     HSLSaturation()     const;
    int     Lightness()         const;
    float   HSLHueF()           const;
    float   HSLSaturationF()    const;
    float   LightnessF()        const;

    // HSV
    int     HSVHue()            const;
    int     HSVSaturation()     const;
    int     Value()             const;
    float   HSVHueF()           const;
    float   HSVSaturationF()    const;
    float   ValueF()            const;

    // CMYK
    int     Cyan()              const;
    int     Magenta()           const;
    int     Yellow()            const;
    int     Key()               const;
    int     Black()             const;
    float   CyanF()             const;
    float   MagentaF()          const;
    float   YellowF()           const;
    float   KeyF()              const;
    float   BlackF()            const;

public:
    // Instance Representation Access API, Setters
    void  SetAlpha( int );
    void  SetAlphaF( float );

    void  SetGrey( int );
    void  SetGreyF( float );

    void  SetRed( int );
    void  SetGreen( int );
    void  SetBlue( int );
    void  SetRedF( float );
    void  SetGreenF( float );
    void  SetBlueF( float );

    void  SetGrey( int g, int alpha = 255 );
    void  SetGreyF( float g, float alpha = 1.f );
    void  SetRGBHexValue( uint32 rgbHexValue, int alpha = 255 );
    void  SetRGB( int r, int g, int b, int alpha = 255 );
    void  SetHSL( int h, int s, int l, int alpha = 255 );
    void  SetHSV( int h, int s, int v, int alpha = 255 );
    void  SetCMYK( int c, int m, int y, int k, int alpha = 255 );
    void  SetRGBF( float r, float g, float b, float alpha = 1.f );
    void  SetHSLF( float h, float s, float l, float alpha = 1.f );
    void  SetHSVF( float h, float s, float v, float alpha = 1.f );
    void  SetCMYKF( float c, float m, float y, float k, float alpha = 1.f );

private:
    // Private Data Members
    eCColorModel  mMode;
    union { // Union with pad '_', internal repr
        struct { uint16 alpha, g, _0, _1, _; } grey;
        struct { uint16 alpha, r, g, b, _; } rgb;
        struct { uint16 alpha, h, s, l, _; } hsl;
        struct { uint16 alpha, h, s, v, _; } hsv;
        struct { uint16 alpha, c, m, y, k; } cmyk;
        uint16 array[5];
    } mRepr;
};

} // namespace ULIS
