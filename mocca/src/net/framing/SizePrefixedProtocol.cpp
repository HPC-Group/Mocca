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

Message SizePrefixedProtocol::readMessageFromStream(IStreamConnection& connection) {
    std::lock_guard<std::mutex> lock(connection.receiveMutex());
    auto numParts = readValue<uint32_t>(connection);
    Message message;
    for (uint32_t i = 0; i < numParts; ++i) {
        auto size = readValue<uint32_t>(connection);
        auto buffer = std::make_shared<std::vector<uint8_t>>();
        readExactly(connection, *buffer, size);
        message.push_back(buffer);
    }
    return message;
}

void SizePrefixedProtocol::writeMessageToStream(IStreamConnection& connection, Message message) {
    std::lock_guard<std::mutex> lock(connection.sendMutex());
    sendValue(connection, static_cast<uint32_t>(message.size()));
    for (const auto& part : message) {
        sendValue(connection, static_cast<uint32_t>(part->size()));
        sendAll(connection, part->data(), part->size());
    }
}