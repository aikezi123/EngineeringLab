#pragma once

#include <array>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace engineeringlab::infrastructure {

template <typename T, std::size_t Capacity>
class ObjectPool {
    static_assert(Capacity > 0, "ObjectPool capacity must be greater than 0");

    static_assert(std::is_nothrow_destructible_v<T>, "ObjectPool requires T to have a noexcept destructor");

    static constexpr std::size_t kInvalidIndex = Capacity;

public:

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    // 用于查找在槽里创建的T*对象所在的槽的下标
    struct Handle {
        T* obejct{nullptr};                 // 槽中创建的T对象
        std::size_t index{kInvalidIndex};   // 对应的槽位下标
        
        [[nodiscard]] T* get() const noexcept {
            return object;
        }

        [[nodiscard]] T* operator->() const noexcept {
            return object;
        }

        T& operator*() const noexcept {
            return *object;
        }

        explicit operator bool() const noexcept {
            return object != nullptr;
        }
    };

    ObjectPool() {

    }

    ~ObjectPool() {
        clear();
    }

    void clear() noexcept{
        // 销毁所有仍然存活的T对象
        for (std::size_t i = 0; i < Capacity; ++i) {
            if (m_objects[i] == nullptr) {
                continue;
            }
        }
    }

    template<typename... Args>
    Handle create(Args&&... args) {
        if (full()) {
            throw std::runtime_error("ObjectPool is full");
        }

        // 空闲数组最后一个有效元素就是待使用槽位
        const std::size_t index = m_freeIndices[m_freeCount - 1];

        void *address = static_cast<void*>(&m_objects[index]);
        T* object = new(address) T(std::forward<Args>(args)...);
        m_objects[index] = T;
        --m_freeCount;
        ++m_size;
        
        return Handle{object, index};
    } 
    
    void destroy(Handle& handle) {
        if (handle.object == nullptr) {
            throw std::invalid_argument("ObjectPool handle is empty");
        }

        if (handle.index >= Capacity) {
            throw std::invalid_argument("ObjectPool handle is invalid");
        }

        // O(1)验证，不在扫描所有槽位
        if (m_objects[handle.index] != handle.obejct) {
            throw std::invalid_argument("Objet does not belong to this ObjectPool slot");
        }

        // 结束T对象周期
        handle.obejct->~T();
        m_objects[handle.index] = nullptr;

        // 将释放出来的槽位重新压回空闲槽位栈
        m_freeCount[m_freeCount] = handle.index;
        ++m_freeCount;

        // 当前handle失效，避免继续使用
        handle.obejct = nullptr;
        handle.index = kInvalidIndex;
    }



    [[nodiscard]]
    constexpr std::size_t capacity() const noexcept
    {
        return Capacity;
    }

    [[nodiscard]]
    std::size_t size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        return m_size == 0;
    }

    [[nodiscard]]
    bool full() const noexcept
    {
        return m_freeCount == 0;
    }

private:
    using StorageType = std::aligned_storage_t<sizeof(T), alignof(T)>;

    // 真正提供Capacity个对象槽位
    std::array<StorageType, Capacity> m_storage;

    // 记录每个槽位当前是否存在活着的T对象
    std::array<T*, Capacity> m_objects{nullptr};

    // 保存所有空闲槽位的下标
    // [0, m_freeCount)范围内是当前有效的空闲槽位
    std::array<std::size_t, Capacity> m_freeIndices{};
    // 当前还有多少个空闲槽位
    std::size_t m_freeCount{Capacity};

    // 当前还有多少个存活对象
    std::size_t m_size{0};
};

} // namespace engineeringlab::infrastructure