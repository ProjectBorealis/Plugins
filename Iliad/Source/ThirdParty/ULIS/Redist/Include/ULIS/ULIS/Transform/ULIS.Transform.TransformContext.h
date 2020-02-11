// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Transform.TransformContext.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Transform/ULIS.Transform.ResamplingMethods.h"
#include "ULIS/Transform/ULIS.Transform.BlockTransformer.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Make/ULIS.Make.MakeContext.h"
#include "ULIS/Maths/ULIS.Maths.Geometry.h"
#include "ULIS/Maths/ULIS.Maths.Utility.h"
#include <glm/matrix.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <assert.h>

// Load optimizations
#include "ULIS/Transform/Optimization/ULIS.Transform.Optimization.RGBA8.h"

namespace ULIS {
/////////////////////////////////////////////////////
// TTransformContext
class TTransformContext
{
public:
    template< uint32 _SH >
    static  TBlock< _SH >*  GetTransformedBlock( const TBlock< _SH >* iSrc, const  glm::mat3& iMat, eResamplingMethod iResamplingMethod = eResamplingMethod::kBilinear, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    {
        /*
        {
            FTransformOBB obb( 0, 0, iSrc->Width(), iSrc->Height() );
            obb.Transform( iMat );
            FTransformAABB aabb( obb );
            glm::vec2 shift = aabb.GetShift();
            obb.Shift( shift );
            glm::mat3 translatedTransformMatrix = glm::translate( glm::identity< glm::mat3 >(), glm::vec2( shift ) ) * iMat;
            glm::mat3 inverseTransform = glm::inverse( translatedTransformMatrix );
            TBlock< _SH >* dst = new TBlock< _SH >( FMath::Ceil( aabb.Width() ), FMath::Ceil( aabb.Height() ) );
            TBlockTransformer< _SH >::Run( iSrc, dst, inverseTransform, iResamplingMethod, iPerformanceOptions );
            return  dst;
        }
        */

        FTransformAABB aabb( 0, 0, iSrc->Width(), iSrc->Height() );
        aabb.Transform( iMat );
        glm::vec2 shift( -aabb.x1, -aabb.y1 );
        aabb.Shift( shift );
        glm::mat3 translated = glm::translate( glm::identity< glm::mat3 >(), glm::vec2( shift ) ) * iMat;
        TBlock< _SH >* dst = new TBlock< _SH >( aabb.Width(), aabb.Height() );
        glm::mat3 inverseTransform = glm::inverse( translated );
        TBlockTransformer< _SH >::Run( iSrc, dst, inverseTransform, iResamplingMethod, iPerformanceOptions );
        return  dst;
    }

    template< uint32 _SH >
    static  void  TransformBlockInto( const TBlock< _SH >* iSrc, TBlock< _SH >* iDst, const  glm::mat3& iMat, eResamplingMethod iResamplingMethod = eResamplingMethod::kBilinear , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions() )
    {
        /*
        {
            FTransformOBB obb( 0, 0, iSrc->Width(), iSrc->Height() );
            obb.Transform( iMat );
            FTransformAABB aabb( obb );
            glm::vec2 shift = aabb.GetShift();
            obb.Shift( shift );
            glm::mat3 translatedTransformMatrix = glm::translate( glm::identity< glm::mat3 >(), glm::vec2( shift ) ) * iMat;
            glm::mat3 inverseTransform = glm::inverse( translatedTransformMatrix );
            FOBBHullExpression hull = obb.BuildHullExpression();
            TBlockTransformer< _SH >::Run( iSrc, iDst, inverseTransform, iResamplingMethod, iPerformanceOptions );
        }
        */

        FTransformAABB aabb( 0, 0, iSrc->Width(), iSrc->Height() );
        aabb.Transform( iMat );
        glm::vec2 shift( -aabb.x1, -aabb.y1 );
        aabb.Shift( shift );
        glm::mat3 translated = glm::translate( glm::identity< glm::mat3 >(), glm::vec2( shift ) ) * iMat;
        glm::mat3 inverseTransform = glm::inverse( translated );
        TBlockTransformer< _SH >::Run( iSrc, iDst, inverseTransform, iResamplingMethod, iPerformanceOptions );
    }
};


} // namespace ULIS

