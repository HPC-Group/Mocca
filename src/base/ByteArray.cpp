#include "mocca/base/ByteArray.h"

#include <limits>

static_assert(std::numeric_limits<float>::is_iec559 == true, "Unsupported floating point type");
static_assert(std::numeric_limits<double>::is_iec559 == true, "Unsupported floating point type");

namespace mocca {

const unsigned char ByteArray::trueConst;
const unsigned char ByteArray::falseConst;

ByteArray::ByteArray(uint32_t capacity)
    : data_(new char[capacity])
    , capacity_(capacity)
    , size_(0)
    , readPos_(0) {}

ByteArray::ByteArray(const ByteArray& other)
    : data_(new char[other.capacity_])
    , capacity_(other.capacity_)
    , size_(other.size_)
    , readPos_(other.readPos_) {
    memcpy(data_.get(), other.data_.get(), other.size_);
}

mocca::ByteArray::ByteArray(ByteArray&& other)
    : data_(std::move(other.data_))
    , capacity_(other.capacity_)
    , size_(other.size_)
    , readPos_(other.readPos_) {
    other.capacity_ = 0;
    other.size_ = 0;
    other.readPos_ = 0;
}

ByteArray& mocca::ByteArray::operator=(ByteArray other) {
    ByteArray tmp(std::move(other));
    std::swap(tmp, *this);
    return *this;
}

char* ByteArray::data() {
    return data_.get();
}

const char* ByteArray::data() const {
    return data_.get();
}

uint32_t ByteArray::size() const {
    return size_;
}

void ByteArray::setSize(uint32_t size) {
    if (size > capacity_) {
        throw Error("Size exceeds capacity of the byte array", __FILE__, __LINE__);
    }
    size_ = size;
}

bool ByteArray::isEmpty() const {
    return size_ == 0;
}

uint32_t ByteArray::capacity() const {
    return capacity_;
}

void ByteArray::resize(uint32_t newCapacity) {
    auto newData = std::unique_ptr<char[]>(new char[newCapacity]);
    memcpy(newData.get(), data_.get(), size_);
    data_ = std::move(newData);
    capacity_ = newCapacity;
}

void ByteArray::append(const void* data, uint32_t size) {
    if (capacity_ < size_ + size) {
        resize(size_ + 2 * size);
    }
    memcpy(data_.get() + size_, data, size);
    size_ += size;
}

ByteArray ByteArray::createFromRaw(const void* raw, uint32_t size) {
    auto byteArray = ByteArray(size);
    memcpy(byteArray.data_.get(), raw, size);
    byteArray.size_ = size;
    return byteArray;
}

ByteArray& mocca::ByteArray::operator<<(int32_t val) {
    append(&val, sizeof(int32_t));
    return *this;
}

ByteArray& mocca::ByteArray::operator>>(int32_t& val) {
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + sizeof(int32_t) > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    memcpy(&val, data_.get() + readPos_, sizeof(int32_t));
    readPos_ += sizeof(int32_t);
    return *this;
}

ByteArray& mocca::ByteArray::operator<<(uint32_t val) {
    return (*this << (int32_t)val);
}

ByteArray& mocca::ByteArray::operator>>(uint32_t& val) {
    return (*this >> (int32_t&)val);
}

ByteArray& mocca::ByteArray::operator<<(int64_t val) {
    append(&val, sizeof(int64_t));
    return *this;
}

ByteArray& mocca::ByteArray::operator>>(int64_t& val) {
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + sizeof(int64_t) > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    memcpy(&val, data_.get() + readPos_, sizeof(int64_t));
    readPos_ += sizeof(int64_t);
    return *this;
}

ByteArray& mocca::ByteArray::operator<<(uint64_t val) {
    return (*this << (int64_t)val);
}

ByteArray& mocca::ByteArray::operator>>(uint64_t& val) {
    return (*this >> (int64_t&)val);
}

ByteArray& mocca::ByteArray::operator<<(bool val) {
    if (val) {
        append(&trueConst, sizeof(unsigned char));
    } else {
        append(&falseConst, sizeof(unsigned char));
    }
    return *this;
}

ByteArray& mocca::ByteArray::operator>>(bool& val) {
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + sizeof(unsigned char) > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    unsigned char code;
    memcpy(&code, data_.get() + readPos_, sizeof(unsigned char));
#ifdef MUI_BYTEARRAY_CHECKS
    if (code != trueConst && code != falseConst) {
        throw Error("Package corrupted", __FILE__, __LINE__);
    }
#endif
    val = (code & trueConst) != 0;
    readPos_ += sizeof(unsigned char);
    return *this;
}

ByteArray& mocca::ByteArray::operator<<(float val) {
    append(&val, sizeof(float));
    return *this;
}

ByteArray& mocca::ByteArray::operator>>(float& val) {
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + sizeof(float) > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    memcpy(&val, data_.get() + readPos_, sizeof(float));
    readPos_ += sizeof(float);
    return *this;
}

ByteArray& mocca::ByteArray::operator<<(double val) {
    append(&val, sizeof(double));
    return *this;
}

ByteArray& mocca::ByteArray::operator>>(double& val) {
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + sizeof(double) > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    memcpy(&val, data_.get() + readPos_, sizeof(double));
    readPos_ += sizeof(double);
    return *this;
}

ByteArray& ByteArray::operator<<(const std::string& val) {
    *this << (uint32_t)val.size();
    append(val.c_str(), (uint32_t)val.size());
    return *this;
}

ByteArray& ByteArray::operator>>(std::string& val) {
    uint32_t strSize;
    *this >> strSize;
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + strSize > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    val.reserve(strSize);
    val = std::string(data_.get() + readPos_, strSize);
    readPos_ += strSize;
    return *this;
}

ByteArray& mocca::ByteArray::operator<<(const char* val) {
    return (*this << std::string(val));
}

ByteArray& mocca::ByteArray::operator<<(const ByteArray& val) {
    *this << (int32_t)val.size();
    append(val.data(), val.size());
    return *this;
}

ByteArray& mocca::ByteArray::operator>>(ByteArray& val) {
    int32_t innerSize;
    *this >> innerSize;
#ifdef MUI_BYTEARRAY_CHECKS
    if (readPos_ + innerSize > size_) {
        throw Error("Reading beyond end of packet", __FILE__, __LINE__);
    }
#endif
    val.append(data_.get() + readPos_, innerSize);
    readPos_ += innerSize;
    return *this;
}

void mocca::ByteArray::resetReadPos() {
    readPos_ = 0;
}
}