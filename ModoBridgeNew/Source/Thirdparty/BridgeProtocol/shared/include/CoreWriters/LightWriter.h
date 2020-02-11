// --------------------------------------------------------------------------
// Helper for constructing a server response or request of the light type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "ContentWriter.h"
#include "Types.h"

namespace SceneTransmissionProtocol {
namespace Writers {

    /** A wrapper to aid in creating a conformant protocol-defined Light type
     *  Response or Request.
     *
     * This class simplifies the process of creating protocol-defined light
     * Response/Request object. It provides a nicer API over the raw property
     * interface of the core Response/Request object. It provides a guide for
     * host-applications and ensures the Response/Request matches the 
     * protocol-defined structure of a light.
     *
     * The LightWriter must be constructed with a Type, and passing a Response/
     * Request upon which the light-specific type data will be set.
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the LightWriter will need to be constructed 
     * before calling beginTypeData, so that the correct field count can be 
     * calculated.
     *
     * The first thing that must occur after this is to call begin().
     * Following this, only the set* methods may be called. set* methods may
     * only be called once.
     *
     * isComplete() can be checked to ensure that all required fields have
     * been set. Once they have, Response::endTypeData() should be called.
     *
     * Each set* method will return one of the ProgressStatus codes. Anything
     * apart from OK means that the operation has failed. Note that the error
     * checking is not intended to prevent bad data, but simply to ensure that
     * the construction is well-ordered and complete.
     *
     * The endTypeData() method should be called once all necessary set* methods
     * have been called.
     *
     * Example of use constructing a Response (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(Names::LightWriter);
     *
     *   LightWriter lightWriter(*response, Point);
     *   response->beginTypeData(lightWriter.calculateFieldCount());
     *
     *   lightWriter.begin();
     *   lightWriter.setIntensity(1000.f);
     *   lightWriter.setColor(0.f, 1.f, 0.f);
     *   
     *   response->endTypeData();
     *
     *   response->beginDynamicData(1);
     *   response->addDataField("myfield", fielddata);
     *   response->endDynamicData();
     *
     *   response->endContent();
     * 
     *   if (response->isValid()) {
     *       // send response...
     *   }
     *
     */
    class LightWriter
    {
    public:
        /** A light is either a directional, spot, or point light. */
        enum Type {
            Directional,
            Spot,
            Point
        };

        /** Construct a LightWriter with a specific type that wraps the given
         *  ContentWriter.
         *
         *
         * @param response The Response upon which we will set type data.
         * @param type The light type to be constructed.
         */
        LightWriter(ContentWriter& writer, Type type);

        /** Destruct this instance of a light writer.
         */
        ~LightWriter() {}

        /** Set any initial fields.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();

        /** Set the intensity of the light.
         *
         * Mandatory method. May only be called once.
         *
         * @param intensity Intensity can be any positive floating point number.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus setIntensity(float intensity);

        /** Set the color of the light.
         *
         * Mandatory method. May only be called once.
         *
         * @param r Red channel.
         * @param g Green channel.
         * @param b Blue channel.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus setColor(float r, float g, float b);

        /** Set the inner cone angle of a spot light.
         *
         * If the light is of type spot, this method is mandatory, otherwise
         * it is disallowed. May be called once.
         *
         * The angle is from the forward axis of the light.
         *
         * @param radians angle of inner cone in radians.
         * @return Status of construction
         */
        ContentWriter::WriteStatus setSpotInnerCone(float radians);

        /** Set the outer cone angle of a spot light.
         *
         * If the light is of type spot, this method is mandatory, otherwise
         * it is disallowed. May be called once.
         *
         * The angle is from the forward axis of the light.
         *
         * @param radians Angle of outer cone in radians.
         * @return Status of construction
         */
        ContentWriter::WriteStatus setSpotOuterCone(float radians);

        /** Set the influence radius of a spot or point light.
         *
         * This is used to determine the falloff of the influence of the light.
         *
         * This method is mandatory for spot and point lights. It is disallowed
         * for directional lights. May be called once.
         *
         * @param radius The distance from the light's location.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus setInfluenceRadius(float radius);

        /** Finalize the LightWriter object.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response.endTypeData can be called.
         *
         * @return Status of construction.
         */
        ContentWriter::WriteStatus isComplete();

        /** Calculate the total number of content fields required for this
         *  object.
         *
         *  @return The number of fields in the content section.
         */
        size_t calculateFieldCount() const;

    private:
        /** Underlying ContentWriter object. */
        ContentWriter& _writer;

        /** The type of light. */
        Type _type;

        /** Various flags to indicate how completion status of the
         *  LightWriter.
         */
        bool _begun;
        bool _hadIntensity;
        bool _hadColor;
        bool _hadInnerCone;
        bool _hadOuterCone;
        bool _hadRadius;
        bool _completed;
    };
} // namespace Writers
} // namespace SceneTransmissionProtocol

