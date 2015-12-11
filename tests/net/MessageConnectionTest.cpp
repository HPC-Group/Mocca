#include "gtest/gtest.h"

#include "mocca/base/Error.h"
#include "mocca/base/Thread.h"
#include "mocca/base/ByteArray.h"
#include "mocca/net/message/FramingObjectFactory.h"
#include "mocca/testing/NetworkTesting.h"

#include <future>
#include <algorithm>

using namespace mocca;
using namespace mocca::net;
using namespace mocca::testing;

#ifdef MOCCA_TEST_TCP
typedef ::testing::Types<LoopbackPhysicalNetworkService, TCPNetworkService> MyTypes;
#else
typedef ::testing::Types<Prefixed_TCP_Factory, Prefixed_MessageQueue_Factory> MyTypes;
#endif
TYPED_TEST_CASE(NetworkServiceTest, MyTypes);

template<typename T>
class NetworkServiceTest : public ::testing::Test {
protected:
    NetworkServiceTest() {
        // You can do set-up work for each test here.
        target.reset(new T());
    }

    virtual ~NetworkServiceTest() {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    std::unique_ptr<T> target;
    using FactoryType = typename T::Stream_Object_Factory_Type;
};

TYPED_TEST(NetworkServiceTest, Identifier)
{
    {
        // identifier is not empty
        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
        auto clientConnection = this->target->connect(createAddress<FactoryType>());
        ASSERT_FALSE(clientConnection->identifier().empty());
        auto serverConnection = acceptor->accept();
        ASSERT_FALSE(serverConnection == nullptr);
        ASSERT_FALSE(serverConnection->identifier().empty());
    }
    {
        // identifiers are distinct
        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
        auto clientConnection1 = this->target->connect(createAddress<FactoryType>());
        auto clientConnection2 = this->target->connect(createAddress<FactoryType>());
        auto serverConnection1 = acceptor->accept();
        auto serverConnection2 = acceptor->accept();
        ASSERT_FALSE(serverConnection1 == nullptr);
        ASSERT_FALSE(serverConnection2 == nullptr);
        ASSERT_NE(clientConnection1->identifier(), clientConnection2->identifier());
        ASSERT_NE(clientConnection2->identifier(), serverConnection1->identifier());
        ASSERT_NE(serverConnection1->identifier(), serverConnection2->identifier());
    }
}

TYPED_TEST(NetworkServiceTest, AcceptorConnections) {
    {
        // cannot connect to an unbound port
        ASSERT_THROW(this->target->connect(createAddress<FactoryType>()), Error);
    }
    {
        // acceptor has no connection when no client connects
        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
        ASSERT_TRUE(acceptor->accept(std::chrono::milliseconds(1)) == nullptr);
    }
    {
        // acceptor has a connection when a client connects
        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
        auto connection = this->target->connect(createAddress<FactoryType>());
        ASSERT_FALSE(acceptor->accept() == nullptr);
    }
    {
        // acceptor has two conenctions when two clients connect
        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
        auto connection1 = this->target->connect(createAddress<FactoryType>());
        auto connection2 = this->target->connect(createAddress<FactoryType>());
        ASSERT_FALSE(acceptor->accept() == nullptr);
        ASSERT_FALSE(acceptor->accept() == nullptr);
    }
    {
        // connection is removed when dequeued
        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
        auto connection = this->target->connect(createAddress<FactoryType>());
        auto connectionFromAcceptor = acceptor->accept();
        ASSERT_FALSE(connectionFromAcceptor == nullptr);
        ASSERT_TRUE(acceptor->accept(std::chrono::milliseconds(1)) == nullptr);
    }
    {
        // each acceptor has its own queue
        auto acceptor1 = this->target->bind(createBindingAddress<FactoryType>());
        auto acceptor2 = this->target->bind(createBindingAddress<FactoryType>(1));
        auto connection = this->target->connect(createAddress<FactoryType>(1));
        ASSERT_TRUE(acceptor1->accept(std::chrono::milliseconds(1)) == nullptr);
        ASSERT_FALSE(acceptor2->accept() == nullptr);
    }
}
//
//TYPED_TEST(NetworkServiceTest, SendAndReceive) {
//    {
//        // client sends to server
//        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
//        auto clientConnection = this->target->connect(createAddress<TypeParam>());
//        auto serverConnection = acceptor->accept();
//        ASSERT_FALSE(serverConnection == nullptr);
//        clientConnection->send(mocca::makeFormattedByteArray("Hello World"));
//        auto recPacket = ByteArray(serverConnection->receive());
//        ASSERT_EQ("Hello World", std::get<0>(mocca::parseFormattedByteArray<std::string>(recPacket)));
//    }
//    {
//        // server sends to client
//        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
//        auto clientConnection = this->target->connect(createAddress<TypeParam>());
//        auto serverConnection = acceptor->accept();
//        ASSERT_FALSE(serverConnection == nullptr);
//        serverConnection->send(mocca::makeFormattedByteArray("Hello World"));
//        auto recPacket = ByteArray(clientConnection->receive());
//        ASSERT_EQ("Hello World", std::get<0>(mocca::parseFormattedByteArray<std::string>(recPacket)));
//    }
//    {
//        // two different clients and two receivers on the same port
//        auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
//        auto clientConnection1 = this->target->connect(createAddress<TypeParam>());
//        auto serverConnection1 = acceptor->accept();
//        ASSERT_FALSE(serverConnection1 == nullptr);
//        auto clientConnection2 = this->target->connect(createAddress<TypeParam>());
//        auto serverConnection2 = acceptor->accept();
//        ASSERT_FALSE(serverConnection2 == nullptr);
//        clientConnection1->send(mocca::makeFormattedByteArray("Hello from 1"));
//        clientConnection2->send(mocca::makeFormattedByteArray("Hello from 2"));
// 
//        auto recPacket1 = ByteArray(serverConnection1->receive());
//        ASSERT_EQ("Hello from 1", std::get<0>(mocca::parseFormattedByteArray<std::string>(recPacket1)));
//        auto recPacket2 = ByteArray(serverConnection2->receive());
//        ASSERT_EQ("Hello from 2", std::get<0>(mocca::parseFormattedByteArray<std::string>(recPacket2)));
//    }
//}
//
//TYPED_TEST(NetworkServiceTest, ReceiveTimeout) {
//    auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
//    auto clientConnection = this->target->connect(createAddress<TypeParam>());
//    auto serverConnection = acceptor->accept();
//    ASSERT_FALSE(serverConnection == nullptr);
//    auto recPacket = ByteArray(serverConnection->receive(std::chrono::milliseconds(10)));
//    ASSERT_TRUE(recPacket.isEmpty());
//}
//
//TYPED_TEST(NetworkServiceTest, ReceiveTimeoutClient) {
//    auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
//    auto clientConnection = this->target->connect(createAddress<TypeParam>());
//    auto serverConnection = acceptor->accept();
//    ASSERT_FALSE(serverConnection == nullptr);
//    auto recPacket = ByteArray(clientConnection->receive(std::chrono::milliseconds(10)));
//    ASSERT_TRUE(recPacket.isEmpty());
//}
//
//TYPED_TEST(NetworkServiceTest, SendReceiveParallel) {
//    auto acceptor = this->target->bind(createBindingAddress<FactoryType>());
//    auto clientConnection = this->target->connect(createAddress<TypeParam>());
//    auto serverConnection = acceptor->accept();
//    ASSERT_FALSE(serverConnection == nullptr);
//    
//    const int numItems = 200;
//    std::vector<std::string> data;
//    for (int i = 0; i < numItems; i++) {
//        data.push_back("item " + std::to_string(i));
//    }
//
//    mocca::AutoJoinThread a([&clientConnection, data] {
//        for (auto item : data) {
//            clientConnection->send(std::move(ByteArray() << item));
//        }
//    });
//
//    std::vector<std::future<std::string>> futures;
//    for (int i = 0; i < numItems; ++i) {
//        futures.push_back(std::async(std::launch::async, [&serverConnection, i]() {
//            auto recPacket = ByteArray(serverConnection->receive());
//            return recPacket.read(recPacket.size());
//        }));
//    }
//
//    std::vector<std::string> result;
//    for (int i = 0; i < numItems; i++) {
//        result.push_back(futures[i].get());
//    }
//    for (auto item : data) {
//        ASSERT_TRUE(std::find(begin(result), end(result), item) != end(result));
//    }
//}
