#pragma once

#include <memory>
#include <vector>

namespace mocca {
namespace net {
class Message {
public:
    Message() = default;
    Message(std::shared_ptr<const std::vector<uint8_t>> data);
    Message(Message&& other);

    Message* append(std::shared_ptr<const std::vector<uint8_t>> data);

    std::shared_ptr<const std::vector<uint8_t>> data() const;
    const Message* next() const;

    bool isEmpty() const;

private:
    std::shared_ptr<const std::vector<uint8_t>> data_;
    std::unique_ptr<Message> next_;
};
}
}