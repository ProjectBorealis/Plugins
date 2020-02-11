// --------------------------------------------------------------------------
// Helper for accessing the fields of a mesh-type STP reader or request. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ArrayData.h"
#include "StringData.h"

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a ContentReader that is of the protocol-specified
     *  Mesh type.
     *
     * This aids the host-application in reading a protocol-specified Mesh
     * type.
     */
    class MeshReader
    {
    public:
        /**
         Construct an instance of a mesh reader from a source ContentReader object.
         */
        MeshReader(const ContentReader& reader);

        /** Determines whether the reader contains a properly formed mesh.
         *
         * @return Whether the reader contains a properly formed mesh.
         */
        bool isValid() const;

        /** Gets the vertices for this Mesh.
         *
         * @param vertices An array in which the vertices will be stored.
         */
        Array::Handle getVertices() const;

        /** Gets the indices for this Mesh.
         *
         * @param indices An array in which the indices will be stored.
         */
        Array::Handle getIndices() const;

        /** Check whether this mesh has vertex normals.
         *
         * @return Whether the mesh has vertex normals.
         */
        bool hasVertexNormals() const;

        /** Gets the normals for this Mesh.
         *
         * @param normals An array in which the normals will be stored.
         * @return Whether there were any normals. Will match the return value
         *         from hasNormals
         */
        Array::Handle getVertexNormals() const;

        /** Get the number of UV sets in this mesh.
         *
         * @return The number of UV sets in this mesh.
         */
        int32_t getUVSetCount() const;

        /** Get a UV set by its index.
         *
         * The index must be less than the value returned from getUVSetCount.
         *
         * @param indices An array in which the UV indices will be stored.
         * @param UVs An array in which the UVs will be stored.
         * @param uvSetIndex The index of the UV set to retrieve.
         * @return Whether there is a UV set at the index.
         */
        Array::Handle getUVSetIndices(int32_t uvSetIndex) const;
        Array::Handle getUVSetUVs(int32_t uvSetIndex) const;

        /** Get the number of faceset in this mesh.
         *
         * @return The number of facesets in this mesh.
         */
        int32_t getFacesetCount() const;

        /** Get a faceset by its index.
         *
         * The index must be less than the value returned from getFacesetCount.
         *
         * @param faces An array in which the faces will be stored.
         * @param facesetIndex The index of the faceset to retrieve.
         * @return Whether there is a faceset at the index.
         */
        Array::Handle getFacesetFaces(int32_t facesetIndex) const;

        /** Get the material name of a faceset by the faceset's index.
         *
         * The index must be less than the value returned from getFacesetCount.
         *
         * @param facesetIndex The index of the material to retrieve.
         * @return The material name.
         */
        String::Data getFacesetMaterial(int32_t facesetIndex) const;

        /** Get the material name for this mesh.
        *
        * @return The material name.
        */
        String::Data getMaterial() const;

        /** Get the number of RGBA sets in this mesh.
        *
        * @return The number of color sets in this mesh.
        */
        int32_t getRGBASetCount() const;

        /** Get a RGBA set by its index.
        *
        * The index must be less than the value returned from getColorSetCount.
        *
        * @param indices An array in which the RGBA indices will be stored.
        * @param RGBA An array in which the RGBA values will be stored.
        * @param RGBASetIndex The index of the RGBA set to retrieve.
        * @return Whether there is a RGBA set at the index.
        */
        Array::Handle getRGBASetIndices(int32_t RGBASetIndex) const;
        Array::Handle getRGBASetColors(int32_t RGBASetIndex) const;

        /** Get the underlying ContentReader
        *
        * @return The underlying ContentReader
        */
        const ContentReader& getContentReader() const;

    private:
        /** The underlying reader object. */
        const ContentReader& _reader;
    };


    inline MeshReader::MeshReader(const ContentReader& reader)
        : _reader(reader) {}

    inline const ContentReader& MeshReader::getContentReader() const
    {
        return _reader;
    }

} // namespace Readers
} // namespace SceneTransmissionProtocol

