/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#pragma once

#include "mocca/base/MessageQueue.h"
#include "mocca/net/stream/QueueConnection.h"

namespace mocca {
namespace net {
class QueueConnectionSpawner {
public:
    QueueConnectionSpawner(const std::string& name);
    std::unique_ptr<IStreamConnection> getClientConnection();
    std::unique_ptr<IStreamConnection> getServerConnection(std::chrono::milliseconds timeout);
    std::string name() const;

private:
    std::string name_;
    MessageQueue<std::unique_ptr<QueueConnection>> serverConnections_;
};
}
}