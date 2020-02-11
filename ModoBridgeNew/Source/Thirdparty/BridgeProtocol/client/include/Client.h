// --------------------------------------------------------------------------
// Provides the API for making requests from a SGP server.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Response.h"
#include "StringData.h"

namespace SceneTransmissionProtocol {
/** The Client library for the SceneTransmissionProtocol
 */
namespace Client {
    class Request;

    /** Provides the API for reading data from the server.
     *
     * This class provides the client host-application the means to fetch
     * scene data from the server.
     *
     * The host-application provides the Client with a function to handle each
     * response. The Client will send each received Response to the function.
     *
     * This interface is designed to be used without exception support. The status
     * should be checked
     */
    class Client
    {
    public:

        Client();
        ~Client();

        /**
         Client can be in one of a number of states throughout its lifetime.
         */
        enum Status
        {
            StatusUnopened,   // constructed, but open() has not been performed
            StatusOK,         // connection has been made with the server and requests can be made
            StatusError,      // the connection has failed, error() method can give a status string. Implies closed.
            StatusClosed      // the connection was closed as a result of close()
        };

        /**
         Client transmission status can be in one of a number of states throughout its lifetime
         Trasnmission status reflects both receive and send ZMQ message methods
         */
        enum TransmissionStatus
        {
            TStatusOK,          // No transmission error was had
            TStatusReceiving,   // Currently receiving response
            TStatusError        // Transmission has invoked an ZMQ error
        };

        /**
         * Definitions of the callbacks used to interface with the host-
         * application.
         * 
         * Each callback has a void* parameter for passing user data back to the
         * host-application. This user data is initially passed to one of the
         * fetch methods.
         * 
         * handlerCB:
         *   When a \ref Response is received from the server, this callback
         *   will be called. The client relinquishes control of the \ref Response
         *   and it is therefore the responsibility of the host-application to
         *   delete the Response.
         *   
         * checkCancelCB:
         *   Called intermittently from the client. If the callback returns true
         *   then the current fetch will be cancelled.
         *   
         * onExitCB:
         *   Called when the client thread end.
         *   
         * onEmptyCB:
         *  Called when all active requests have been sent and their responses
         *  received. Will only occur after a fetch has been initiated, not after
         *  get calls.
         */
        typedef void (*handlerCB)(Response*, void* data);
        typedef bool (*checkCancelCB)(void* data);
        typedef void (*onExitCB)(void* data);
        typedef void (*onEmptyCB)(void* data);

        /**
         * Wraps the host application callbacks and user data into an easier
         * to handle struct. All host-applications must provide a handlerCB at
         * a minimum.
         */
        struct HostInterface
        {
            HostInterface(handlerCB h)
                : handler(h),
                  checkCancel(nullptr),
                  onExit(nullptr),
                  onEmpty(nullptr) {}

            handlerCB handler;
            checkCancelCB checkCancel;
            onExitCB onExit;
            onEmptyCB onEmpty;
            void* userData;
        };

        /// Determine the status of the client.
        Status getStatus();

        /// If the status is StatusError, get a debugging message for this error
        String::RAII getErrorString();

        /// Determine the status of the transmission.
        TransmissionStatus getTransmissionStatus();

        /// If the transmission status is StatusError, get a debugging message for this error
        String::RAII getTransmissionErrorString();

        /** Connect to a server.
         *
         * Attempt connecting to a server.
         *
         * This will change the status from StatusUnopened, StatusClosed or StatusError to StatusOK
         * on successful connect, or to StatusError on connection failure.
         *
         * Trying to reopen an already open will result in StatusError.
         *
         * @param host Hostname or IP address of the server to connect to.
         * @param port Port on the host to attach to.
         * @param timeout Timeout for not receiving any data (in milliseconds).
         * @param checkInterval How often should updates be checked for (in milliseconds).
         */
        void open(const char* host, int port, int timeout, int checkInterval);

        /**
         * Disconnect from the server.
         *
         * Updates the status to StatusClosed, except if the status is StatusError.
         */
        void close();

        /**
         * Start client thread that runs continually, sends requests, sends periodic update requests and fetches responses
         */
        void startClient(const HostInterface& hostInterface);

        /**
        * Stops client thread that runs continually
        */
        void stopClient();

        /**
        Is client thread running
        */
        bool isRunning();

        /**
         * Enqueue a token to be fetched asynchronously.
         * 
         * This is automatically converted into a Request object internally.
         * 
         * @param token The token to fetch.
         * @return Whether the token was successfully queued to be fetched. 
         */
        bool fetchToken(const char* token);

        /**
         * Add a \ref Request to be fetched asynchronously.
         * 
         * The Client takes ownership of the \ref Request object and should not be
         * used by the caller after this point.
         * 
         * @param request The \ref Request to enqueue.
         * @param Whether the \ref Request was successfully queued to be fetched.
         */
        bool fetchRequest(Request* request);

        /** Retrieve the entire scene graph blockingly and synchronously.
         *
         * @param handler Function that will be passed the response object.
         * @param checkCancel Function that will check the cancellation state,
         *                    returning true when cancellation is required.
         * @param data Opaque pointer to host-application data that will be
         *                    passed to the callbacks. May be null.
         */
        void fetch(const HostInterface& hostInterface);

        /** Cancel a background fetch.
         */
        void cancelFetch();

        /** Retrieve a specific entity by token.
         *
         * @param entitytag Entity tag to fetch, or empty-string for the root.
         * @param handler   Callback that will be passed the response object.
         * @param checkCancel Callback that will be called during the send and
         *                    receive, to allow the host application to cancel
         *                    the operation.
         * @param userData Opaque pointer to host-application data that will be
         *                 passed to the handlers. May be null.
         */
        void get(const char* entitytag, const HostInterface& hostInterface);

        /** Retrieve a specific entity by Request.
         * 
         * Note that the Client assumes ownership of the Request object. It
         * should not be retained by the host-application after calling this
         * function.
         *
         * @param request Request to enqueue for fetch.
         * @param handler   Callback that will be passed the response object.
         * @param checkCancel Callback that will be called during the send and
         *                    receive, to allow the host application to cancel
         *                    the operation.
         * @param userData Opaque pointer to host-application data that will be
         *                 passed to the handlers. May be null.
         */
        void get(Request* request, const HostInterface& hostInterface);

    private:
        class Pimpl;
        Pimpl* _pimpl;
    };

} // namespace Client
} // namespace SceneTransmissionProtocol

