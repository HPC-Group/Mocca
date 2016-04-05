#include "mocca/net/Message.h"

#include "mocca/base/Memory.h"

using namespace mocca::net;

Message::Message(std::shared_ptr<const std::vector<uint8_t>> data)
    : data_(data)
    , next_(nullptr) {}

Message::Message(Message&& other)
    : data_(other.data_)
    , next_(std::move(other.next_)) {}

Message* Message::append(std::shared_ptr<const std::vector<uint8_t>> data) {
    next_ = mocca::make_unique<Message>(data);
    return next_.get();
}

std::shared_ptr<const std::vector<uint8_t>> Message::data() const {
    return data_;
}

const Message* Message::next() const {
    return next_.get();
}

bool Message::isEmpty() const {
    return data_ == nullptr;
}