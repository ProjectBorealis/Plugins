// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Color.ModelSupport.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Spec/ULIS.Spec.Spec.h"
#include "lcms2.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Color Enums Info:
// cmsColorSpaceSignature is the lcms2 way of telling which model is associated to a profile
// ULIS::e_cm it the enum for data layout, which is a subset of cmsColorSpaceSignature, and must match with lcms2 cmsColorSpaceSignature when a profile is assigned
// ULIS::eCColorModel it the enum for CColor models, which is a subset of ULIS::e_cm, and is totally profile agnostic

/////////////////////////////////////////////////////
// Supported CColor Models
enum class eCColorModel : int
{
    kInvalid,
    kG,
    kRGB,
    kHSL,
    kHSV,
    kCMYK,
};


/////////////////////////////////////////////////////
// Model Correspondance
static inline e_cm  ColorModelFromCColorModel( eCColorModel iValue )
{
    switch( iValue )
    {
        case eCColorModel::kInvalid:    return  e_cm::kG;
        case eCColorModel::kG:          return  e_cm::kG;
        case eCColorModel::kRGB:        return  e_cm::kRGB;
        case eCColorModel::kHSL:        return  e_cm::kHSL;
        case eCColorModel::kHSV:        return  e_cm::kHSV;
        case eCColorModel::kCMYK:       return  e_cm::kCMYK;
    }
}


static inline eCColorModel  CColorModelFromColorModel( e_cm iValue )
{
    switch( iValue )
    {
        case e_cm::kG:      return  eCColorModel::kG;
        case e_cm::kRGB:    return  eCColorModel::kRGB;
        case e_cm::kHSL:    return  eCColorModel::kHSL;
        case e_cm::kHSV:    return  eCColorModel::kHSV;
        case e_cm::kCMYK:   return  eCColorModel::kCMYK;
        default:            return  eCColorModel::kInvalid;
    }
}


#define ULIS_SWITCH_SUPPORTED_MODELS( iValue, ... )                                                                 \
    switch( iValue )                                                                                                \
    {                                                                                                               \
        BOOST_PP_CAT( ULIS_SWITCH_SUPPORTED_MODELS_, BOOST_PP_TUPLE_SIZE( (__VA_ARGS__) ) )( iValue, __VA_ARGS__ )  \
    }

#define ULIS_SWITCH_SUPPORTED_MODELS_1( iValue, i0 )                            \
    case i0: return  true;                                                      \
    default: return  false;

#define ULIS_SWITCH_SUPPORTED_MODELS_2( iValue, i0, i1 )                        \
    case i0: return  true;                                                      \
    case i1: return  true;                                                      \
    default: return  false;

#define ULIS_SWITCH_SUPPORTED_MODELS_3( iValue, i0, i1, i2 )                    \
    case i0: return  true;                                                      \
    case i1: return  true;                                                      \
    case i2: return  true;                                                      \
    default: return  false;

#define ULIS_SWITCH_SUPPORTED_MODELS_4( iValue, i0, i1, i2, i3 )                \
    case i0: return  true;                                                      \
    case i1: return  true;                                                      \
    case i2: return  true;                                                      \
    case i3: return  true;                                                      \
    default: return  false;

#define ULIS_SWITCH_SUPPORTED_MODELS_5( iValue, i0, i1, i2, i3, i4 )            \
    case i0: return  true;                                                      \
    case i1: return  true;                                                      \
    case i2: return  true;                                                      \
    case i3: return  true;                                                      \
    case i4: return  true;                                                      \
    default: return  false;

#define ULIS_SWITCH_SUPPORTED_MODELS_6( iValue, i0, i1, i2, i3, i4, i5 )        \
    case i0: return  true;                                                      \
    case i1: return  true;                                                      \
    case i2: return  true;                                                      \
    case i3: return  true;                                                      \
    case i4: return  true;                                                      \
    case i5: return  true;                                                      \
    default: return  false;

#define ULIS_SWITCH_SUPPORTED_MODELS_7( iValue, i0, i1, i2, i3, i4, i5, i6 )    \
    case i0: return  true;                                                      \
    case i1: return  true;                                                      \
    case i2: return  true;                                                      \
    case i3: return  true;                                                      \
    case i4: return  true;                                                      \
    case i5: return  true;                                                      \
    case i6: return  true;                                                      \
    default: return  false;

static inline e_cm ColorModelFromColorSpaceSignature( cmsColorSpaceSignature iValue )
{
    e_cm default_cm = e_cm::kRGB;
    switch( iValue )
    {
        case cmsSigXYZData      : return  e_cm::kXYZ;
        case cmsSigLabData      : return  e_cm::kLab;
        case cmsSigLuvData      : return  default_cm;
        case cmsSigYxyData      : return  default_cm;
        case cmsSigRgbData      : return  e_cm::kRGB;
        case cmsSigGrayData     : return  e_cm::kG;
        case cmsSigHsvData      : return  e_cm::kRGB;
        case cmsSigHlsData      : return  e_cm::kRGB;
        case cmsSigCmykData     : return  e_cm::kCMYK;
        case cmsSigCmyData      : return  e_cm::kRGB;
        case cmsSigMCH1Data     : return  default_cm;
        case cmsSigMCH2Data     : return  default_cm;
        case cmsSigMCH3Data     : return  default_cm;
        case cmsSigMCH4Data     : return  default_cm;
        case cmsSigMCH5Data     : return  default_cm;
        case cmsSigMCH6Data     : return  default_cm;
        case cmsSigMCH7Data     : return  default_cm;
        case cmsSigMCH8Data     : return  default_cm;
        case cmsSigMCH9Data     : return  default_cm;
        case cmsSigMCHAData     : return  default_cm;
        case cmsSigMCHBData     : return  default_cm;
        case cmsSigMCHCData     : return  default_cm;
        case cmsSigMCHDData     : return  default_cm;
        case cmsSigMCHEData     : return  default_cm;
        case cmsSigMCHFData     : return  default_cm;
        case cmsSigNamedData    : return  default_cm;
        case cmsSig1colorData   : return  default_cm;
        case cmsSig2colorData   : return  default_cm;
        case cmsSig3colorData   : return  default_cm;
        case cmsSig4colorData   : return  default_cm;
        case cmsSig5colorData   : return  default_cm;
        case cmsSig6colorData   : return  default_cm;
        case cmsSig7colorData   : return  default_cm;
        case cmsSig8colorData   : return  default_cm;
        case cmsSig9colorData   : return  default_cm;
        case cmsSig10colorData  : return  default_cm;
        case cmsSig11colorData  : return  default_cm;
        case cmsSig12colorData  : return  default_cm;
        case cmsSig13colorData  : return  default_cm;
        case cmsSig14colorData  : return  default_cm;
        case cmsSig15colorData  : return  default_cm;
        case cmsSigLuvKData     : return  default_cm;
        default                 : return  default_cm;
    }
}


static inline constexpr e_cm ColorModelCompatFallback( e_cm iModel )
{
    switch( iModel )
    {
        case e_cm::kG:      return  e_cm::kG;
        case e_cm::kRGB:    return  e_cm::kRGB;
        case e_cm::kHSL:    return  e_cm::kRGB;
        case e_cm::kHSV:    return  e_cm::kRGB;
        case e_cm::kCMYK:   return  e_cm::kCMYK;
        case e_cm::kLab:    return  e_cm::kLab;
        case e_cm::kXYZ:    return  e_cm::kXYZ;

        default:            return  e_cm::kRGB;
    }
}

} // namespace ULIS

