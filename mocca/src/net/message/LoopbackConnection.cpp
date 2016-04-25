/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#include "mocca/net/message/LoopbackConnection.h"

#include "mocca/net/NetworkError.h"
#include "mocca/base/Thread.h"

using namespace mocca;
using namespace mocca::net;

LoopbackConnection::LoopbackConnection(std::shared_ptr<MQ> sendQueue, std::shared_ptr<MQ> receiveQueue, std::shared_ptr<SQ> outSignalQueue,
                                       std::shared_ptr<SQ> inSignalQueue, std::shared_ptr<const ConnectionID> connectionID)
    : connectionID_(connectionID)
    , connected_(true)
    , sendQueue_(sendQueue)
    , receiveQueue_(receiveQueue)
    , outSignalQueue_(outSignalQueue)
    , inSignalQueue_(inSignalQueue) {}

LoopbackConnection::~LoopbackConnection() {
    outSignalQueue_->enqueue(Signal::Disconnect);
}

std::shared_ptr<const ConnectionID> LoopbackConnection::connectionID() const {
    return connectionID_;
}

bool LoopbackConnection::isConnected() const {
    auto signal = inSignalQueue_->dequeueNoWait();
    if (!signal.isNull() && signal == Signal::Disconnect) {
        connected_ = false;
    }
    return connected_;
}

void LoopbackConnection::send(Message message) const {
    if (!isConnected()) {
        throw ConnectionClosedError("Connection to peer " + connectionID_->peerEndpoint.toString() + " has been closed", *connectionID_,
                                    __FILE__, __LINE__);
    }
    sendQueue_->enqueue(std::move(message));
}

Message LoopbackConnection::receive() const {
    mocca::Nullable<Message> message;
    while (message.isNull()) {
        message = receiveQueue_->dequeue(std::chrono::milliseconds(100));
        if (!isConnected()) {
            throw ConnectionClosedError("Connection to peer " + connectionID_->peerEndpoint.toString() + " has been closed", *connectionID_,
                __FILE__, __LINE__);
        }
        if (Runnable::isCurrentInterrupted()) {
            throw ThreadInterrupt(__FILE__, __LINE__);
        }
    }
    return message.release();
}