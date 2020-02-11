// --------------------------------------------------------------------------
// Helper for constructing a response or request of the mesh instance type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ContentWriter.h"
namespace SceneTransmissionProtocol {
namespace Writers {

    /** A utility wrapper for creating mesh instance Response/Request objects.
     *
     * This class simplifies the process of creating mesh instance Response/Request
     * objects. It provides a nicer API over the raw property interface of the
     * core Response/Request object. It provides a guide for host-applications
     * and ensures that the Response/Request matches the protocol-defined 
     * structure of a mesh instance.
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the MeshInstanceWriter will need to be constructed before
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
     * may only be called once, while add must be called as many times.
     *
     * The endTypeData() method should be called once all necessary set* and
     * add* methods have been called.
     *
     * Each set* and add* method will return one of the ProgressStatus codes.
     * Anything apart from OK means that the operation has failed. Note that
     * the error checking is not intended to prevent bad data, but simply to
     * ensure that the construction is well-ordered and complete.
     *
     * Example of use constructing a Response (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(STP::Names::ResponseTypeMeshInstance);
     *
     *   MeshInstanceWriter meshInstWriter(*response);
     *   response->beginTypeData(meshInstWriter.calculateFieldCount());
     *   
     *   meshInstWriter.begin();
     *   meshInstWriter.setMeshToken(meshToken);
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

    class MeshInstanceWriter
    {
    public:
        /** Construct a MeshInstanceWriter, wrapping a response object
         *  and configuring it to accept the optional fields.
         *
         * @param writer The writer object upon which we will set type data.
         * @param meshToken Item token of the instanced mesh. 
         * Note that mesh token will be applied after begin has been called.
         */
        MeshInstanceWriter(ContentWriter& writer, const char* meshToken);
        ~MeshInstanceWriter();

        /** Set any initial fields.
         *
         * Sets the original mesh token of this instance.
         *
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();

        /** Checks whether the MeshInstanceWriter is complete.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response.endTypeData() can be called.
         *
         * @return The status of the construction.
         */
        ContentWriter::WriteStatus isComplete();

        /** Calculate the total number of fields required for this object.
         *
         * @return The number of fields in the content section.
         */
        size_t calculateFieldCount() const;

    private:
        /** The underlying Response object. */
        ContentWriter& _writer;

        /** Unowned mesh token memory pointer. */
        const char* _meshToken;

        /** Various flags to indicate the completion status of the
         *  MeshInstanceWriter object.
         */
        bool _begun;
        bool _hadMeshToken;
        bool _completed;
    };

} // namespace Writers
} // namespace SceneTransmissionProtocol
