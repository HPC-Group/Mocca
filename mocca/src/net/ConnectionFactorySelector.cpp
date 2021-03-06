/*The MIT License(MIT)

Copyright(c) 2016 David McCann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the
"Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include "mocca/net/ConnectionFactorySelector.h"

#include "mocca/base/Error.h"
#include "mocca/base/Memory.h"
#include "mocca/base/StringTools.h"
#include "mocca/net/framing/SizePrefixedProtocol.h"
#include "mocca/net/framing/WebSocketProtocol.h"
#include "mocca/net/message/FramingConnectionFactory.h"
#include "mocca/net/message/LoopbackConnectionFactory.h"
#include "mocca/net/stream/QueueConnectionFactory.h"
#include "mocca/net/stream/TCPConnectionFactory.h"

using namespace mocca::net;

std::vector<std::pair<std::string, std::unique_ptr<IMessageConnectionFactory>>> ConnectionFactorySelector::messageConnectionFactories_;
std::vector<std::pair<std::string, std::unique_ptr<IStreamConnectionFactory>>> ConnectionFactorySelector::streamConnectionFactories_;

IMessageConnectionFactory& ConnectionFactorySelector::messageConnectionFactory(const std::string& protocol) {
    for (auto& factory : messageConnectionFactories_) {
        if (factory.first == protocol) {
            return *factory.second;
        }
    }
    throw Error(mocca::formatString("Factory for protocol '%%' is not available", protocol), __FILE__, __LINE__);
}

IStreamConnectionFactory& ConnectionFactorySelector::streamConnectionFactory(const std::string& protocol) {
    for (auto& factory : streamConnectionFactories_) {
        if (factory.first == protocol) {
            return *factory.second;
        }
    }
    throw Error(mocca::formatString("Factory for protocol '%%' is not available", protocol), __FILE__, __LINE__);
}

void ConnectionFactorySelector::addMessageConnectionFactory(std::unique_ptr<IMessageConnectionFactory> factory,
                                                            const std::string& protocol) {
    for (auto& fac : messageConnectionFactories_) {
        if (fac.first == protocol) {
            throw Error(mocca::formatString("Factory for protocol '%%' already provided to selector", fac.first), __FILE__, __LINE__);
        }
    }
    messageConnectionFactories_.push_back({protocol, std::move(factory)});
}

void ConnectionFactorySelector::addStreamConnectionFactory(std::unique_ptr<IStreamConnectionFactory> factory, const std::string& protocol) {
    for (auto& fac : streamConnectionFactories_) {
        if (fac.first == protocol) {
            throw Error(mocca::formatString("Factory for protocol '%%' already provided to selector", fac.first), __FILE__, __LINE__);
        }
    }
    streamConnectionFactories_.push_back({protocol, std::move(factory)});
}

void ConnectionFactorySelector::removeAll() {
    messageConnectionFactories_.clear();
    streamConnectionFactories_.clear();
}

void ConnectionFactorySelector::addDefaultFactories() {
    messageConnectionFactories_.clear();
    streamConnectionFactories_.clear();

    // tcp
    addStreamConnectionFactory(mocca::make_unique<TCPConnectionFactory>(), tcp());

    // queue
    addStreamConnectionFactory(mocca::make_unique<QueueConnectionFactory>(), queue());

    // tcp.prefixed
    addMessageConnectionFactory(mocca::make_unique<FramingConnectionFactory>(mocca::make_unique<TCPConnectionFactory>(),
                                                                             mocca::make_unique<SizePrefixedProtocol>()),
                                tcpPrefixed());

    // queue.prefixed
    addMessageConnectionFactory(mocca::make_unique<FramingConnectionFactory>(mocca::make_unique<QueueConnectionFactory>(),
                                                                             mocca::make_unique<SizePrefixedProtocol>()),
                                queuePrefixed());

    // tcp.ws
    addMessageConnectionFactory(
        mocca::make_unique<FramingConnectionFactory>(mocca::make_unique<TCPConnectionFactory>(), mocca::make_unique<WebSocketProtocol>()),
        tcpWebSocket());

    // queue.ws
    addMessageConnectionFactory(
        mocca::make_unique<FramingConnectionFactory>(mocca::make_unique<QueueConnectionFactory>(), mocca::make_unique<WebSocketProtocol>()),
        queueWebSocket());

    // loopback
    addMessageConnectionFactory(mocca::make_unique<LoopbackConnectionFactory>(), loopback());
}

std::unique_ptr<IMessageConnectionAcceptor> ConnectionFactorySelector::bind(const Endpoint& endpoint) {
    auto& serv = messageConnectionFactory(endpoint.protocol);
    return serv.bind(endpoint.machine, endpoint.port);
}

std::unique_ptr<IMessageConnection> ConnectionFactorySelector::connect(const Endpoint& endpoint) {
    auto& serv = messageConnectionFactory(endpoint.protocol);
    return serv.connect(endpoint.address());
}

const std::string& ConnectionFactorySelector::tcp() {
    static std::string str = "tcp";
    return str;
}

const std::string& ConnectionFactorySelector::queue() {
    static std::string str = "queue";
    return str;
}

const std::string& ConnectionFactorySelector::loopback() {
    static std::string str = "loopback";
    return str;
}

const std::string& ConnectionFactorySelector::tcpPrefixed() {
    static std::string str = "tcp.prefixed";
    return str;
}

const std::string& ConnectionFactorySelector::tcpWebSocket() {
    static std::string str = "tcp.ws";
    return str;
}

const std::string& ConnectionFactorySelector::queuePrefixed() {
    static std::string str = "queue.prefixed";
    return str;
}

const std::string& ConnectionFactorySelector::queueWebSocket() {
    static std::string str = "queue.ws";
    return str;
}