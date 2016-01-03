#pragma once

#include "mocca/base/MessageQueue.h"
#include "mocca/net/IMessageConnection.h"

namespace mocca {
namespace net {
class LoopbackConnection : public IMessageConnection {
public:
    enum class Signal { Disconnect };
    using MQ = MessageQueue<ByteArray>;
    using SQ = MessageQueue<Signal>;

    LoopbackConnection(std::shared_ptr<MQ> sendQueue, std::shared_ptr<MQ> receiveQueue, std::shared_ptr<SQ> outSignalQueue,
                       std::shared_ptr<SQ> inSignalQueue, const ConnectionID& connectionID);
    ~LoopbackConnection();

    ConnectionID connectionID() const override;
    void send(ByteArray message, std::chrono::milliseconds timeout) const override;
    ByteArray receive(std::chrono::milliseconds timeout) const override;

private:
    ConnectionID connectionID_;
    std::shared_ptr<MQ> sendQueue_;
    std::shared_ptr<MQ> receiveQueue_;

    std::shared_ptr<SQ> outSignalQueue_;
    std::shared_ptr<SQ> inSignalQueue_;
};
}
}