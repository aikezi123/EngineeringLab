#pragma once
#include <cstddef>
#include <stdexcept>
#include <array>
#include <type_traits>
#include <memory>
#include <new>
#include <utility>
#include <cassert>

namespace engineeringlab::infrastructure {

template<typename T, std::size_t Capacity>
class ObjectPool {
    static_assert(Capacity > 0, "ObjectPool capacity must greater than 0");
    static_assert(std::is_nothrow_destructible_v<T>, "ObjectPool requirs T to have a noexcept destructor");
public:
    ObjectPool() {
        for (std::size_t i = 0; i < Capacity; ++i) {
            m_freeSlots[i] = i;
        }
    }

    ~ObjectPool() noexcept {
        // 所有Handle必须先于ObjectPool析构
        assert(empty() && "ObjectPool destroyed while Handles are still alive");
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

public:
    struct Deleter {
        ObjectPool* pool{nullptr};
        std::size_t index{Capacity};
        
        // std::unique_ptr<T, Deleter>,在析构时会默认调用Deleter deleter; deleter(pointer);也就是这个可回调函数。
        void operator()(T* object) const noexcept {
            if (object == nullptr) {
                return;
            }
            assert(pool != nullptr);
            assert(index < Capacity);

            pool->release(object, index);
        }
    };
    using Handle = std::unique_ptr<T, Deleter>;

    // 在槽上创建一个T对象，RAII控制，自动析构，析构时直接调用Deleter的可回调函数
    template<typename... Args>
    Handle create(Args&&... args) {
        if (full()) {
            throw std::runtime_error("ObjectPool is already full");
        }
        // 先从m_freeSlots里取出一个空闲槽的下标
        std::size_t index = m_freeSlots[m_freeCounts - 1];
        // 从m_storages里取一个空闲槽的地址
        void* address = static_cast<void*>(&m_storages[index]);
        // 在这个槽里placement new一个对象
        T* object = new(address) T(std::forward<Args>(args)...);
        // 空闲槽位-1,对应的槽位置为true
        m_used[index] = true;
        --m_freeCounts;
        
        return Handle{object, Deleter{this, index}};
    }

private:
    // 只有Deleter使用此函数
    void release(T* object, std::size_t index) noexcept{
        // 这些条件如果失败，说明ObjectPool自己内部状态出现了bug
        assert(object != nullptr);
        assert(index < Capacity);
        assert(m_used[index]);
        assert(objectAt(index) == object);
        assert(m_freeCounts < Capacity);
        
        // 只析构对象，不释放内存
        object->~T();
        m_used[index] = false;

        // 空闲槽压回栈里
        m_freeSlots[m_freeCounts++] = index;
    }

    // 根据Slot Index重新取得当前存活的T*
    T* objectAt(std::size_t index) noexcept {
        return std::launder(reinterpret_cast<T*>(&m_storages[index]));
    }

public:
    [[nodiscard]]
    static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return Capacity - m_freeCounts;
    }

    [[nodiscard]]
    bool full() const noexcept {
        return m_freeCounts == 0;
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return m_freeCounts == Capacity;
    }

private:
    // 提供原始内存槽的类型
    using StorageType = std::aligned_storage_t<sizeof(T), alignof(T)>;
    // 提供ObjectPool原始内存槽位数组
    std::array<StorageType, Capacity> m_storages;
    // 管理m_storages的槽位是否被使用，下标一一对应
    std::array<bool, Capacity> m_used{};

    // 维护空闲槽位下标的数组，空闲槽位不一定按顺序排列
    // [0, m_freeCounts)为维护的空闲数组
    // m_freeSlots和m_freeCounts在功能上其实形成了一个栈结构
    std::array<std::size_t, Capacity> m_freeSlots;
    std::size_t m_freeCounts{Capacity};

};

} // engineeringlab::infrastructure