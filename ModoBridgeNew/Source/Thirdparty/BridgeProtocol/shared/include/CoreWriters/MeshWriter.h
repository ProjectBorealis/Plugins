// --------------------------------------------------------------------------
// Helper for constructing a response or request of the mesh type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ContentWriter.h"
namespace SceneTransmissionProtocol {
namespace Writers {

    /** A utility wrapper for creating mesh Response/Request objects.
     *
     * This class simplifies the process of creating mesh Response/Request
     * objects. It provides a nicer API over the raw property interface of the
     * core Response/Request object. It provides a guide for host-applications
     * and ensures that the Response/Request matches the protocol-defined 
     * structure of a mesh.
     *
     * The MeshConfig type must be passed to the constructor to setup the
     * optional fields.
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the MeshWriter will need to be constructed before
     * calling beginTypeData, so that the correct field count can be calculated.
     *
     * The first thing that must occur after this is to call begin().
     * Following this, only the set* methods may be called. set* methods may
     * only be called once.
     *
     * isComplete() can be checked to ensure that all required fields have
     * been set. Once they have, Response/Request::endTypeData() should be called.
     *
     * Following this, only set* and add* methods may be called. set* methods
     * may only be called once, while add must be called as many times as to
     * match the setup in the MeshConfig.
     *
     * The endTypeData() method should be called once all necessary set* and
     * add* methods have been called.
     *
     * Each set* and add* method will return one of the ProgressStatus codes.
     * Anything apart from OK means that the operation has failed. Note that
     * the error checking is not intended to prevent bad data, but simply to
     * ensure that the construction is well-ordered and complete.
     *
     * A facesetCount of zero is permitted. In this case, the mesh should be
     * interpreted as if there were an implicit faceset that contains all the
     * meshes faces. The setMaterial() function must be called to set the
     * entity tag of the material to use for this implicit faceset.
     *
     * Example of use constructing a Response (without error checking):
     *
     *   // Assumes that variables like request and vertices have been
     *   // declared elsewhere.
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(STP::Names::ResponseTypeMesh);
     *   
     *   MeshWriter::MeshConfig config;
     *   config.hasNormals = true;
     *   config.uvsetCount = 1;
     *   config.facesetCount = 2;
     *
     *   MeshWriter meshWriter(*response, config);
     *   response->beginTypeData(meshWriter.calculateFieldCount());
     *   
     *   meshWriter.begin();
     *   meshWriter.setVertices(vertices, vcount);
     *   meshWriter.setIndices(indices, icount);
     *   meshWriter.setVertexNormals(normals, ncount);
     *   meshWriter.addFaceset(faces1, fcount1, entityTagFor("material-one"));
     *   meshWriter.addFaceset(faces2, fcount2, entityTagFor("material-two"));
     *   meshWriter.addUVs(uvs, uvcount);
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

    class MeshWriter
    {
    public:
        /** Used to configure the Response to accept the correct number of data
         *  fields.
         */
        struct MeshConfig
        {
			// Inline constructor to set the defaults
			inline MeshConfig()
				: uvsetCount (0)
				, facesetCount (0)
				, RGBAsetCount (0)
				, hasNormals (false)
			{
			}

            /** Number of UVs to add. May be zero. */
            int32_t uvsetCount;

            /** Number of facesets to add. May be zero. */
            int32_t facesetCount;

			/** Number of RGBA sets to add. May be zero. */
			int32_t RGBAsetCount;

            /** Whether this mesh has normals. */
            bool hasNormals;
        };

    public:
        /** Construct a MeshWriter, wrapping a response object
         *  and configuring it to accept the optional fields.
         *
         * @param response The response object upon which we will set type data.
         * @param dataConfig Details of optional fields.
         */
        MeshWriter(ContentWriter& response, const MeshConfig& dataConfig);
        ~MeshWriter();

