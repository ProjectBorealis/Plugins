// --------------------------------------------------------------------------
// Helper for constructing a response or request of the camera type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ContentWriter.h"

namespace SceneTransmissionProtocol {
namespace Writers {


    /** A wrapper to aid in creating a conforming protocol-defined Camera type
     *  Response or Request.
     *
     * This class simplifies the process of creating protocol-defined camera
     * Response or Request object. It provides a nicer API over the raw property
     * interface of the core Response/Request objects. It provides a guide for
     * host-applications and ensures the Response/Request matches the 
     * protocol-defined structure of a camera.
     *
     * The CameraWriter must be constructed with a Type. 
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the CameraWriter will need to be constructed before
     * calling beginTypeData, so that the correct field count can be calculated.
     * 
     * The first thing that must occur after this is to call begin().
     * Following this, only the set* methods may be called. set* methods may
     * only be called once.
     *
     * isComplete() can be checked to ensure that all required fields have
     * been set. Once they have, Response/Request::endTypeData() should be called.
     *
     * Each set* method will return one of the WriteStatus codes. Anything
     * apart from OK means that the operation has failed. Note that the error
     * checking is not intended to prevent bad data, but simply to ensure that
     * the construction is well-ordered and complete.
     *
     * Example of use, constructing a response (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(Names::ResponseTypeCamera);
     *
     *   CameraWriter cameraWriter(*response, CameraWriter::Perspective);
     *   response->beginTypeData(cameraWriter.calculateFieldCount());
     *   
     *   cameraWriter.begin();
     *   cameraWriter.setNearClip(1.f);
     *   cameraWriter.setFarClip(10000.f);
     *   cameraWriter.setPerspVerticalFOV(1.04f);
     *   cameraWriter.setPerspAspectRatio(1.5f);
     *   response->endTypeData();
     *
     *   response->beginDynamicData(1);
     *   response->addDataField("myfield", fielddata);
     *   response->endDynamicData();

     *   response->endContent();
     *   if (response->isValid()) {
     *       // send response...
     *   }
     */
    class CameraWriter
    {
    public:
        /**
         Standard projection types for the camera.
         */
        enum Type {
            Orthographic, ///< Orthographic projection (no foreshortening)
            Perspective ///< Perspective projection (with foreshortening)
        };

        /** Construct a CameraWriter wrapping a ContentWriter object
         *  and configuring it to accept the optional fields.
         *
         * It is not valid to call setRequest after using this constructor. The
         * next method call must be to beginContent.
         *
         * @param writer The ContentWriter object upon which we will set type data.
         * @param type The camera type to be constructed.
         */
        CameraWriter(ContentWriter& writer, Type type);

        /**
         */
        ~CameraWriter() {}

        /** Set any initial fields.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();

        /** Set the near clip value of the camera.
         *
         * Mandatory method. May only be called once.
         *
         * @param clip The distance of the near clip plane from the camera.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setNearClip(float clip);

        /** Set the far clip value of the camera.
         *
         * Mandatory method. May only be called once and only after
         * beginContent.
         *
         * @param clip The distance of the far clip plane from the camera.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setFarClip(float clip);

        /** Set the vertical field of view of a perspective camera.
         *
         * Mandatory method. May only be called once and only after
         * beginContent.
         *
         * @param fovRadians The vertical FOV of the camera in radians.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setPerspVerticalFOV(float fovRadians);

        /** Set the aspect ratio of a perspective camera.
         *
         * Mandatory method. May only be called once and only after
         * beginContent.
         *
         * @param aspect The aspect ratio of the camera.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setPerspAspectRatio(float aspect);

        /** Set the horizontal magnification of an orthographic camera.
         *
         * Mandatory method. May only be called once and only after
         * beginContent.
         *
         * @param mag The horizontal magnification.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setOrthoHorizontalMagnification(float mag);


        /** Set the vertical magnification of an orthographic camera.
         *
         * Mandatory method. May only be called once and only after
         * beginContent.
         *
         * @param mag The vertical magnification.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setOrthoVerticalMagnification(float mag);

        /** Finalize the CameraWriter object.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response/request.endTypeData can be called.
         *
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus isComplete();

        /** Calculate the total number of content fields required for this
         *  object.
         *
         *  @return The number of fields in the content section.
         */
        size_t calculateFieldCount() const;

    private:
        /** wrapped ContentWriter object. */
        ContentWriter& _writer;

        /** The type of camera. */
        Type _type;

        /** Various flags to indicate how completion status of the
         *  LightWriter.
         */
        bool _begun;
        bool _hadNearClip;
        bool _hadFarClip;
        bool _hadVerticalFOV;
        bool _hadAspectRatio;
        bool _hadHorizontalMag;
        bool _hadVerticalMag;
        bool _completed;
    };


} // namespace Writers
} // namespace SceneTransmissionProtocol
