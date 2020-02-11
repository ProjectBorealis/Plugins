// --------------------------------------------------------------------------
// API for constructing requests to send to a SGP server.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "Operation.h"
#include "Types.h"
#include "ContentWriter.h"
#include <string>

namespace SceneTransmissionProtocol {
namespace Client {
    /** Packages a request for a resource to be sent to the server.
     *
     * Once the Request is created, \ref startHeader must be called before
     * \ref endHeader. Calling either more than once is invalid. Calling any of
     * the set methods more than once is valid. Calling them before
     * startHeader() or after endHeader() is invalid.
     *
     *      startHeader
     *      setEntityToken | setIdentifier | setOperation
     *      endHeader
     *
     * Note that the \ref Request will generate defaults for each part of the
     * header. These defaults are:
     *
     *      entity token = ""
     *      identifier = ""
     *      operation = Operation::ReadOp
     *
     * The defaults will request a read of the root of the scene - an empty
     * entity token is equivalent to explicitly requesting the root - with no
     * identifier.
     *
     * It is therefore possible to simply call startHeader() and then
     * endHeader() and get a valid request.
     *
     * This class implements the pimpl pattern, so as to hide the underlying
     * data packing method.
     */
    class Request
    {
    public:
        /** Progress of the Request.
         */
        enum ProgressStatus
        {
            OK,                     ///< Everything is OK.
            OpMismatch,             ///< Tried to do something not supported by current op.
            DuplicateHeaderField,   ///< Two instances of the same header field exist
            IncorrectStage,         ///< Current stage is not expected
            InsufficientFields,     ///< Fewer fields than were requested have been provided
            TooManyFields,          ///< More fields than were requested have been provided
            DuplicateField,         ///> Attempted to add named field again.
            IncorrectFieldForType,  ///> Attempted to set a type-data field which is incorrect for the type.
            ConfigMismatchForType   ///> Attempted to set a type-data field which breaks a configuration constraint (i.e. adding too many instances of an optional field)
        };

        // Define request scene root token string constant
        // The empty string is the host-application-neutral 
        // way of asking the server for the root of the scene graph.
        // Used in Request::setEntityToken(SceneRootToken);
        static constexpr const char* SceneRootToken = "";

    public:
        /* Constructs an empty Request.
         * The request will be invalid until the header has been constructed
         * via the method calls outlined above.
         */
        Request(Operation::Operation op);

        /* Destroys the Request.
         * Note that the Request retains ownership of its data and therefore
         * the pointer returned from @getData is invalid after destruction.
         */
        ~Request();

        /** Indicates whether this Request is correctly constructed.
         *
         * @return Whether the Request is well-formed.
         */
        bool isValid() const;

        /** Begins the header data write.
         *
         * This must be called before any of the set* methods and before
         * endHeader. To do otherwise is a critical error.
         *
         * @see setEntityToken(), setIdentifier(), setOperation(),
         *      endHeader()
         *
         * @return Status after starting the header.
         */
        ProgressStatus startHeader();

        /** Sets the unique token that maps to the entity being requested.
         *
         * @param token The unique token for the requested entity.
         *
         * @return Status after setting the entity token.
         */
        ProgressStatus setEntityToken(const char* token);

        /** Set the identifer for this request.
         *
         * This is the value that a client can use to match the corresponding
         * response. This is useful in system that supports parallel requests.
         *
         * It is valid for the identifier to be an empty string, i.e. its
         * default.
         *
         * @param id The identifier.
         *
         * @return Status after setting the identifier.
         */
        ProgressStatus setIdentifier(const char* id);

        /** Finish processing of the header.
         *
         * It is invalid to call this before following the full
         * sequence given above.
         *
         * @return Status after finishing the header.
         */
        ProgressStatus endHeader();

        /** Start the content section.
         *
         * This must only be called after endHeader unless the Request was
         * constructed with a Request object, in which case it must be called
         * immediately after construction.
         *
         * @return The status of the construction.
         */
        ProgressStatus beginContent();

        /** End the content section.
         *
         * This will finalize the Request and no other calls are valid. Can
         * only be called after all of the content fields have been set.
         *
         * @return The status of the construction.
         */
        ProgressStatus endContent();

        /** Set all of the standard content fields.
         *
         * This is a shortcut for setting the standard shortcut fields, rather
         * than having to call individual methods. This is useful as it is
         * possible that the standard content fields will change in the future
         * and non-conforming calls will cause compile errors.
         *
         * Can only be called once. Precludes any other calls to setType,
         * setParentToken, setChildTokens, and setTransform.
         *
         * It is still necessary to call setType. This is normally done by type-
         * specific helpers (like LightRequest).
         *
         * @param parentToken @see setParentToken
         * @param childTokens @see setChildTokens
         * @param childCount Length of childTokens array
         * @param transform @see setTransform
         * @return The status of the construction.
         */
        ProgressStatus setStandardContentFields(
            const char* parentToken,
            const char** childTokens,
            size_t childCount,
            const Types::Transform& transform);

