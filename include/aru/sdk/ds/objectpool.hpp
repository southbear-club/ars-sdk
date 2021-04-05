/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file objectpool.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace aru {

namespace sdk {

#define ARU_DEFAULT_OBJECT_POOL_INIT_NUM    0
#define ARU_DEFAULT_OBJECT_POOL_MAX_NUM     4
#define ARU_DEFAULT_OBJECT_POOL_TIMEOUT     3000 // ms

template<class T>
class ObjectFactory {
public:
    static T* create() {
        return new T;
    }
};

template<class T, class TFactory = ObjectFactory<T>>
class ObjectPool {
public:
    ObjectPool(
        int init_num = ARU_DEFAULT_OBJECT_POOL_INIT_NUM,
        int max_num = ARU_DEFAULT_OBJECT_POOL_MAX_NUM,
        int timeout = ARU_DEFAULT_OBJECT_POOL_TIMEOUT)
        : _max_num(max_num)
        , _timeout(timeout)
    {
        for (int i = 0; i < init_num; ++i) {
            T* p = TFactory::create();
            if (p) {
                objects_.push_back(std::shared_ptr<T>(p));
            }
        }
        _object_num = objects_.size();
    }

    ~ObjectPool() {}

    int ObjectNum() { return _object_num; }
    int IdleNum() { return objects_.size(); }
    int BorrowNum() { return ObjectNum() - IdleNum(); }

    std::shared_ptr<T> TryBorrow() {
        std::shared_ptr<T> pObj = NULL;
        std::lock_guard<std::mutex> locker(mutex_);
        if (!objects_.empty()) {
            pObj = objects_.front();
            objects_.pop_front();
        }
        return pObj;
    }

    std::shared_ptr<T> Borrow() {
        std::shared_ptr<T> pObj = TryBorrow();
        if (pObj) {
            return pObj;
        }

        std::unique_lock<std::mutex> locker(mutex_);
        if (_object_num < _max_num) {
            ++_object_num;
            // NOTE: unlock to avoid TFactory::create block
            mutex_.unlock();
            T* p = TFactory::create();
            mutex_.lock();
            if (!p) --_object_num;
            return std::shared_ptr<T>(p);
        }

        if (_timeout > 0) {
            std::cv_status status = cond_.wait_for(locker, std::chrono::milliseconds(_timeout));
            if (status == std::cv_status::timeout) {
                return NULL;
            }
            if (!objects_.empty()) {
                pObj = objects_.front();
                objects_.pop_front();
                return pObj;
            }
            else {
                // WARN: No idle object
            }
        }
        return pObj;
    }

    void Return(std::shared_ptr<T>& pObj) {
        if (!pObj) return;
        std::lock_guard<std::mutex> locker(mutex_);
        objects_.push_back(pObj);
        cond_.notify_one();
    }

    bool Add(std::shared_ptr<T>& pObj) {
        std::lock_guard<std::mutex> locker(mutex_);
        if (_object_num >= _max_num) {
            return false;
        }
        objects_.push_back(pObj);
        ++_object_num;
        cond_.notify_one();
        return true;
    }

    bool Remove(std::shared_ptr<T>& pObj) {
        std::lock_guard<std::mutex> locker(mutex_);
        auto iter = objects_.begin();
        while (iter !=  objects_.end()) {
            if (*iter == pObj) {
                iter = objects_.erase(iter);
                --_object_num;
                return true;
            }
            else {
                ++iter;
            }
        }
        return false;
    }

    void Clear() {
        std::lock_guard<std::mutex> locker(mutex_);
        objects_.clear();
        _object_num = 0;
    }

    int     _object_num;
    int     _max_num;
    int     _timeout;
private:
    std::list<std::shared_ptr<T>>   objects_;
    std::mutex              mutex_;
    std::condition_variable cond_;
};

template<class T, class TFactory = ObjectFactory<T>>
class PoolObject {
public:
    typedef ObjectPool<T, TFactory> PoolType;

    PoolObject(PoolType& pool) : pool_(pool)
    {
        sptr_ = pool_.Borrow();
    }

    ~PoolObject() {
        if (sptr_) {
            pool_.Return(sptr_);
        }
    }

    PoolObject(const PoolObject<T>&) = delete;
    PoolObject<T>& operator=(const PoolObject<T>&) = delete;

    T* get() {
        return sptr_.get();
    }

    operator bool() {
        return sptr_.get() != NULL;
    }

    T* operator->() {
        return sptr_.get();
    }

    T operator*() {
        return *sptr_.get();
    }

private:
    PoolType& pool_;
    std::shared_ptr<T> sptr_;
};

} // namespace sdk

} // namespace aru
