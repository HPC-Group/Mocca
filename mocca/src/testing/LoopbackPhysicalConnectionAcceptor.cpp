#include "mocca/base/MessageQueue.h"
#include "mocca/testing/LoopbackPhysicalConnectionAcceptor.h"
#include "mocca/testing/LoopbackPhysicalConnection.h"

namespace mocca {
namespace net {

LoopbackPhysicalConnectionAcceptor::LoopbackPhysicalConnectionAcceptor(std::shared_ptr<LoopbackConnectionQueue> connectionQueue)
    : connectionQueue_(connectionQueue) {}

std::unique_ptr<IPhysicalConnection> LoopbackPhysicalConnectionAcceptor::getConnection(std::chrono::milliseconds timeout) {
    auto connectionNullable = connectionQueue_->tryDequeue(timeout);
    if (connectionNullable.isNull()) {
        return nullptr;
    }
    return connectionNullable.release();
}
}
}