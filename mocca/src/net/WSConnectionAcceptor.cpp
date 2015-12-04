#include "mocca/net/WSConnectionAcceptor.h"

#include "mocca/net/PhysicalConnection.h"
#include "mocca/net/WSConnection.h"
#include "mocca/net/WSHandshake.h"

#include "mocca/log/LogManager.h"

using namespace mocca;
using namespace mocca::net;

WSConnectionAcceptor::WSConnectionAcceptor(std::unique_ptr<IPhysicalConnectionAcceptor> physicalConnectionAcceptor)
    : physicalConnectionAcceptor_(std::move(physicalConnectionAcceptor)) {}

std::unique_ptr<IProtocolConnection> WSConnectionAcceptor::getConnection(std::chrono::milliseconds timeout) {
    auto physicalConnection = physicalConnectionAcceptor_->getConnection(timeout);
    if (physicalConnection == nullptr) {
        return nullptr;
    }
    auto handshake = receiveUntil(*physicalConnection, "\r\n\r\n");
    std::string handshakeStr((char*)handshake.data(), handshake.size());
    auto connectionInfo = mocca::net::parseWSHandshake(handshakeStr);

    auto responseStr = mocca::net::createWSHandshakeResponse(connectionInfo);
    ByteArray handshakeResponse = ByteArray::createFromRaw(responseStr.c_str(), responseStr.size());
    physicalConnection->send(std::move(handshakeResponse));

    return std::unique_ptr<IProtocolConnection>(new WSConnection(std::move(physicalConnection), connectionInfo));
}