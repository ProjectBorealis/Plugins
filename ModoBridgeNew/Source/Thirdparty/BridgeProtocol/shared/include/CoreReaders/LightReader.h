// --------------------------------------------------------------------------
// Helper for accessing the fields of a light-type SGP server esponse. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a Response that is of the protocol-specified
     *  light type.
     *
     * This aids the host-application in reading a protocol-specified light
     * type.
     */
    class LightReader
    {
    public:
        /**
         Known types of light.
         */
        enum Type
        {
            Unknown,
            Directional,
            Point,
            Spot
        };

        /**
         Representation of RGB colors with float values.
         */
        struct Color
        {
            float r; ///< Red color component
            float g; ///< Green color component
            float b; ///< Blue color component

            Color() : r(1.f), g(1.f), b(1.f) {}
        };

    public:
        /**
         Construct an instance of a light response from a source Response object.
         */
        LightReader(const ContentReader& reader);

        /**
         Query if this is a valid light response.
         @return true if the response is valid for a light, otherwise false.
         */
        bool isValid() const;

        /**
         Retrieve the light type.
         */
        Type getType() const;

        /**
         Retrieve the light intensity.
         */
        float getIntensity() const;

        /**
         Retrieve the light color.
         */
        Color getColor() const;

        /**
         For spotlights, retrieve the inner cone angle, in radians.
         */
        float getInnerConeAngle() const;

        /**
         For spotlights, retrieve the outer cone angle, in radians.
         */
        float getOuterConeAngle() const;

        /**
         For spotlights, retrieve the radius of the light.
         */
        float getSpotRadius() const;

        /**
         For point lights, retrieve the radius of the light.
         */
        float getPointRadius() const;

        /** Get the underlying ContentReader
        *
        * @return The underlying ContentReader
        */
        const ContentReader& getContentReader() const;
    private:
        const ContentReader& _reader;
    };

    inline LightReader::LightReader(const ContentReader& reader)
        : _reader(reader) {}

    inline const ContentReader& LightReader::getContentReader() const
    {
        return _reader;
    }

} // namespace Readers
} // namespace SceneTransmissionProtocol

