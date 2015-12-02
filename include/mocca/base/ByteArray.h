#pragma once

#include "mocca/base/Error.h"
#include "mocca/base/Traits.h"

#include <cstring>
#include <memory>
#include <string>
#include <tuple>

#define MOCCA_BYTEARRAY_CHECKS

namespace mocca {

class ByteArray {
public:
    ByteArray(uint32_t capacity = 256);
    ByteArray(const ByteArray& other) = delete;
    ByteArray(ByteArray&& other);

    friend void swap(ByteArray& lhs, ByteArray& rhs);

    ByteArray clone();

    unsigned char* data();
    const unsigned char* data() const;

    uint32_t capacity() const;

    uint32_t size() const;
    void setSize(uint32_t size);
    bool isEmpty() const;

    static ByteArray createFromRaw(const void* raw, uint32_t size);
    void append(const void* data, uint32_t size);
    void append(const ByteArray& byteArray);

    ByteArray& operator<<(int16_t val);
    ByteArray& operator>>(int16_t& val);

    ByteArray& operator<<(uint16_t val);
    ByteArray& operator>>(uint16_t& val);

    ByteArray& operator<<(int32_t val);
    ByteArray& operator>>(int32_t& val);

    ByteArray& operator<<(uint32_t val);
    ByteArray& operator>>(uint32_t& val);

    ByteArray& operator<<(int64_t val);
    ByteArray& operator>>(int64_t& val);

    ByteArray& operator<<(uint64_t val);
    ByteArray& operator>>(uint64_t& val);

    static const unsigned char falseConst = 0x01;
    static const unsigned char trueConst = 0x02;
    ByteArray& operator<<(bool val);
    ByteArray& operator>>(bool& val);

    ByteArray& operator<<(float val);
    ByteArray& operator>>(float& val);

    ByteArray& operator<<(double val);
    ByteArray& operator>>(double& val);

    ByteArray& operator<<(const char* val);
    ByteArray& operator<<(const std::string& val);
    std::string read(uint32_t size);

    ByteArray& operator<<(const ByteArray& val);
    ByteArray& operator>>(ByteArray& val);

    unsigned char& operator[](uint32_t index);
    const unsigned char& operator[](uint32_t index) const;

    template <typename T> T read() {
        T val;
        *this >> val;
        return val;
    }

    void resetReadPos();

private:
    void resize(uint32_t newCapacity);

    std::unique_ptr<unsigned char[]> data_;
    uint32_t capacity_;
    uint32_t size_;
    uint32_t readPos_;
};


// tag-dispatch impementation details
namespace impl {
template <typename T> void makeFormattedByteArray(ByteArray& result, const T& value, std::false_type) {
    result << value;
}

template <typename T> void makeFormattedByteArray(ByteArray& result, const T& val, std::true_type) {
    std::string str(val);
    result << static_cast<uint32_t>(str.size());
    result.append(str.c_str(), static_cast<uint32_t>(str.size()));
}

template <typename T> std::tuple<T> parseFormattedByteArray(ByteArray& byteArray, std::true_type) {
    uint32_t size;
    byteArray >> size;
    std::string value = byteArray.read(size);
    return std::tuple<T>(value);
}

template <typename T> std::tuple<T> parseFormattedByteArray(ByteArray& byteArray, std::false_type) {
    T value;
    byteArray >> value;
    return std::tuple<T>(value);
}
} // end impl


template <typename T> void makeFormattedByteArray(ByteArray& result, const T& val) {
    impl::makeFormattedByteArray(result, val, typename isString<T>::type());
}

template <typename T, typename... Args> void makeFormattedByteArray(ByteArray& result, const T& value, const Args&... args) {
    makeFormattedByteArray(result, value);
    makeFormattedByteArray(result, args...);
}

template <typename T, typename... Args> ByteArray makeFormattedByteArray(const T& value, const Args&... args) {
    ByteArray result;
    makeFormattedByteArray(result, value, args...);
    return result;
}

template <typename T> std::tuple<T> parseFormattedByteArray(ByteArray& byteArray) {
    return impl::parseFormattedByteArray<T>(byteArray, typename isString<T>::type());
}

template <typename T, typename Arg, typename... Args> std::tuple<T, Arg, Args...> parseFormattedByteArray(ByteArray& byteArray) {
    auto value = parseFormattedByteArray<T>(byteArray);
    return std::tuple_cat(value, parseFormattedByteArray<Arg, Args...>(byteArray));
}
}