        /** Set the type field of the content.
         *
         * Can only be called between beginContent and endContent. May only be
         * called once, including a call to setStandardContentFields. Normally
         * done by type-specific helpers (like LightRequest). Must be called
         * to set the type when a type-specific helper isn't used.
         *
         * @param type Type of the response.
         *
         * @return The status of the construction.
         */
        ProgressStatus setType(const char* type);

        /** Set the parent-token field of the content.
         *
         * Can only be called between beginContent and endContent. May only be
         * called once, including a call to setStandardContentFields.
         *
         * @param parent The token of the entity's parent.
         * @return The status of the construction.
         */
        ProgressStatus setParentToken(const char* parent);

        /** Set the children field of the content.
         *
         * Can only be called between beginContent and endContent. May only be
         * called once, including a call to setStandardContentFields.
         *
         * @param children The set of the entity's child tokens.
         * @return The status of the construction.
         */
        ProgressStatus setChildTokens(const char** children, size_t count);

        /** Set the transform field of the content.
         *
         * Can only be called between beginContent and endContent. May only be
         * called once, including a call to setStandardContentFields.
         *
         * @param transform The local space transform of the entity.
         * @return The status of the construction.
         */
        ProgressStatus setTransform(const Types::Transform& transform);

        /** Set the display name field of the content.
        *
        * Can only be called between beginContent and endContent. May only be
        * called once, including a call to setStandardContentFields.
        *
        * setDisplayName is an optional method.
        *
        * @param displayName The display name string of the entity
        * @return The status of the construction.
        */
        ProgressStatus setDisplayName(const char* displayName);

        /** Start the type data section.
         *
         * Can only be called between beginContent and endContent. Cannot be
         * called more than once.
         *
         * @param fieldCount Number of fields in the response.
         *
         * @return The status of the construction.
         */
        ProgressStatus beginTypeData(size_t fieldCount);

        /** End the type data section.
         *
         * Can only be called after beginTypeData has been called and after
         * the number of calls to addDataField matches the number passed to
         * beginTypeData.
         *
         * @return The status of the construction.
         */
        ProgressStatus endTypeData();

        /** Start the dynamic data section.
         *
         * Can only be called between beginContent and endContent. Cannot be
         * called more than once.
         *
         * @param fieldCount Number of fields in the response.
         *
         * @return The status of the construction.
         */
        ProgressStatus beginDynamicData(size_t fieldCount);

        /** End the dynamic data section.
         *
         * Can only be called after beginDynamicData has been called and after
         * the number of calls to addDataField matches the number passed to
         * beginDynamicData.
         *
         * @return The status of the construction.
         */
        ProgressStatus endDynamicData();

        /** Get the ContentWriter for the content section
         *
         * Should only be used between beginning and ending the type and dynamic
         * data sections.
         *
         * @return The \ref ContentWriter for the content section.
         */
        Writers::ContentWriter& getContentWriter();


        /** Get the buffer that contains the completed request.
         *
         * It is invalid to call this before the initialisation sequence
         * has been completed.
         *
         * It is invalid to call this after calling takeData.
         *
         * @return The buffer containing the request if the Request is
         *         completed and takeData has not been called, otherwise
         *         nullptr.
         */
        const char* getData() const;

        /** Get the size of the buffer that contains the completed request.
         *
         * It is invalid to call this before the initialisation sequence has
         * been completed.
         *
         * It is invalid to call this after calling takeData, therefore this
         * must be called before calling takeData and the return value stored.
         *
         * @return The size of the  buffer containing the request if it is
         *         completed and takeData has not been called, otherwise
         *         nullptr.
         */
        size_t getDataSize() const;

        /** Gets and releases ownership of the buffer that contains the
         *  completed request.
         *
         * The Request will no longer clear up the memory as ownership has been
         * transferred.
         *
         * It is invalid to call this before the initialisation sequence has
         * been completed.
         *
         * It is invalid to call this more than once.
         *
         * @return The buffer containing the request, if it is complete,
         *         otherwise nullptr.
         */
        char* takeData();

        /** Gets a string representation of this Request
         * 
         * @return string representation
         */
        const char* getRepresentation() const;

    private:
        class Pimpl;
        Pimpl* _pimpl;

        /** Used internally to determine current construction status. */
        bool _hadEntityToken;
        bool _hadIdentifier;

        bool _hadParent;
        bool _hadChildren;
        bool _hadTransform;
        bool _hadType;
        bool _hadDisplayName;
    };

    inline Request::ProgressStatus writeToProgressStatus(Writers::ContentWriter::WriteStatus status)
    {
        Request::ProgressStatus ps = Request::OK;
        switch (status) {
        case Writers::ContentWriter::InvalidActiveSection:
            ps = Request::IncorrectStage;
            break;

        case Writers::ContentWriter::TooManyFields:
            ps = Request::TooManyFields;
            break;

        default:
            break;
        }
        return ps;
    }

} // namespace Client
} // namespace SceneTransmissionProtocol
