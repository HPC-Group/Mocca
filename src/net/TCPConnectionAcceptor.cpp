#include "mocca/net/TCPConnectionAcceptor.h"

#include "mocca/base/StringTools.h"
#include "mocca/log/LogManager.h"
#include "mocca/net/Sockets.h"

namespace mocca {
namespace net {

TCPConnectionAcceptor::TCPConnectionAcceptor(int port)
    : port_(port) {
    try {
        server_.SetReuseAddress(true);
        server_.SetNonBlocking(true);
        server_.Bind(IVDA::NetworkAddress(IVDA::NetworkAddress::Any, port));
        server_.Listen(3); // ???
    } catch (const IVDA::SocketException& err) {
        std::string internalError = mocca::joinString(err.what(), ", ", err.internalError());
        throw NetworkError("Network error while binding to port (internal error: " + internalError + ")", __FILE__, __LINE__);
    }
}

std::unique_ptr<IPhysicalConnection> TCPConnectionAcceptor::getConnection(std::chrono::milliseconds timeout) {
    IVDA::TCPSocket* connectionSocket = nullptr;
    try {
        server_.AcceptNewConnection((IVDA::ConnectionSocket**)&connectionSocket, static_cast<uint32_t>(timeout.count()));
    } catch (const IVDA::SocketConnectionException&) {
        // a connection terminated before it could be accepted; presumably this is not a
        // critical situation, so we ignore the error
    }
    if (connectionSocket) {
        auto socketPtr = std::unique_ptr<IVDA::ConnectionSocket>(connectionSocket);
        auto ip = connectionSocket->GetPeerAddress();
        LDEBUG("Accepted TCP connection on port " << port_ << " from " << ip);
        return std::unique_ptr<TCPConnection>(new TCPConnection(move(socketPtr)));
    }
    return nullptr;
}
}
}