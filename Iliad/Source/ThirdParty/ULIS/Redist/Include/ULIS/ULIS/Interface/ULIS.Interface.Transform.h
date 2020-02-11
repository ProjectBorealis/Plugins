// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Transform.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FTransformContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Transform/ULIS.Transform.ResamplingMethods.h"
#include <glm/matrix.hpp>

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FTransformContext
/// @brief      The FTransformContext class provides a context for all Transform operations on Blocks.
class FTransformContext
{
public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------- Public Static Matrix API
    /// @fn         static  glm::mat3  GetIdentityMatrix()
    /// @brief      Get the identity 3x3 matrix.
    /// @return     A fresh identity matrix representing a no-op transform.
    static  glm::mat3  GetIdentityMatrix();


    /// @fn         static  glm::mat3  GetTranslationMatrix( float iX, float iY )
    /// @brief      Get a 2D translation transformation as 3x3 matrix.
    /// @param      iX      The amount of translation on the x axis in pixels.
    /// @param      iY      The amount of translation on the y axis in pixels.
    /// @return     A fresh pure translation matrix representing a 2D translation.
    static  glm::mat3  GetTranslationMatrix( float iX, float iY );


    /// @fn         static  glm::mat3  GetRotationMatrix( float iAngleRad )
    /// @brief      Get a 2D rotation transformation as 3x3 matrix.
    /// @param      iAngleRad   The angle of the rotation in radians.
    /// @return     A fresh pure rotation matrix representing a 2D rotation.
    static  glm::mat3  GetRotationMatrix( float iAngleRad );


    /// @fn         static  glm::mat3  GetScaleMatrix( float iX, float iY )
    /// @brief      Get a 2D scale transformation as 3x3 matrix.
    /// @details    The scale values are not percentage, but normalized: 1 is no scale, 2 is twice.
    /// @param      iX      The amount of scale on the x axis.
    /// @param      iY      The amount of scale on the y axis.
    /// @return     A fresh pure scale matrix representing a 2D scale.
    static  glm::mat3  GetScaleMatrix( float iX, float iY );


    /// @fn         static  glm::mat3  GetShearMatrix( float iX, float iY )
    /// @brief      Get a 2D shear transformation as 3x3 matrix.
    /// @param      iX      The amount of shear on the x axis.
    /// @param      iY      The amount of shear on the y axis.
    /// @return     A fresh pure shear matrix representing a 2D shear.
    static  glm::mat3  GetShearMatrix( float iX, float iY );


public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         static IBlock* GetTransformed( const IBlock* iBlockSrc, const  glm::mat3& imat, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    /// @brief      Get the result of the block transformed by the input transform.
    /// @param      iBlock                  The source block to transform ( remains untouched ).
    /// @param      iMat                    The transform to apply.
    /// @param      iPerformanceOptions     The performance preferences.
    /// @return     A fresh newly allocated block, with contents transformed from source, with size deduced from input block and transform.
    static IBlock* GetTransformed( const IBlock* iBlock
                                 , const  glm::mat3& iMat
                                 , eResamplingMethod iResamplingMethod = eResamplingMethod::kBilinear
                                 , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() );


    /// @fn         static IBlock* TransformInto( const IBlock* iBlockSrc, IBlock* iDst, const  glm::mat3& imat, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    /// @brief      Transform source block into preallocatod destination.
    /// @param      iBlockSrc               The source block to transform ( remains untouched ).
    /// @param      iBlockDst               The destination block to transform ( receives the result ).
    /// @param      iMat                    The transform to apply.
    /// @param      iPerformanceOptions     The performance preferences.
    /// @return     A fresh newly allocated block, with contents transformed from source, with size deduced from input block and transform.
    static IBlock* TransformInto( const IBlock* iBlockSrc
                                , IBlock* iBlockDst
                                , const  glm::mat3& iMat
                                , eResamplingMethod iResamplingMethod = eResamplingMethod::kBilinear
                                , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() );


    /// @fn         static FRect  GetTransformPreviewRect( const IBlock* iBlockSrc, const  glm::mat3& iMat )
    /// @brief      Get the Rect corresponding to the size after the transform, for previewing purpose.
    /// @details    No transform is actually computed, just the bounding box.
    /// @param      iBlockSrc               The source block to transform ( remains untouched ).
    /// @param      iMat                    The transform to apply.
    /// @return     A FRect with the preview location and size of the transformed input.
    static FRect  GetTransformPreviewRect( const IBlock* iBlockSrc
                                         , const  glm::mat3& iMat );

};

} // namespace ULIS

