#pragma once
#include <array>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace engineeringlab::infrastructure {
template<typename T, std::size_t Capacity>
class ObjectPool {
public:
    static_assert(
        Capacity > 0,
        "ObjectPool capacity must be greater than 0"
    );
    
    ObjectPool() = default;
    ~ObjectPool() {
        clear();
    }
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    template<typename... ConstructorArgs>
    T* create(ConstructorArgs&&... args) {
        if (full()) {
            throw std::runtime_error("ObjectPool is full");
        }

        for(std::size_t i = 0; i < Capacity; ++i) {
            // 查询下标i的槽位是否已满，如果满了直接continue
            if (m_objects[i] != nullptr) {
                continue;
            }

            // 未满槽位，则在这个槽位创建placement new一个对象
            void* address = static_cast<void*>(&m_storage[i]);
            T* object = new(address) T(std::forward<ConstructorArgs>(args)...);
            m_objects[i] = object;
            ++m_size;
            return object;
        }
        throw std::runtime_error("ObjectPool internal state error");
    }

    void destroy(T* object) {
        if (object == nullptr) {
            throw std::invalid_argument("object is nullptr");
        }
        for (std::size_t i = 0; i < Capacity; ++i) {
            if (m_objects[i] == object) {
                m_objects[i]->~T();
                m_objects[i] = nullptr;
                --m_size;
                return;
            }
        }
        throw std::invalid_argument("Object does not belong to this ObjectPool");
    }

    void clear() {
        if (empty()) {
            return;
        }
        for (std::size_t i = 0; i < Capacity; ++i) {
            if (m_objects[i] == nullptr) {
                continue;
            }
            m_objects[i]->~T();
            m_objects[i] = nullptr;
            --m_size;
        }

    }



    [[nodiscard]] constexpr std::size_t capacity() const noexcept {
        return Capacity;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return m_size;
    }

    [[nodiscard]]bool empty() const noexcept {
        return m_size == 0;
    }

    [[nodiscard]]bool full() const noexcept {
        return m_size == Capacity;
    }

private:
    using StorageType = std::aligned_storage_t<sizeof(T), alignof(T)>;
    // 真正提供Capacity个内存槽位
    std::array<StorageType, Capacity> m_storage;
    // 记录每个槽位是否有对象
    std::array<T*, Capacity> m_objects{nullptr};
    // 当前槽位有多少个对象
    std::size_t m_size{0};
};

} // engineeringlab::infrastructure