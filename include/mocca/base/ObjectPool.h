/****************************************************************
* Copyright (C) 2016 Andrey Krekhov, David McCann
*
* The content of this file may not be copied and/or distributed
* without the expressed permission of the copyright owner.
*
****************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace mocca {

template <typename T> class ObjectPool {
public:
    ObjectPool(size_t initialSize)
        : initialSize_(initialSize) {
        for (size_t i = 0; i < initialSize_; ++i) {
            freeObjects_.push_back(new T());
        }
    }

    using ObjectPtr = std::unique_ptr<T, std::function<void(T*)>>;
    ObjectPtr getObject() {
        T* obj = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (freeObjects_.empty()) {
                for (size_t i = 0; i < initialSize_; ++i) {
                    freeObjects_.push_back(new T());
                }
            }
            obj = freeObjects_.back();
            freeObjects_.pop_back();
        }
        auto deleter = [this](T* obj) {
            obj->clear();
            std::unique_lock<std::mutex> lock(mutex_);
            freeObjects_.push_back(obj);
        };
        return std::unique_ptr<T, decltype(deleter)>(obj, deleter);
    }

    size_t numFreeObjects() {
        std::unique_lock<std::mutex> lock(mutex_);
        return freeObjects_.size();
    }

private:
    std::mutex mutex_;
    std::vector<T*> freeObjects_;
    size_t initialSize_;
};
}