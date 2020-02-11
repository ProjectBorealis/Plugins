// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Global.GlobalPalettes.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Color/ULIS.Color.CColor.h"


namespace ULIS {
/////////////////////////////////////////////////////
// Global Color Palettes
namespace PaletteMSWindows {
/////////////////////////////////////////////////////
// eMSWindows
/*
static const CColor white       ( 255, 255, 255 );
static const CColor black       ( 0,   0,   0   );
static const CColor red         ( 255, 0,   0   );
static const CColor darkRed     ( 128, 0,   0   );
static const CColor green       ( 0,   255, 0   );
static const CColor darkGreen   ( 0,   128, 0   );
static const CColor blue        ( 0,   0,   255 );
static const CColor darkBlue    ( 0,   0,   128 );
static const CColor cyan        ( 0,   255, 255 );
static const CColor darkCyan    ( 0,   128, 128 );
static const CColor magenta     ( 255, 0,   255 );
static const CColor darkMagenta ( 128, 0,   128 );
static const CColor yellow      ( 255, 255, 0   );
static const CColor darkYellow  ( 128, 128, 0   );
static const CColor gray        ( 160, 160, 164 );
static const CColor darkGray    ( 128, 128, 128 );
static const CColor lightGray   ( 192, 192, 192 );
*/
} // namespace PaletteMSWindows
namespace PaletteMSPaintDebug {
/////////////////////////////////////////////////////
// PaletteMSPaintDebug 
static const CColor colors[] = {
    CColor( 237,  28,  36 ),
    CColor( 255, 127,  39 ),
    CColor( 255, 242,   0 ),
    CColor(  34, 177,  76 ),
    CColor(   0, 162, 232 ),
    CColor(  63,  72, 204 ),
    CColor( 163,  73, 164 ),
    CColor( 255, 174, 201 ),
    CColor( 255, 201,  14 ),
    CColor( 239, 228, 176 ),
    CColor( 181, 230,  29 ),
    CColor( 153, 217, 234 ),
    CColor( 112, 146, 190 ),
    CColor( 200, 191, 231 ),
}; // colors[]
static constexpr int length = sizeof( colors ) / sizeof( CColor );
} // namespace PaletteMSPaintDebug
} // namespace ULIS

