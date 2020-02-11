// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Color.Illuminants.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once
#include "ULIS/ULIS.Config.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include "lcms2.h"

namespace ULIS {
namespace Chroma {
/////////////////////////////////////////////////////
// Precomputed Chromaticity coordinates pairs
// Chromaticity coordinates CIE1931 2�, whitepoint
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_A(     0.44757f, 0.40745f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_B(     0.34842f, 0.35161f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_C(     0.31006f, 0.31616f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_D50(   0.34567f, 0.35850f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_D55(   0.33242f, 0.34743f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_D65(   0.31271f, 0.32902f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_D75(   0.29902f, 0.31485f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_E(     1/3.f,    1/3.f     );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F1(    0.31310f, 0.33727f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F2(    0.37208f, 0.37529f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F3(    0.40910f, 0.39430f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F4(    0.44018f, 0.40329f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F5(    0.31379f, 0.34531f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F6(    0.37790f, 0.38835f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F7(    0.31292f, 0.32933f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F8(    0.34588f, 0.35875f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F9(    0.37417f, 0.37281f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F10(   0.34609f, 0.35986f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F11(   0.38052f, 0.37713f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1931_2_F12(   0.43695f, 0.40441f  );
// Chromaticity coordinates CIE1964 10�, whitepoint
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_A(    0.45117f, 0.40594f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_B(    0.34980f, 0.35270f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_C(    0.31039f, 0.31905f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_D50(  0.34773f, 0.35952f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_D55(  0.33411f, 0.34877f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_D65(  0.31382f, 0.33100f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_D75(  0.29968f, 0.31740f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_E(    1/3.f,     1/3.f    );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F1(   0.31811f, 0.33559f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F2(   0.37925f, 0.36733f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F3(   0.41761f, 0.38324f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F4(   0.44920f, 0.39074f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F5(   0.31975f, 0.34246f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F6(   0.38660f, 0.37847f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F7(   0.31569f, 0.32960f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F8(   0.34902f, 0.35939f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F9(   0.37829f, 0.37045f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F10(  0.35090f, 0.35444f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F11(  0.38541f, 0.37123f  );
static const glm::vec2 standardIlluminant_chromaticityCoordinates_CIE_1964_10_F12(  0.44256f, 0.39717f  );
/////////////////////////////////////////////////////
// Precomputed Tristimulus values
// Tristimulus values CIE1931 2, whitepoint
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_A(       1.0985f     ,1.f  ,0.3558f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_B(       0.9909f     ,1.f  ,0.8531f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_C(       0.9807f     ,1.f  ,1.1822f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_D50(     0.9642f     ,1.f  ,0.8251f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_D55(     0.9568f     ,1.f  ,0.9214f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_D65(     0.9504f     ,1.f  ,1.0888f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_D75(     0.9497f     ,1.f  ,1.2263f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_E(       1.f         ,1.f  ,1.f      );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F1(      0.9283f     ,1.f  ,1.0366f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F2(      0.9914f     ,1.f  ,0.6731f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F3(      1.0375f     ,1.f  ,0.4986f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F4(      1.0914f     ,1.f  ,0.3881f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F5(      0.9087f     ,1.f  ,0.9872f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F6(      0.9730f     ,1.f  ,0.6019f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F7(      0.9501f     ,1.f  ,1.0863f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F8(      0.9641f     ,1.f  ,0.8233f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F9(      1.0036f     ,1.f  ,0.6786f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F10(     0.9617f     ,1.f  ,0.8171f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F11(     1.0089f     ,1.f  ,0.6426f  );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1931_2_F12(     1.0804f     ,1.f  ,0.3922f  );
// Tristimulus values CIE1964 10�, whitepoint
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_A(      1.1114f    ,1.f  ,0.3519f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_B(      0.9917f    ,1.f  ,0.8434f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_C(      0.9728f    ,1.f  ,1.1614f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_D50(    0.9672f    ,1.f  ,0.8142f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_D55(    0.9579f    ,1.f  ,0.9092f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_D65(    0.9480f    ,1.f  ,1.0730f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_D75(    0.9441f    ,1.f  ,1.2064f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_E(      1.f        ,1.f  ,1.f       );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F1(     0.9479f    ,1.f  ,1.0319f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F2(     1.0324f    ,1.f  ,0.6898f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F3(     1.0896f    ,1.f  ,0.5196f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F4(     1.1496f    ,1.f  ,0.4096f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F5(     0.9336f    ,1.f  ,0.9863f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F6(     1.0214f    ,1.f  ,0.6207f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F7(     0.9577f    ,1.f  ,1.0761f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F8(     0.9711f    ,1.f  ,0.8113f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F9(     1.0211f    ,1.f  ,0.6782f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F10(    0.9900f    ,1.f  ,0.8313f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F11(    1.0382f    ,1.f  ,0.6555f   );
static const glm::vec3 standardIlluminant_tristimulusValues_CIE_1964_10_F12(    1.1142f    ,1.f  ,0.4035f   );
/////////////////////////////////////////////////////
// Standard Illuminant Chromaticity Pairs & Tristimulus Values conversion, xyY <=> XYZ
// This is equivalent to xyY -> XYZ
static inline glm::vec3 standardIlluminantTristimulusFromChromaticity( const glm::vec2& iVec )
{
    float tempY = 1.f / iVec.y;
    float X = tempY * iVec.x;
    float Z = tempY * ( 1.f - iVec.x - iVec.y );
    return  glm::vec3( X, 1.f, Z );
}


// This is equivalent to XYZ -> xyY
static inline glm::vec2 standardIlluminantChromaticityFromTristimulus( const glm::vec3& iVec )
{
    float tempSum = iVec.x + iVec.y + iVec.z;
    float x = iVec.x / tempSum;
    float y = iVec.y / tempSum;
    return  glm::vec2( x, y );
}
/////////////////////////////////////////////////////
// Chromatic Adaptation base, mathematical transform model for computing Bradford chromatic adaptation matrix according to source & destination illuminants
//Bradford transformation matrix, used for Chromatic adaptation
static const glm::mat3x3 chromaticAdaptaion_bradfordTransformationMatrix( 0.8951f,  0.2664f, -0.1614f,
                                                                         -0.7502f,  1.7135f,  0.0367f,
                                                                          0.0389f, -0.0685f,  1.0296f );
//Bradford inverse transformation matrix, used for Chromatic adaptation
static const glm::mat3x3 chromaticAdaptaion_bradfordInverseTransformationMatrix( 0.9869929f, -0.1470543f, 0.1599627f,
                                                                                 0.4323053f,  0.5183603f, 0.0492912f,
                                                                                -0.0085287f,  0.0400428f, 0.9684867f );

/////////////////////////////////////////////////////
// LCMS API xyY whitepoints
static const cmsCIExyY whitepoint_D50 = { standardIlluminant_chromaticityCoordinates_CIE_1931_2_D50.x, standardIlluminant_chromaticityCoordinates_CIE_1931_2_D50.y, 1.f };
static const cmsCIExyY whitepoint_D65 = { standardIlluminant_chromaticityCoordinates_CIE_1931_2_D65.x, standardIlluminant_chromaticityCoordinates_CIE_1931_2_D65.y, 1.f };
} // namespace Chroma
} // namespace ULIS

