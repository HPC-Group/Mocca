#pragma once

#include "mocca/net/IStreamConnection.h"
#include "mocca/net/stream/Sockets.h"

namespace mocca {
namespace net {

class TCPConnection : public IStreamConnection {
public:
    TCPConnection(std::unique_ptr<IVDA::ConnectionSocket> socket);
    ~TCPConnection();

    ConnectionID connectionID() const override;
    void send(ByteArray message, std::chrono::milliseconds timeout) const override;
    ByteArray receive(uint32_t maxSize, std::chrono::milliseconds timeout) const override;

private:
    std::unique_ptr<IVDA::ConnectionSocket> socket_;
};
}
}