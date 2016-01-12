/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#include "mocca/net/framing/SizePrefixedProtocol.h"

#include "mocca/net/framing/FramingUtils.h"

using namespace mocca;
using namespace mocca::net;

std::unique_ptr<FramingStrategy> mocca::net::SizePrefixedProtocol::clone() const {
    return std::unique_ptr<FramingStrategy>(new SizePrefixedProtocol(*this));
}

std::string SizePrefixedProtocol::name() const {
    return "prefixed";
}

ByteArray SizePrefixedProtocol::readFrameFromStream(IStreamConnection& connection, std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(connection.receiveMutex());

    ByteArray sizeBuffer;
    if (readExactly(connection, sizeBuffer, sizeof(uint32_t), timeout) == ReadStatus::Incomplete) {
        connection.putBack(sizeBuffer);
        return ByteArray();
    }

    ByteArray buffer;
    auto frameSize = sizeBuffer.read<uint32_t>();
    if (readExactly(connection, buffer, frameSize, timeout) == ReadStatus::Incomplete) {
        connection.putBack(buffer);
        connection.putBack(sizeBuffer);
        return ByteArray();
    }
    return buffer;
}

void SizePrefixedProtocol::writeFrameToStream(IStreamConnection& connection, ByteArray frame, std::chrono::milliseconds timeout) {
    // fixme: performance loss; implement prepend method for ByteArray
    std::lock_guard<std::mutex> lock(connection.sendMutex());
    ByteArray newFrame(frame.size() + sizeof(uint32_t));
    newFrame << frame.size();
    newFrame.append(frame);
    connection.send(std::move(newFrame));
}