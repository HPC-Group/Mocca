#include "mocca/net/NetworkServiceLocator.h"

#include "mocca/base/StringTools.h"
#include "mocca/net/Error.h"
#include "mocca/net/framing/SizePrefixedProtocol.h"
#include "mocca/net/message/FramingConnectionFactory.h"
#include "mocca/net/stream/QueueConnectionFactory.h"
#include "mocca/net/stream/TCPConnectionFactory.h"

using namespace mocca::net;

std::vector<std::shared_ptr<IMessageConnectionFactory>> NetworkServiceLocator::services_;

std::shared_ptr<IMessageConnectionFactory> NetworkServiceLocator::service(const std::string& protocol) {
    for (auto& service : services_) {
        if (service->protocol() == protocol) {
            return service;
        }
    }
    throw NetworkError(mocca::formatString("Network service for protocol '%%' is not available", protocol), __FILE__, __LINE__);
}

void NetworkServiceLocator::provideService(std::shared_ptr<IMessageConnectionFactory> service) {
    for (auto& s : services_) {
        if (s->protocol() == service->protocol()) {
            throw NetworkError(mocca::formatString("Network service for protocol '%%' already provided to locator", service->protocol()),
                               __FILE__, __LINE__);
        }
    }
    services_.push_back(service);
}

void NetworkServiceLocator::removeAll() {
    services_.clear();
}

void NetworkServiceLocator::provideAll() {
    // tcp.prefixed
    provideService(std::make_shared<FramingConnectionFactory>(std::unique_ptr<IStreamConnectionFactory>(new TCPConnectionFactory()),
                                                              std::unique_ptr<FramingStrategy>(new SizePrefixedProtocol())));

    // queue.prefixed
    provideService(std::make_shared<FramingConnectionFactory>(std::unique_ptr<IStreamConnectionFactory>(new QueueConnectionFactory()),
                                                              std::unique_ptr<FramingStrategy>(new SizePrefixedProtocol())));
}

std::unique_ptr<IMessageConnectionAcceptor> mocca::net::NetworkServiceLocator::bind(const Endpoint& endpoint) {
    auto serv = service(endpoint.protocol());
    return serv->bind(endpoint.address());
}

std::unique_ptr<IMessageConnection> mocca::net::NetworkServiceLocator::connect(const Endpoint& endpoint) {
    auto serv = service(endpoint.protocol());
    return serv->connect(endpoint.address());
}