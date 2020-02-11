// --------------------------------------------------------------------------
// Helper for constructing a server response of the update type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "ContentWriter.h"
#include "Types.h"

namespace SceneTransmissionProtocol {
namespace Writers {

    /** A wrapper to aid in creating a conformant protocol-defined Update type
     *  Response/Request.
     *
     * This class simplifies the process of creating protocol-defined Update
     * Response/Request object. It provides a nicer API over the raw property
     * interface of the core Response/Request object. It provides a guide for
     * host-applications and ensures the Response/Request matches the protocol-
     * defined structure of an update.
     *
     * Update response/request asks the client to request specific tokens from
     * the server. We can send recursive or individual type of updates.
     * ( recursive only supported for now )
     *
     * The wrapped response/Request must be in a state so that beginTypeData()
     * can be called on it: the response must have been created,  the content
     * section started, and standard content fields set.
     *
     * begin() should be called after calling beginTypeData()
     * Following this, only the set* methods may be called. set* methods may
     * only be called once.
     *
     * The endTypeData() method should be called once all necessary set* and
     * add* methods have been called.
     *
     * Each set* method will return one of the ProgressStatus codes. Anything
     * apart from OK means that the operation has failed. Note that the error
     * checking is not intended to prevent bad data, but simply to ensure that
     * the construction is well-ordered and complete.
     *
     * Example of use (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent();
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(STP::Names::ResponseTypeUpdate);
     *   
     *   UpdateWriter updateWriter(*response, format);
     *   response->beginTypeData(updateWriter.calculateFieldCount());
     *   
     *   updateWriter.begin();
     *   updateWriter.setTokens();
     *   
     *   response->endTypeData();
     *
     *   response.beginDynamicData(0);
     *   response.endDynamicData();

     *   response.endContent();
     *   if (response.isValid()) {
     *       // send response...
     *   }
     */
    class UpdateWriter
    {
    public:
        /** Describes the type of the update response.
         *
         * Individual only updates items in the token array
         * Recursive goes over children of the token arrays
         *
         * Recursive supported for now
         */
        enum Format {
            Individual,
            Recursive
        };

        /** Construct a UpdateWriter wrapping a ContentWriter object
         *  and configuring it to accept the optional fields.
         *
         * It is not valid to call setRequest after using this constructor.
         *
         * @param response The response object upon which we will set type data.
         * @param format The type of the update.
         */
        UpdateWriter(ContentWriter& writer, Format format);

        /**
         */
        ~UpdateWriter() {}

        /** Set any initial fields.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();

        /** Set the raw byte data for updating token strings.
         *
         * Mandatory method. May only be called once.
         * This method iterates through the vector of item tokens and creates a payload
         * of token strings.
         *
         * @param tokens Array of tokens
         * @param count Numer of tokens in array.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setTokenData(const char** tokens, size_t count);


        /** Checks whether the UpdateWriter is complete.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response.endTypeData() can be called.
         *
         * @return The status of the construction.
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
        ContentWriter & _writer;

        /** The update format. */
        Format _format;

        /** Various flags to indicate how completion status of the
         *  UpdateWriter.
         */
        bool _begun;
        bool _hadData;
        bool _completed;
    };
} // namespace Writers
} // namespace SceneTransmissionProtocol
