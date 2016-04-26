/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#include "mocca/net/framing/FramingUtils.h"
#include "mocca/base/Thread.h"

#include <algorithm>

using namespace mocca;
using namespace mocca::net;

void mocca::net::readUntil(IStreamConnection& stream, std::vector<uint8_t>& buffer, const std::string& delim) {
    bool cont = true;
    uint8_t byte;
    while (cont) {
        if (Runnable::isCurrentInterrupted()) {
            throw ThreadInterrupt(__FILE__, __LINE__);
        }
        auto bytesReceived = stream.receive(&byte, 1);
        auto offset = buffer.size() - std::min(static_cast<uint32_t>(delim.size()), static_cast<uint32_t>(buffer.size()));
        buffer.push_back(byte);
        auto searchBegin = buffer.data() + offset;
        auto searchEnd = buffer.data() + buffer.size();
        auto searchIt = std::search(searchBegin, searchEnd, begin(delim), end(delim));
        if (searchIt != searchEnd) {
            cont = false;
        }
    }
}

void mocca::net::readExactly(IStreamConnection& stream, std::vector<uint8_t>& buffer, uint32_t size) {
    auto oldSize = buffer.size();
    buffer.resize(buffer.size() + size);
    auto startPos = buffer.data() + oldSize;
    uint32_t bytesTotal = 0;
    while (bytesTotal < size) {
        if (Runnable::isCurrentInterrupted()) {
            throw ThreadInterrupt(__FILE__, __LINE__);
        }
        auto bytesReceived = stream.receive(startPos + bytesTotal, size - bytesTotal);
        bytesTotal += bytesReceived;
    }
}

void mocca::net::sendAll(IStreamConnection& stream, const uint8_t* buffer, uint32_t size) {
    uint32_t bytesTotal = 0;
    while (bytesTotal < size) {
        if (Runnable::isCurrentInterrupted()) {
            throw ThreadInterrupt(__FILE__, __LINE__);
        }
        auto bytesSent = stream.send(buffer + bytesTotal, size - bytesTotal);
        bytesTotal += bytesSent;
    }
}