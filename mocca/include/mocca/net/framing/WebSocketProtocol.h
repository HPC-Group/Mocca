/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#pragma once

#include "mocca/net/framing/FramingStrategy.h"

#include <unordered_map>

namespace mocca {
namespace net {
class WebSocketProtocol : public FramingStrategy {
public:
    struct ConnectionInfo {
        std::string method;
        std::string path;
        std::string httpVersion;
        std::unordered_map<std::string, std::string> header;
    };

    std::unique_ptr<FramingStrategy> clone() const override;
    std::string name() const override;
    Message readMessageFromStream(IStreamConnection& connection) override;
    void writeMessageToStream(IStreamConnection& connection, Message frame) override;
    void performHandshake(IStreamConnection& connection) override;

private:
    void receiveHandshake(IStreamConnection& connection);
    void sendHandshakeResponse(IStreamConnection& connection);

private:
    ConnectionInfo connectionInfo_;
};
}
}
