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

#pragma once

#include "mocca/base/Error.h"

namespace mocca {

/* A Nullable object represents an optional value. The value is either set or null. Nullable objects
 * offer a value semantics alternative to null pointers. Use Nullable objects instead of special
 * values (e.g., magic numbers or empty strings) to represent missing values. */
template <typename T> class Nullable {
public:
    Nullable()
        : value_{}
        , isNull_(true) {}

    // copy and move
    Nullable(const Nullable& other)
        : value_(other.value_)
        , isNull_(other.isNull_) {}
    Nullable(Nullable&& other)
        : value_(std::move(other.value_))
        , isNull_(other.isNull_) {
        other.isNull_ = true;
    }
    Nullable& operator=(Nullable other) {
        Nullable tmp(std::move(other));
        swap(tmp, *this);
        return *this;
    }

    // copy and move from T
    Nullable(const T& value)
        : value_(value)
        , isNull_(false) {}
    Nullable(T&& value)
        : value_(std::move(value))
        , isNull_(false) {}
    Nullable& operator=(T value) {
        Nullable tmp(std::move(value));
        swap(tmp, *this);
        return *this;
    }

    friend void swap(Nullable& first, Nullable& second) {
        using std::swap;
        swap(first.value_, second.value_);
        swap(first.isNull_, second.isNull_);
    }

    friend bool operator==(const Nullable<T>& lhs, const Nullable<T>& rhs) {
        return (lhs.isNull() && rhs.isNull()) || (!lhs.isNull_ && !rhs.isNull_ && lhs.value_ == rhs.value_);
    }
    friend bool operator==(const Nullable<T>& lhs, const T& rhs) { return !lhs.isNull_ && lhs.value_ == rhs; }
    friend bool operator==(const T& lhs, const Nullable<T>& rhs) { return !rhs.isNull_ && lhs == rhs.value_; }

    T release() {
        if (!isNull_) {
            T tmp(std::move(value_));
            isNull_ = true;
            return std::move(tmp); // not necessary, but vs2013 complains
        } else {
            throw Error("Value of Nullable not set", __FILE__, __LINE__);
        }
    }

    const T& get() const {
        if (!isNull_) {
            return value_;
        } else {
            throw Error("Value of Nullable not set", __FILE__, __LINE__);
        }
    }

    T getOr(const T& alt) const {
        if (!isNull_) {
            return value_;
        } else {
            return alt;
        }
    }

    operator const T&() const { return get(); }

    bool isNull() const { return isNull_; }

private:
    T value_;
    bool isNull_;
};
}