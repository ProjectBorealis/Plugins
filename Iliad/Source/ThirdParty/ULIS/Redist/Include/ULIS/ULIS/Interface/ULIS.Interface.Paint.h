// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Paint.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FPainterContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Color/ULIS.Color.CColor.h"
#include "ULIS/Maths/ULIS.Maths.Geometry.h"
#include <vector>

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FPainterContext
/// @brief      The FPainterContext class provides a context for all Painting operations on Blocks.
class FPainterContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         static  void  DrawLine( IBlock* iBlock, const FPoint& iP0, const FPoint& iP1, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a line without AA.
    /// @details    An aliased line of width 1px will be drawn in the block.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iP0                     The start point of the line.
    /// @param      iP1                     The end point of the line.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static void DrawLine( IBlock* iBlock
                        , const FPoint& iP0
                        , const FPoint& iP1
                        , const CColor& iColor
                        , const FRect& iClippingRect = FRect()
                        , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                        , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawLineAA( IBlock* iBlock, const FPoint& iP0, const FPoint& iP1, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a line with AA.
    /// @details    An anti-aliased line of width 1px will be drawn in the block.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iP0                     The start point of the line.
    /// @param      iP1                     The end point of the line.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawLineAA( IBlock* iBlock
                            , const FPoint& iP0
                            , const FPoint& iP1
                            , const CColor& iColor
                            , const FRect& iClippingRect = FRect()
                            , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                            , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawCircleAndres( IBlock* iBlock, const FPoint& iCenter, int iRadius, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a circle without AA.
    /// @details    A circle will be drawn in the block.
    ///                 - Using Andres method.
    ///                 - No anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the circle is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawCircleAndres( IBlock* iBlock
                                  , const FPoint& iCenter
                                  , int iRadius
                                  , const CColor& iColor
                                  , bool iFilled = false
                                  , const FRect& iClippingRect = FRect()
                                  , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                  , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawCircleAndresAA( IBlock* iBlock, const FPoint& iCenter, int iRadius, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a circle with AA.
    /// @details    A circle will be drawn in the block.
    ///                 - Using Andres method.
    ///                 - With anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the circle is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawCircleAndresAA( IBlock* iBlock
                                    , const FPoint& iCenter
                                    , int iRadius
                                    , const CColor& iColor
                                    , bool iFilled = false
                                    , const FRect& iClippingRect = FRect()
                                    , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                    , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawCircleBresenham( IBlock* iBlock, const FPoint& iCenter, int iRadius, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a circle without AA.
    /// @details    A circle will be drawn in the block.
    ///                 - Using Bresenham method.
    ///                 - No anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the circle is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawCircleBresenham( IBlock* iBlock
                                     , const FPoint& iCenter
                                     , int iRadius
                                     , const CColor& iColor
                                     , bool iFilled = false
                                     , const FRect& iClippingRect = FRect()
                                     , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                     , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawCircleBresenhamAA( IBlock* iBlock, const FPoint& iCenter, int iRadius, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a circle with AA.
    /// @details    A circle will be drawn in the block.
    ///                 - Using Bresenham method.
    ///                 - With anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the circle is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawCircleBresenhamAA( IBlock* iBlock
                                       , const FPoint& iCenter
                                       , int iRadius
                                       , const CColor& iColor
                                       , bool iFilled = false
                                       , const FRect& iClippingRect = FRect()
                                       , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                       , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawRotatedEllipse( IBlock* iBlock, const FPoint& iCenter, int iA, int iB, int iRotationDegrees, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a rotated ellipse without AA.
    /// @details    A rotated ellipse will be drawn.
    ///                 - Without anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the ellipse.
    /// @param      iA                      The first radius of the ellipse.
    /// @param      iB                      The second radius of the ellipse.
    /// @param      iRotationDegrees        The rotation of the ellipse.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the ellipse is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawRotatedEllipse( IBlock* iBlock
                                    , const FPoint& iCenter
                                    , int iA
                                    , int iB
                                    , int iRotationDegrees
                                    , const CColor& iColor
                                    , bool iFilled = false
                                    , const FRect& iClippingRect = FRect()
                                    , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                    , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawRotatedEllipseAA( IBlock* iBlock, const FPoint& iCenter, int iA, int iB, int iRotationDegrees, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a rotated ellipse with AA.
    /// @details    A rotated ellipse will be drawn.
    ///                 - With anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the ellipse.
    /// @param      iA                      The first radius of the ellipse.
    /// @param      iB                      The second radius of the ellipse.
    /// @param      iRotationDegrees        The rotation of the ellipse.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the ellipse is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawRotatedEllipseAA( IBlock* iBlock
                                      , const FPoint& iCenter
                                      , int iA
                                      , int iB
                                      , int iRotationDegrees
                                      , const CColor& iColor
                                      , bool iFilled = false
                                      , const FRect& iClippingRect = FRect()
                                      , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions()
                                      , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawEllipse( IBlock* iBlock, const FPoint& iCenter, int iA, int iB, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true  )
    /// @brief      Draw an ellipse without AA.
    /// @details    An ellipse will be drawn.
    ///                 - Without anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the ellipse.
    /// @param      iA                      The first radius of the ellipse.
    /// @param      iB                      The second radius of the ellipse.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the ellipse is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawEllipse( IBlock* iBlock
                             , const FPoint& iCenter
                             , int iA
                             , int iB
                             , const CColor& iColor
                             , bool iFilled = false
                             , const FRect& iClippingRect = FRect()
                             , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                             , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawEllipseAA( IBlock* iBlock, const FPoint& iCenter, int iA, int iB, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an ellipse with AA.
    /// @details    An ellipse will be drawn.
    ///                 - with anti-aliasing.
    ///                 - Optional filled.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the ellipse.
    /// @param      iA                      The first radius of the ellipse.
    /// @param      iB                      The second radius of the ellipse.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iFilled                 Wether the ellipse is filled or not.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawEllipseAA( IBlock* iBlock
                               , const FPoint& iCenter
                               , int iA
                               , int iB
                               , const CColor& iColor
                               , bool iFilled = false
                               , const FRect& iClippingRect = FRect()
                               , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                               , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawArcAndres( IBlock* iBlock, const FPoint& iCenter, int iRadius, int iStartDegree, int iEndDegree, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an arc circle without AA.
    /// @details    An arc circle will be drawn without anti-aliasing, using Andres method.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iStartDegree            The angular start of the arc in degrees.
    /// @param      iEndDegree              The angular end of the arc in degrees.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawArcAndres( IBlock* iBlock
                               , const FPoint& iCenter
                               , int iRadius
                               , int iStartDegree
                               , int iEndDegree
                               , const CColor& iColor
                               , const FRect& iClippingRect = FRect()
                               , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                               , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawArcAndresAA( IBlock* iBlock, const FPoint& iCenter, int iRadius, int iStartDegree, int iEndDegree, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an arc circle with AA.
    /// @details    An arc circle will be drawn with anti-aliasing, using Andres method.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iStartDegree            The angular start of the arc in degrees.
    /// @param      iEndDegree              The angular end of the arc in degrees.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawArcAndresAA( IBlock* iBlock
                                 , const FPoint& iCenter
                                 , int iRadius
                                 , int iStartDegree
                                 , int iEndDegree
                                 , const CColor& iColor
                                 , const FRect& iClippingRect = FRect()
                                 , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                 , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawArcBresenham( IBlock* iBlock, const FPoint& iCenter, int iRadius, int iStartDegree, int iEndDegree, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an arc circle without AA.
    /// @details    An arc circle will be drawn without anti-aliasing, using Bresenham method.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iStartDegree            The angular start of the arc in degrees.
    /// @param      iEndDegree              The angular end of the arc in degrees.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawArcBresenham( IBlock* iBlock
                                  , const FPoint& iCenter
                                  , int iRadius
                                  , int iStartDegree
                                  , int iEndDegree
                                  , const CColor& iColor
                                  , const FRect& iClippingRect = FRect()
                                  , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                  , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawArcBresenhamAA( IBlock* iBlock, const FPoint& iCenter, int iRadius, int iStartDegree, int iEndDegree, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an arc circle with AA.
    /// @details    An arc circle will be drawn with anti-aliasing, using Bresenham method.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCenter                 The center of the circle.
    /// @param      iRadius                 The radius of the circle.
    /// @param      iStartDegree            The angular start of the arc in degrees.
    /// @param      iEndDegree              The angular end of the arc in degrees.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawArcBresenhamAA( IBlock* iBlock
                                    , const FPoint& iCenter
                                    , int iRadius
                                    , int iStartDegree
                                    , int iEndDegree
                                    , const CColor& iColor
                                    , const FRect& iClippingRect = FRect()
                                    , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                                    , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawRectangle( IBlock* iBlock, const FPoint& iTopLeft, const FPoint& iBottomRight, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a rectangle.
    /// @details    An rectangle optionally filled will be drawn.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iTopLeft                The top left corner.
    /// @param      iBottomRight            The bottom right corner.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawRectangle( IBlock* iBlock
                               , const FPoint& iTopLeft
                               , const FPoint& iBottomRight
                               , const CColor& iColor
                               , bool iFilled = false
                               , const FRect& iClippingRect = FRect()
                               , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                               , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawPolygon( IBlock* iBlock, std::vector< FPoint >& iPoints, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an arbitrary convex or concave polygon without anti-aliasing.
    /// @details    The polygon can have any number of points, works for any polygon.
    ///             The polygon is filled, and the fill rule follows the even-odd rule.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iPoints                 A vector of points.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawPolygon( IBlock* iBlock
                             , std::vector< FPoint >& iPoints
                             , const CColor& iColor
                             , bool iFilled = false
                             , const FRect& iClippingRect = FRect()
                             , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                             , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawPolygonAA( IBlock* iBlock, std::vector< FPoint >& iPoints, const CColor& iColor, bool iFilled = false, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw an arbitrary convex or concave polygon with anti-aliasing.
    /// @details    The polygon can have any number of points, works for any polygon.
    ///             The polygon is filled, and the fill rule follows the even-odd rule.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iPoints                 A vector of points.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawPolygonAA( IBlock* iBlock
                               , std::vector< FPoint >& iPoints
                               , const CColor& iColor
                               , bool iFilled = false
                               , const FRect& iClippingRect = FRect()
                               , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions()
                               , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawQuadraticBezier( IBlock* iBlock, const FPoint& iCtrlPt0, const FPoint& iCtrlPt1, const FPoint& iCtrlPt2, float iWeight, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a quadratic bezier curve without AA;
    /// @details    A quadratic bezier curve will be drawn without anti-aliasing.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCtrlPt0                The control point 0 of the quadratic bezier.
    /// @param      iCtrlPt1                The control point 1 of the quadratic bezier.
    /// @param      iCtrlPt2                The control point 2 of the quadratic bezier.
    /// @param      iWeight                 The weight of the control points of the quadratic bezier.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawQuadraticBezier( IBlock* iBlock
                                     , const FPoint& iCtrlPt0
                                     , const FPoint& iCtrlPt1
                                     , const FPoint& iCtrlPt2
                                     , float iWeight
                                     , const CColor& iColor
                                     , const FRect& iClippingRect = FRect()
                                     , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions()
                                     , bool iCallInvalidCB = true );


    /// @fn         static  void  DrawQuadraticBezierAA( IBlock* iBlock, const FPoint& iCtrlPt0, const FPoint& iCtrlPt1, const FPoint& iCtrlPt2, float iWeight, const CColor& iColor, const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions(), bool iCallInvalidCB = true )
    /// @brief      Draw a quadratic bezier curve with AA;
    /// @details    A quadratic bezier curve will be drawn with anti-aliasing.
    /// @param      iBlock                  The pointer to the \e IBlock to draw on.
    /// @param      iCtrlPt0                The control point 0 of the quadratic bezier.
    /// @param      iCtrlPt1                The control point 1 of the quadratic bezier.
    /// @param      iCtrlPt2                The control point 2 of the quadratic bezier.
    /// @param      iWeight                 The weight of the control points of the quadratic bezier.
    /// @param      iColor                  The color to use for drawing.
    /// @param      iClippingRect           The clipping rectangle to draw the form in
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    /// @param      iCallInvalidCB          Whether or not the function should call the invalid call back in the back block after the operation finished.
    static  void  DrawQuadraticBezierAA( IBlock* iBlock
                                       , const FPoint& iCtrlPt0
                                       , const FPoint& iCtrlPt1
                                       , const FPoint& iCtrlPt2
                                       , float iWeight
                                       , const CColor& iColor
                                       , const FRect& iClippingRect = FRect()
                                       , const FPerformanceOptions& iPerformanceOptions = FPerformanceOptions()
                                       , bool iCallInvalidCB = true );

};

} // namespace ULIS