        /** Set any initial fields.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();

        /** Set the vertices for this mesh.
         *
         * Mandatory method. May only be called once.
         *
         * @param vertices All of the vertices of this mesh.
         * @param count The number of vertices in the mesh.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setVertices(const Types::vertex_elem_t* vertices,
                                             size_t count);

        /** Set the indices for this mesh.
         *
         * Defines a set of polygons constructed from the mesh's vertices.
         *
         * Only triangle polygons are currently supported.
         *
         * Must consist of a set of a triplet indices into the vertex array.
         * Each index must therefore be greater than or equal to zero or less
         * than the length of the vertex array.
         *
         * Mandatory method. May only be called once.
         *
         * @param indices The indices of this mesh.
         * @param count The number of indices.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setIndices(const Types::index_elem_t* indices,
                                            size_t count);

        /** Set the vertex normals for this mesh.
         *
         * Must consist of a set of normals for each polygon defined by the
         * mesh's indices. There must be a number of normals per polygon equal
         * to the polygon's vertex count.
         *
         * The normals must have the same order as the polygons defined by the
         * indices.
         *
         * Note that each normal will consist of three values.
         *
         * Voluntary method. Must be called once for each UV specified in the
         * config on construction.
         *
         * If a MeshWriter has no normals then it's up to the client to
         * reconstruct them.
         *
         * @param normals The normals of the mesh.
         * @param count The number of normals in the mesh.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setVertexNormals(const Types::normal_elem_t* normals,
                                                  size_t count);


        /** Set the material for this mesh.
         *
         * This should only be called when there are no facesets in order to specify
         * the entity token of the material to use for the implicit faceset.
         *
         * @param materialEntityToken Entity token of the material to use for the
         *                               implicit faceset.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setMaterial(const char* materialEntityToken);

        /** Add a set of UVs for this mesh.
         *
         * Must consists of a set of UVs for each polygon defined by the mesh's
         * indices. There must a number of UVs per polygon equal to the
         * polygon's vertex count.
         *
         * The UVs must have the same order as the polygons defined by the
         * indices.
         *
         * Voluntary method. Must be called once for each UV specified in the
         * config on construction.
         *
         * @param UVs The UVs to be added. Must have uvCount * 2 members.
         * @param uvCount The number of UV pairs.
         * @param indices The indices of the vertices. Must have indicesCount members.
         * @param indicesCount The number of indices.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus addUVs(const Types::uv_elem_t* UVs,
                                        size_t uvCount,
                                        const Types::index_elem_t* indices,
                                        size_t indicesCount);

        /** Define a faceset.
         *
         * The indices must be contained within the set defined by setIndices.
         *
         * Voluntary method. Must be called once for each faceset specified in the
         * config on construction.
         *
         * @param faces The faceset to add.
         * @param count The number of faces in the faceset.
         * @param materialEntityToken The entity token that clients can use
         *                            to fetch the material for this faceset.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus addFaceset(const Types::face_elem_t* faces,
                                            size_t count,
                                            const char* materialEntityToken);

		/** Add a set of vertex RGBA values for this mesh.
		*
		* Must consists of a set of RGBA values for each polygon defined by the mesh's
		* indices. There must be a number of RGBA values per polygon equal to the
		* polygon's vertex count. Must also have an array of indices of size equal to the
        * number of pairs in the vertex array.
		*
		* The RGBA values must have the same order as the polygons defined by the
		* indices.
		*
		* Voluntary method. Must be called once for each color set specified in the
		* config on construction.
		*
		* @param RGBA The RGBA values to be added. Must have RGBACount * 4 float members.
		* @param RGBACount The number of RGBA values.
		* @param indices The indices of the vertices. Must have indicesCount members.
		* @param indicesCount The number of indices.
		* @return Status of the construction.
		*/
		ContentWriter::WriteStatus addVertRGBA(const Types::rgba_elem_t* RGBA,
			size_t RGBACount,
			const Types::index_elem_t* indices,
			size_t indicesCount);


        /** Checks whether the MeshWriter is complete.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response.endTypeData() can be called.
         *
         * @return The status of the construction.
         */
        ContentWriter::WriteStatus isComplete();

        /** Calculate the total number of fields required for this object.
         *
         * Certain properties, e.g. facesets, require more than one field for
         * each call to add.
         *
         * @return The number of fields in the content section.
         */
        size_t calculateFieldCount() const;

    private:
        /** Configuration for the variable content fields. */
        MeshConfig _config;

        /** The underlying Response object. */
        ContentWriter& _writer;

        /** Various flags to indicate the completion status of the
         *  MeshWriter object.
         */
        bool _begun;
        bool _hadVertices;
        bool _hadIndices;
        bool _hadNormals;
        bool _hadMaterial;
        bool _completed;

        /** Tallies for the variable content fields. */
        int32_t _uvsetsAdded;
        int32_t _facesetsAdded;
		int32_t _colorsetsAdded;
    };

} // namespace Writers
} // namespace SceneTransmissionProtocol
