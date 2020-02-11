// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.BlendingModes.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once

namespace ULIS {
/////////////////////////////////////////////////////
// eBlendingMode
enum class eBlendingMode : uint8
{
      kNormal
    , kErase
    , kBehind
    , kDissolve

    , kDarken
    , kMultiply
    , kColorBurn
    , kLinearBurn
    , kDarkerColor

    , kLighten
    , kScreen
    , kColorDodge
    , kLinearDodge
    , kLighterColor

    , kOverlay
    , kSoftLight
    , kHardLight
    , kVividLight
    , kLinearLight
    , kPinLight
    , kHardMix

    , kDifference
    , kExclusion
    , kSubstract
    , kDivide

    , kHue
    , kSaturation
    , kColor
    , kLuminosity

    , kNumBlendingModes
};


static const char* kwBlendingMode[] =
{
      "Normal"
    , "Erase"
    , "Behind"
    , "Dissolve"

    , "Darken"
    , "Multiply"
    , "ColorBurn"
    , "LinearBurn"
    , "DarkerColor"

    , "Lighten"
    , "Screen"
    , "ColorDodge"
    , "LinearDodge"
    , "LighterColor"

    , "Overlay"
    , "SoftLight"
    , "HardLight"
    , "VividLight"
    , "LinearLight"
    , "PinLight"
    , "HardMix"

    , "Difference"
    , "Exclusion"
    , "Substract"
    , "Divide"

    , "Hue"
    , "Saturation"
    , "Color"
    , "Luminosity"

    , "Invalid"
};


#define ULIS_FOR_ALL_BLENDING_MODES_DO( iSelectedBlendingMode, X )                                  \
    switch( iSelectedBlendingMode ) {                                                               \
        case eBlendingMode::kNormal             :   X( eBlendingMode::kNormal           );  break;  \
        case eBlendingMode::kErase              :   X( eBlendingMode::kErase            );  break;  \
        case eBlendingMode::kBehind             :   X( eBlendingMode::kBehind           );  break;  \
        case eBlendingMode::kDissolve           :   X( eBlendingMode::kDissolve         );  break;  \
        case eBlendingMode::kDarken             :   X( eBlendingMode::kDarken           );  break;  \
        case eBlendingMode::kMultiply           :   X( eBlendingMode::kMultiply         );  break;  \
        case eBlendingMode::kColorBurn          :   X( eBlendingMode::kColorBurn        );  break;  \
        case eBlendingMode::kLinearBurn         :   X( eBlendingMode::kLinearBurn       );  break;  \
        case eBlendingMode::kDarkerColor        :   X( eBlendingMode::kDarkerColor      );  break;  \
        case eBlendingMode::kLighten            :   X( eBlendingMode::kLighten          );  break;  \
        case eBlendingMode::kScreen             :   X( eBlendingMode::kScreen           );  break;  \
        case eBlendingMode::kColorDodge         :   X( eBlendingMode::kColorDodge       );  break;  \
        case eBlendingMode::kLinearDodge        :   X( eBlendingMode::kLinearDodge      );  break;  \
        case eBlendingMode::kLighterColor       :   X( eBlendingMode::kLighterColor     );  break;  \
        case eBlendingMode::kOverlay            :   X( eBlendingMode::kOverlay          );  break;  \
        case eBlendingMode::kSoftLight          :   X( eBlendingMode::kSoftLight        );  break;  \
        case eBlendingMode::kHardLight          :   X( eBlendingMode::kHardLight        );  break;  \
        case eBlendingMode::kVividLight         :   X( eBlendingMode::kVividLight       );  break;  \
        case eBlendingMode::kLinearLight        :   X( eBlendingMode::kLinearLight      );  break;  \
        case eBlendingMode::kPinLight           :   X( eBlendingMode::kPinLight         );  break;  \
        case eBlendingMode::kHardMix            :   X( eBlendingMode::kHardMix          );  break;  \
        case eBlendingMode::kDifference         :   X( eBlendingMode::kDifference       );  break;  \
        case eBlendingMode::kExclusion          :   X( eBlendingMode::kExclusion        );  break;  \
        case eBlendingMode::kSubstract          :   X( eBlendingMode::kSubstract        );  break;  \
        case eBlendingMode::kDivide             :   X( eBlendingMode::kDivide           );  break;  \
        case eBlendingMode::kHue                :   X( eBlendingMode::kHue              );  break;  \
        case eBlendingMode::kSaturation         :   X( eBlendingMode::kSaturation       );  break;  \
        case eBlendingMode::kColor              :   X( eBlendingMode::kColor            );  break;  \
        case eBlendingMode::kLuminosity         :   X( eBlendingMode::kLuminosity       );  break;  \
        case eBlendingMode::kNumBlendingModes   :   X( eBlendingMode::kNumBlendingModes );  break;  \
    }


/////////////////////////////////////////////////////
// eAlphaMode
enum class eAlphaMode : uint8
{
      kNormal
    , kErase
    , kTop
    , kBack
    , kSub
    , kAdd
    , kMul
    , kMin
    , kMax
    , kNumAlphaModes
};


static const char* kwAlphaMode[] =
{
      "Normal"
    , "Erase"
    , "Top"
    , "Back"
    , "Sub"
    , "Add"
    , "Mul"
    , "Min"
    , "Max"
    , "Invalid"
};


#define ULIS_FOR_ALL_ALPHA_MODES_DO( iSelectedAlphaMode, X )                                \
    switch( iSelectedAlphaMode ) {                                                          \
        case eAlphaMode::kNormal            :   X( eAlphaMode::kNormal          );  break;  \
        case eAlphaMode::kErase             :   X( eAlphaMode::kErase           );  break;  \
        case eAlphaMode::kTop               :   X( eAlphaMode::kTop             );  break;  \
        case eAlphaMode::kBack              :   X( eAlphaMode::kBack            );  break;  \
        case eAlphaMode::kSub               :   X( eAlphaMode::kSub             );  break;  \
        case eAlphaMode::kAdd               :   X( eAlphaMode::kAdd             );  break;  \
        case eAlphaMode::kMin               :   X( eAlphaMode::kMin             );  break;  \
        case eAlphaMode::kMax               :   X( eAlphaMode::kMax             );  break;  \
        case eAlphaMode::kNumAlphaModes     :   X( eAlphaMode::kNumAlphaModes   );  break;  \
    }


static inline constexpr bool IsNonSeparableBlendingMode( eBlendingMode iBlendingMode )
{
    switch( iBlendingMode )
    {
        case eBlendingMode::kDissolve       :   return  true;
        case eBlendingMode::kDarkerColor    :   return  true;
        case eBlendingMode::kLighterColor   :   return  true;
        case eBlendingMode::kHue            :   return  true;
        case eBlendingMode::kSaturation     :   return  true;
        case eBlendingMode::kColor          :   return  true;
        case eBlendingMode::kLuminosity     :   return  true;
        default:                                return  false;
    }
}


} // namespace ULIS