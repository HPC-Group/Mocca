/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#pragma once

#include "mocca/base/MessageQueue.h"
#include "mocca/net/message/LoopbackConnection.h"

namespace mocca {
namespace net {
class LoopbackConnectionSpawner {
public:
    LoopbackConnectionSpawner(const std::string& name);
    std::unique_ptr<IMessageConnection> getClientConnection();
    std::unique_ptr<IMessageConnection> getServerConnection(std::chrono::milliseconds timeout);
    std::string name() const;

private:
    std::string name_;
    MessageQueue<std::unique_ptr<LoopbackConnection>> serverConnections_;
};
}
}
