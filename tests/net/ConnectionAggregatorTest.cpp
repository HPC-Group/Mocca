#include <future>

#include "gtest/gtest.h"

#include "mocca/base/ByteArray.h"
#include "mocca/net/ConnectionAggregator.h"
#include "mocca/net/LoopbackNetworkService_tmp.h"
#include "mocca/net/LoopbackConnectionAcceptor_tmp.h"
#include "mocca/net/TCPNetworkService.h"
#include "mocca/net/Error.h"
#include "mocca/net/MoccaNetworkService.h"

#include "mocca/testing/NetworkTesting.h"

using namespace mocca;
using namespace mocca::net;
using namespace mocca::testing;

#ifdef MOCCA_TEST_TCP
typedef ::testing::Types<LoopbackNetworkService_tmp, TCPNetworkService> MyTypes;
#else
typedef ::testing::Types<LoopbackNetworkService_tmp> MyTypes;
#endif
TYPED_TEST_CASE(ConnectionAggregatorTest, MyTypes);

template <typename T> class ConnectionAggregatorTest : public ::testing::Test {
protected:
    ConnectionAggregatorTest() {
        service.reset(new MoccaNetworkService(std::unique_ptr<IPhysicalNetworkService>(new T())));
    }

    virtual ~ConnectionAggregatorTest() {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    std::unique_ptr<IProtocolNetworkService> service;
};

TYPED_TEST(ConnectionAggregatorTest, EnqueueDequeue) {
    auto acceptor = service->bind(createBindingString<TypeParam>());
    ConnectionAggregator target(std::move(acceptor));
    auto clientConnection1 = service->connect(createConnectionString<TypeParam>());
    auto clientConnection2 = service->connect(createConnectionString<TypeParam>());

    ByteArray packet1, packet2;
    packet1 << "Hello 1";
    packet2 << "Hello 2";

    clientConnection1->send(packet1);
    clientConnection2->send(packet2);

    auto data1 = target.receive(std::chrono::milliseconds(200));
    auto data2 = target.receive(std::chrono::milliseconds(200));
    ASSERT_FALSE(data1.isNull());
    ASSERT_FALSE(data2.isNull());
    ByteArray recPacket1(data1.release().message);
    ByteArray recPacket2(data2.release().message);
    auto recStr1 = recPacket1.get<std::string>();
    auto recStr2 = recPacket2.get<std::string>();
    ASSERT_TRUE(recStr1 == "Hello 1" && recStr2 == "Hello 2" ||
                recStr1 == "Hello 2" && recStr2 == "Hello 1");
}

TYPED_TEST(ConnectionAggregatorTest, SendReceiveParallel) {
    TypeParam network;
    auto acceptor = service->bind(createBindingString<TypeParam>());
    ConnectionAggregator target(std::move(acceptor));
    auto clientConnection1 = service->connect(createConnectionString<TypeParam>());
    auto clientConnection2 = service->connect(createConnectionString<TypeParam>());

    const int numItems = 100;
    std::vector<std::string> data;
    for (int i = 0; i < numItems; i++) {
        data.push_back("item " + std::to_string(i));
    }

    auto sendFunction = [](IProtocolConnection& connection, const std::vector<std::string>& data) {
        for (auto item : data) {
            connection.send((ByteArray() << item));
            static int sleepTime = 0;
            sleepTime = (sleepTime + 1) % 3;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
    };
    auto clientFut1 =
        async(std::launch::async, sendFunction, std::ref(*clientConnection1), std::ref(data));
    auto clientFut2 =
        async(std::launch::async, sendFunction, std::ref(*clientConnection2), std::ref(data));

    auto receiveFunction = [](ConnectionAggregator& aggregator, int numItems) {
        std::vector<std::string> result;
        while (result.size() != numItems) {
            auto envelopeNullable = aggregator.receive(std::chrono::milliseconds(50));
            if (!envelopeNullable.isNull()) {
                auto envelope = envelopeNullable.release();
                ByteArray recPacket(envelope.message);
                result.push_back(recPacket.get<std::string>());
            }
        }
        return result;
    };
    auto serverFut = std::async(receiveFunction, std::ref(target), 2 * numItems);
    std::vector<std::string> result = serverFut.get();
    for (auto str : data) {
        ASSERT_EQ(2, std::count(begin(result), end(result), str));
    }
}

TYPED_TEST(ConnectionAggregatorTest, DisconnectStrategyThrowException) {
    auto acceptor = service->bind(createBindingString<TypeParam>());
    ConnectionAggregator target(std::move(acceptor),
                                ConnectionAggregator::DisconnectStrategy::ThrowException);
    {
        auto clientConnection = service->connect(createConnectionString<TypeParam>());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    bool exceptionCaught = false;
    for (int i = 0; i < 10; ++i) {
        try {
            target.receive(std::chrono::milliseconds(0));
        } catch (mocca::net::ConnectionClosedError) {
            exceptionCaught = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ASSERT_TRUE(exceptionCaught);
}

TYPED_TEST(ConnectionAggregatorTest, DisconnectStrategyRemoveConnection) {
    auto acceptor = service->bind(createBindingString<TypeParam>());
    ConnectionAggregator target(std::move(acceptor),
                                ConnectionAggregator::DisconnectStrategy::RemoveConnection);
    {
        auto clientConnection = service->connect(createConnectionString<TypeParam>());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    bool exceptionCaught = false;
    for (int i = 0; i < 10; ++i) {
        try {
            target.receive(std::chrono::milliseconds(0));
        } catch (mocca::net::ConnectionClosedError) {
            exceptionCaught = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ASSERT_FALSE(exceptionCaught);
}