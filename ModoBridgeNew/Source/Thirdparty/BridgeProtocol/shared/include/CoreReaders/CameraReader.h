// --------------------------------------------------------------------------
// Helper for accessing the fields of a camera-type SGP server response. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a Response that is of the protocol-specified
     *  camera type.
     */
    class CameraReader
    {
    public:
        /**
         Standard camera projection types.
         */
        enum ProjectionType
        {
            Perspective, ///< Perspective projection (with foreshortening)
            Orthographic, ///< Orthographic projection (no foreshortening)
            Unknown ///< Unknown projection type
        };

    public:
        /** Construct the CameraReader wrapper around a Response object:
         *
         * @param response The Response object containing a camera type.
         */
        CameraReader(const ContentReader& reader);

        /** Checks whether the wrapped Response conforms to the camera type
         *  specified in the protocol.
         *
         * @return True if the Response conforms, false otherwise.
         */
        bool isValid();

        /** Get the type of camera.
         *
         * @return One of the projection types. Will return Unknown if the
         *         wrapped Response is not a valid camera.
         */
        ProjectionType getType() const;

        /** Get the vertical field of view in radians.
         *
         * Only valid for perspective cameras.
         *
         * @return Vertical field of view in radians. Will return 0.0f if this
         *         is an orthographic or invalid camera.
         */
        float getVerticalFov() const;

        /** Get the aspect ratio of the field of view.
         *
         * Only valid for perspective cameras.
         *
         * @return The aspect ratio of the field of view. Will return 0.0f if
         *         this is an orthographic or invalid camera.
         */
        float getAspectRatio() const;

        /** Get the distance to the near clipping plane.
         *
         * Will be less than the result of getFarClip.
         *
         * @return The distance to the near clipping plane. Will return -1.0f
         *         if this is an invalid camera.
         */
        float getNearClip() const;

        /** Get the distance to the far clipping plane.
         *
         * Will be greater than the result of getNearClip.
         *
         * @return The distance to the far clipping plane. Will return -1.0f
         *         if this is an invalid camera.
         */
        float getFarClip() const;

        /** Get the horizontal magnification of the view.
         *
         * Only valid for orthographic cameras.
         *
         * @return The horizontal magnification of the view. Will return 0.0f
         *         if this is a perspective or invalid camera.
         */
        float getHorizontalMagnification() const;

        /** Get the vertical magnification of the view.
         *
         * @return The vertical magnification of the view. Will return 0.0f
         *         if this is a perspective or invalid camera.
         */
        float getVerticalMagnification() const;

        /** Get the underlying ContentReader
        *
        * @return The underlying ContentReader
        */
        const ContentReader& getContentReader() const;

    private:
        const ContentReader& _reader;
    };


    inline CameraReader::CameraReader(const ContentReader& reader)
        : _reader(reader) {}

    inline const ContentReader& CameraReader::getContentReader() const
    {
        return _reader;
    }

} // namespace Readers
} // namespace SceneTransmissionProtocol
