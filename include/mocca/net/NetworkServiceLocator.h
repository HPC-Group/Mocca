#pragma once

#include "mocca/net/Endpoint.h"
#include "mocca/net/IMessageConnectionFactory.h"

#include <vector>

namespace mocca {
namespace net {
class NetworkServiceLocator {
public:
    static std::shared_ptr<IMessageConnectionFactory> service(const std::string& protocol);
    static void provideService(std::shared_ptr<IMessageConnectionFactory> service);
    static void removeAll();
    static void provideAll();

    static const std::string& loopback();
    static const std::string& tcpPrefixed();
    static const std::string& tcpWebSocket();
    static const std::string& queuePrefixed();
    static const std::string& queueWebSocket();

    static std::unique_ptr<IMessageConnectionAcceptor> bind(const Endpoint& endpoint);
    static std::unique_ptr<IMessageConnection> connect(const Endpoint& endpoint);

private:
    static std::vector<std::shared_ptr<IMessageConnectionFactory>> services_;
};
}
}