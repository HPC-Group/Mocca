#include "mocca/net/MoccaConnection.h"
#include "mocca/net/PhysicalConnection.h"

#include <mutex>

using namespace mocca;
using namespace mocca::net;

mocca::net::MoccaConnection::MoccaConnection(std::unique_ptr<IPhysicalConnection> physicalConnection)
    : physicalConnection_(std::move(physicalConnection)) {}

std::string MoccaConnection::identifier() const {
    return physicalConnection_->identifier();
}

void MoccaConnection::send(ByteArray message) const {
    // fixme: performance loss; implement prepend method for ByteArray
    ByteArray newMessage(message.size() + sizeof(uint32_t));
    newMessage << message.size();
    newMessage.append(message);
    physicalConnection_->send(std::move(newMessage));
}

ByteArray MoccaConnection::receive(std::chrono::milliseconds timeout) const {
    std::lock_guard<IPhysicalConnection> lock(*physicalConnection_);
    auto sizeData = receiveExactly(*physicalConnection_, sizeof(uint32_t), timeout);
    if (sizeData.isEmpty()) {
        return ByteArray();
    }
    auto size = sizeData.get<uint32_t>();
    auto data = receiveExactly(*physicalConnection_, size, timeout);
    return data;
}