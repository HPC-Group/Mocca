#include "mocca/net/IStreamConnection.h"

using namespace mocca;
using namespace mocca::net;

IStreamConnection::IStreamConnection()
    : putBackReadPos_(0) {}

ByteArray IStreamConnection::receive(uint32_t maxSize, std::chrono::milliseconds timeout) const {
    ByteArray result;
    if (!putBackData_.isEmpty()) {
        uint32_t numPutBackBytes = putBackData_.size() - putBackReadPos_;
        if (numPutBackBytes < maxSize) {
            result.append(putBackData_.data() + putBackReadPos_, numPutBackBytes);
            putBackData_.setSize(0);
            maxSize -= numPutBackBytes;
            putBackReadPos_ = 0;
        } else {
            result.append(putBackData_.data() + putBackReadPos_, maxSize);
            putBackReadPos_ += maxSize;
            return result;
        }
    }
    result.append(readFromStream(maxSize, timeout));
    return result;
}

void IStreamConnection::putBack(const ByteArray& data) {
    putBackData_.append(data);
}