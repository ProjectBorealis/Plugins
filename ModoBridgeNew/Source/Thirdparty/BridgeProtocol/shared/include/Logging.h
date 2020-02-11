// --------------------------------------------------------------------------
// Provides a logging API. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

namespace SceneTransmissionProtocol {
    /** Logging utility. */
namespace Logging {


    /** API for the host-application to provide its own logging function.
     *
     * The STP libraries will output log information via the #Logging::log
     * function. By default they will output to the standard implementation
     * of CustomSink, which sends log information to std::cout.
     *
     * If the host-application wishes to intercept this, they may inherit from
     * this class and call #setCustomSink. All #Logging::log calls will then
     * be routed to their class.
     */
    struct CustomSink
    {
        /** Logs text.
         *
         * The default implementation sends it to std::cout
         *
         * @param text The text to log.
         */
        virtual void log(const char* text);
    };

    /** Override the sink with a new CustomSink.
     *
     * @param sink The new CustomSink.
     */
    void setCustomSink(CustomSink* sink);

#ifdef STP_ENABLE_LOGGING
    /**
     * Internal implementation of logging some text. Call the \ref log functions in preference.
     * @param text The text to log.
     */
    void internalLog(const char* text);

    /** Log text to the current CustomSink.
     *
     * This is used by the STP libraries to log information.
     *
     * @param text The text to log.
     */
    inline void log(const char* text)
    {
        internalLog(text);
    }

#else
    /**
     Disabled log function.
     @param text Ignored
     */
    inline void log(const char* text) { (void)text; }
#endif

} // namespace Logging
} // namespace SceneTransmissionProtocol
