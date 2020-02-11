// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Decl.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the specialisations related to block types.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.Registry.h"
#include "ULIS/Data/ULIS.Data.Block.h"

#ifdef ULIS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
#endif

namespace ULIS {
/////////////////////////////////////////////////////
// Reg baking
/* we spec the name of the build reg */
#define ULIS_REG_BUILDER ulis_reg_builder
/* we create the reg with a first invalid entry (0) */
ULIS_CREATE_REG( ULIS_REG_BUILDER, 0 )


/////////////////////////////////////////////////////
// Types declspec
/* Actual types specializations */
ULIS_DECLSPEC(uint8,G,hasAlpha,GA,typeLimits)
ULIS_DECLSPEC(uint8,G,hasAlpha,AG,typeLimits)
ULIS_DECLSPEC(uint8,G,noAlpha,G,typeLimits)
ULIS_DECLSPEC(uint16,G,hasAlpha,GA,typeLimits)
ULIS_DECLSPEC(uint16,G,hasAlpha,AG,typeLimits)
ULIS_DECLSPEC(uint16,G,noAlpha,G,typeLimits)
ULIS_DECLSPEC(uint32,G,hasAlpha,GA,typeLimits)
ULIS_DECLSPEC(uint32,G,hasAlpha,AG,typeLimits)
ULIS_DECLSPEC(uint32,G,noAlpha,G,typeLimits)
ULIS_DECLSPEC(float,G,hasAlpha,GA,normalized)
ULIS_DECLSPEC(float,G,hasAlpha,AG,normalized)
ULIS_DECLSPEC(float,G,noAlpha,G,normalized)
ULIS_DECLSPEC(float,G,noAlpha,G,typeLimits)
ULIS_DECLSPEC(uint8,RGB,hasAlpha,ARGB,typeLimits)
ULIS_DECLSPEC(uint8,RGB,hasAlpha,ABGR,typeLimits)
ULIS_DECLSPEC(uint8,RGB,hasAlpha,RGBA,typeLimits)
ULIS_DECLSPEC(uint8,RGB,hasAlpha,BGRA,typeLimits)
ULIS_DECLSPEC(uint8,RGB,noAlpha,RGB,typeLimits)
ULIS_DECLSPEC(uint8,RGB,noAlpha,BGR,typeLimits)
ULIS_DECLSPEC(uint16,RGB,hasAlpha,ARGB,typeLimits)
ULIS_DECLSPEC(uint16,RGB,hasAlpha,ABGR,typeLimits)
ULIS_DECLSPEC(uint16,RGB,hasAlpha,RGBA,typeLimits)
ULIS_DECLSPEC(uint16,RGB,hasAlpha,BGRA,typeLimits)
ULIS_DECLSPEC(uint16,RGB,noAlpha,RGB,typeLimits)
ULIS_DECLSPEC(uint16,RGB,noAlpha,BGR,typeLimits)
ULIS_DECLSPEC(uint32,RGB,hasAlpha,ARGB,typeLimits)
ULIS_DECLSPEC(uint32,RGB,hasAlpha,ABGR,typeLimits)
ULIS_DECLSPEC(uint32,RGB,hasAlpha,RGBA,typeLimits)
ULIS_DECLSPEC(uint32,RGB,hasAlpha,BGRA,typeLimits)
ULIS_DECLSPEC(uint32,RGB,noAlpha,RGB,typeLimits)
ULIS_DECLSPEC(uint32,RGB,noAlpha,BGR,typeLimits)
ULIS_DECLSPEC(float,RGB,hasAlpha,ARGB,normalized)
ULIS_DECLSPEC(float,RGB,hasAlpha,ABGR,normalized)
ULIS_DECLSPEC(float,RGB,hasAlpha,RGBA,normalized)
ULIS_DECLSPEC(float,RGB,hasAlpha,BGRA,normalized)
ULIS_DECLSPEC(float,RGB,noAlpha,RGB,normalized)
ULIS_DECLSPEC(float,RGB,noAlpha,BGR,normalized)
ULIS_DECLSPEC(uint8,HSL,hasAlpha,HSLA,typeLimits)
ULIS_DECLSPEC(uint16,HSL,hasAlpha,HSLA,typeLimits)
ULIS_DECLSPEC(uint32,HSL,hasAlpha,HSLA,typeLimits)
ULIS_DECLSPEC(float,HSL,hasAlpha,HSLA,normalized)
ULIS_DECLSPEC(uint8,HSV,hasAlpha,HSVA,typeLimits)
ULIS_DECLSPEC(uint16,HSV,hasAlpha,HSVA,typeLimits)
ULIS_DECLSPEC(uint32,HSV,hasAlpha,HSVA,typeLimits)
ULIS_DECLSPEC(float,HSV,hasAlpha,HSVA,normalized)
ULIS_DECLSPEC(uint8,CMYK,hasAlpha,CMYKA,typeLimits)
ULIS_DECLSPEC(uint16,CMYK,hasAlpha,CMYKA,typeLimits)
ULIS_DECLSPEC(uint32,CMYK,hasAlpha,CMYKA,typeLimits)
ULIS_DECLSPEC(float,CMYK,hasAlpha,CMYKA,normalized)
ULIS_DECLSPEC(uint8,Lab,hasAlpha,LabA,typeLimits)
ULIS_DECLSPEC(uint16,Lab,hasAlpha,LabA,typeLimits)
ULIS_DECLSPEC(uint32,Lab,hasAlpha,LabA,typeLimits)
ULIS_DECLSPEC(float,Lab,hasAlpha,LabA,normalized)
ULIS_DECLSPEC(float,Lab,hasAlpha,LabA,typeLimits)
ULIS_DECLSPEC(uint8,XYZ,hasAlpha,XYZA,typeLimits)
ULIS_DECLSPEC(uint16,XYZ,hasAlpha,XYZA,typeLimits)
ULIS_DECLSPEC(uint32,XYZ,hasAlpha,XYZA,typeLimits)
ULIS_DECLSPEC(float,XYZ,hasAlpha,XYZA,normalized)
ULIS_DECLSPEC(float,XYZ,hasAlpha,XYZA,typeLimits)


namespace generated {
/////////////////////////////////////////////////////
// Reg assign
/* we assign the result reg */
ULIS_REG_TYPE ulis_types_reg = ULIS_ASSIGN_REG( ULIS_REG_BUILDER );
/* we spec the name of the result reg for easy usage */
#define ULIS_REG ::ULIS::generated::ulis_types_reg


/////////////////////////////////////////////////////
// Reg Size
/* temp assign reg size ( workaround ) */
constexpr auto temp_reg_size = ulis_types_reg.Size();
/* Reg size macro for usage during preprocessor ( workaround ) */
#define ULIS_REG_SIZE 59

} // namespace generated


/////////////////////////////////////////////////////
// Extern template declarations
/*
#define ULIS_REG_OP( z, n, data ) extern template class TBlock< ULIS_REG[ n ] >;
ULIS_REPEAT( ULIS_REG_SIZE, ULIS_REG_OP, void )
#undef ULIS_REG_OP
*/


/////////////////////////////////////////////////////
// Mainstream typedefs
/* easy typedefs for the most common block types defined earlier */
namespace Format {
static uint32 Format_RGBA8 = ::ULIS::Format::Format_uint8RGBhasAlphaRGBAtypeLimits;
} // namespace Format

typedef ::ULIS::Format::FBlockuint8GnoAlphaGtypeLimits          FBlockG8;
typedef ::ULIS::Format::FBlockfloatGnoAlphaGnormalized          FBlockGf;
typedef ::ULIS::Format::FBlockuint32HSLhasAlphaHSLAtypeLimits   FBlockHSLA32;
typedef ::ULIS::Format::FBlockuint8RGBhasAlphaRGBAtypeLimits    FBlockRGBA8;
typedef ::ULIS::Format::FBlockuint8RGBhasAlphaBGRAtypeLimits    FBlockBGRA8;
typedef ::ULIS::Format::FBlockuint8RGBhasAlphaARGBtypeLimits    FBlockARGB8;
typedef ::ULIS::Format::FBlockuint8RGBhasAlphaABGRtypeLimits    FBlockABGR8;
typedef ::ULIS::Format::FBlockfloatRGBhasAlphaRGBAnormalized    FBlockRGBAfn;
typedef ::ULIS::Format::FBlockfloatRGBhasAlphaBGRAnormalized    FBlockBGRAfn;
typedef ::ULIS::Format::FBlockfloatRGBhasAlphaARGBnormalized    FBlockARGBfn;
typedef ::ULIS::Format::FBlockfloatRGBhasAlphaABGRnormalized    FBlockABGRfn;
typedef ::ULIS::Format::FBlockfloatLabhasAlphaLabAtypeLimits    FBlockLabAfn;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint8GnoAlphaGtypeLimits > FValueG8;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatGnoAlphaGnormalized > FValueGf;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint8HSLhasAlphaHSLAtypeLimits >    FValueHSLA8;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatHSLhasAlphaHSLAnormalized >    FValueHSLAf;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint32HSLhasAlphaHSLAtypeLimits >   FValueHSLA32;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint8RGBhasAlphaRGBAtypeLimits > FValueRGBA8;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint8RGBhasAlphaBGRAtypeLimits > FValueBGRA8;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint8RGBhasAlphaARGBtypeLimits > FValueARGB8;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_uint8RGBhasAlphaABGRtypeLimits > FValueABGR8;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatRGBhasAlphaRGBAnormalized > FValueRGBAfn;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatRGBhasAlphaBGRAnormalized > FValueBGRAfn;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatRGBhasAlphaARGBnormalized > FValueARGBfn;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatRGBhasAlphaABGRnormalized > FValueABGRfn;
typedef ::ULIS::TPixelValue< ::ULIS::Format::Format_floatLabhasAlphaLabAtypeLimits > FValueLabAfn;


/////////////////////////////////////////////////////
// TDefaultModelFormat Specialization
template<> constexpr uint32 TDefaultModelFormat< e_cm::kG >()     { return  ::ULIS::Format::Format_floatGhasAlphaGAnormalized;       }
template<> constexpr uint32 TDefaultModelFormat< e_cm::kRGB >()   { return  ::ULIS::Format::Format_floatRGBhasAlphaRGBAnormalized;   }
template<> constexpr uint32 TDefaultModelFormat< e_cm::kHSL >()   { return  ::ULIS::Format::Format_floatHSLhasAlphaHSLAnormalized;   }
template<> constexpr uint32 TDefaultModelFormat< e_cm::kHSV >()   { return  ::ULIS::Format::Format_floatHSVhasAlphaHSVAnormalized;   }
template<> constexpr uint32 TDefaultModelFormat< e_cm::kCMYK >()  { return  ::ULIS::Format::Format_floatCMYKhasAlphaCMYKAnormalized; }
template<> constexpr uint32 TDefaultModelFormat< e_cm::kLab >()   { return  ::ULIS::Format::Format_floatLabhasAlphaLabAtypeLimits;   }
template<> constexpr uint32 TDefaultModelFormat< e_cm::kXYZ >()   { return  ::ULIS::Format::Format_floatXYZhasAlphaXYZAtypeLimits;   }


/////////////////////////////////////////////////////
// CMS extra formats for connection
#define ULIS_LCMS_DTYPE_GRAYA_FLT   (FLOAT_SH(1)|COLORSPACE_SH(PT_GRAY)|EXTRA_SH(1)|CHANNELS_SH(1)|BYTES_SH(4))
#define ULIS_LCMS_DTYPE_RGBA_FLT    (FLOAT_SH(1)|COLORSPACE_SH(PT_RGB)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(4))
#define ULIS_LCMS_DTYPE_CMYKA_FLT   (FLOAT_SH(1)|COLORSPACE_SH(PT_CMYK)|EXTRA_SH(1)|CHANNELS_SH(4)|BYTES_SH(4))
#define ULIS_LCMS_DTYPE_LabA_FLT    (FLOAT_SH(1)|COLORSPACE_SH(PT_Lab)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(4))
#define ULIS_LCMS_DTYPE_XYZA_FLT    (FLOAT_SH(1)|COLORSPACE_SH(PT_XYZ)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(4))

#define ULIS_LCMS_DTYPE_GRAYA_U16   (COLORSPACE_SH(PT_GRAY)|EXTRA_SH(1)|CHANNELS_SH(1)|BYTES_SH(2))
#define ULIS_LCMS_DTYPE_RGBA_U16    (COLORSPACE_SH(PT_RGB)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(2))
#define ULIS_LCMS_DTYPE_CMYKA_U16   (COLORSPACE_SH(PT_CMYK)|EXTRA_SH(1)|CHANNELS_SH(4)|BYTES_SH(2))
#define ULIS_LCMS_DTYPE_LabA_U16    (COLORSPACE_SH(PT_Lab)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(2))
#define ULIS_LCMS_DTYPE_XYZA_U16    (COLORSPACE_SH(PT_XYZ)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(2))


/////////////////////////////////////////////////////
// TCMSConnectionFormat Specialization
template<> constexpr uint32 TCMSConnectionType< e_cm::kG >()     { return  ULIS_LCMS_DTYPE_GRAYA_FLT; }
template<> constexpr uint32 TCMSConnectionType< e_cm::kRGB >()   { return  ULIS_LCMS_DTYPE_RGBA_FLT;  }
template<> constexpr uint32 TCMSConnectionType< e_cm::kHSL >()   { return  ULIS_LCMS_DTYPE_RGBA_FLT;  }
template<> constexpr uint32 TCMSConnectionType< e_cm::kHSV >()   { return  ULIS_LCMS_DTYPE_RGBA_FLT;  }
template<> constexpr uint32 TCMSConnectionType< e_cm::kCMYK >()  { return  ULIS_LCMS_DTYPE_CMYKA_U16; }
template<> constexpr uint32 TCMSConnectionType< e_cm::kLab >()   { return  ULIS_LCMS_DTYPE_LabA_FLT;  }
template<> constexpr uint32 TCMSConnectionType< e_cm::kXYZ >()   { return  ULIS_LCMS_DTYPE_XYZA_FLT;  }


/////////////////////////////////////////////////////
// TModelConnectionFormat Specialization
template<> constexpr uint32 TModelConnectionFormat< e_cm::kG >()     { return  ::ULIS::Format::Format_floatGhasAlphaGAnormalized;       }
template<> constexpr uint32 TModelConnectionFormat< e_cm::kRGB >()   { return  ::ULIS::Format::Format_floatRGBhasAlphaRGBAnormalized;   }
template<> constexpr uint32 TModelConnectionFormat< e_cm::kHSL >()   { return  ::ULIS::Format::Format_floatRGBhasAlphaRGBAnormalized;   }
template<> constexpr uint32 TModelConnectionFormat< e_cm::kHSV >()   { return  ::ULIS::Format::Format_floatRGBhasAlphaRGBAnormalized;   }
template<> constexpr uint32 TModelConnectionFormat< e_cm::kCMYK >()  { return  ::ULIS::Format::Format_uint16CMYKhasAlphaCMYKAtypeLimits;}
template<> constexpr uint32 TModelConnectionFormat< e_cm::kLab >()   { return  ::ULIS::Format::Format_floatLabhasAlphaLabAtypeLimits;   }
template<> constexpr uint32 TModelConnectionFormat< e_cm::kXYZ >()   { return  ::ULIS::Format::Format_floatXYZhasAlphaXYZAtypeLimits;   }

} // namespace ULIS

#ifdef ULIS_CLANG
#pragma GCC diagnostic pop
#endif

