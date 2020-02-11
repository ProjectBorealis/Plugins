// --------------------------------------------------------------------------
// Helpers for zmq send and recv.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#define NOMINMAX
#include "zmq.h"
#include <functional>

namespace SceneTransmissionProtocol {
namespace ZeroMQWrappers {

/**
 * ZMQ send message wrapper function.
 *
 * zmq seems to produce EAGAIN errors for blocking sockets in some
 * circumstances. This wrapper handles the EAGAIN cases retrying with
 * a slightly inceasing delay. Will eventually fail with errno EAGAIN
 * after a number of retries to prevent infinitely retrying.
 */
int zmq_msg_send_with_retry(zmq_msg_t *msg, void *s, int flags);

/**
 * ZMQ receive message wrapper function.
 *
 * zmq seems to produce EAGAIN errors for blocking sockets in some
 * circumstances. This wrapper handles the EAGAIN cases retrying with
 * a slightly inceasing delay. Will eventually fail with errno EAGAIN
 * after a number of retries to prevent infinitely retrying.
 */
int zmq_msg_recv_with_retry(zmq_msg_t *msg, void *clientSocket, int flags,
    bool(*checkCancel)(void*) = nullptr, void* userData = nullptr,
    int maxDelay = 10000, int checkInterval = 1000);

} // namespace ZeroMQWrappers
} // namespace SceneTransmissionProtocol
