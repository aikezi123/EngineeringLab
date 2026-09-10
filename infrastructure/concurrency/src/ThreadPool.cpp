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
    static_assert(
        Capacity > 0,
        "ObjectPool capacity must be greater than 0"
    );

    static_assert(
        std::is_nothrow_destructible_v<T>,
        "ObjectPool requires T to have a noexcept destructor"
    );

public:
    static constexpr std::size_t kInvalidIndex = Capacity;

    // 第四阶段的 Handle 只负责保存：
    // 1. T 对象地址
    // 2. 对应的槽位下标
    //
    // 当前还不是 RAII Handle，
    // 仍然需要手动调用 pool.destroy(handle)。
    struct Handle {
        T* object{nullptr};
        std::size_t index{kInvalidIndex};

        [[nodiscard]]
        T* get() const noexcept {
            return object;
        }

        T* operator->() const noexcept {
            return object;
        }

        T& operator*() const noexcept {
            return *object;
        }

        explicit operator bool() const noexcept {
            return object != nullptr;
        }
    };

public:
    ObjectPool() {
        // 初始化空闲槽位栈。
        //
        // Capacity = 4 时：
        //
        // m_freeIndices = [0][1][2][3]
        // m_freeCount   = 4
        //
        // [0, m_freeCount) 范围内的元素
        // 都表示当前空闲槽位。
        for (std::size_t i = 0; i < Capacity; ++i) {
            m_freeIndices[i] = i;
        }
    }

    ~ObjectPool() noexcept {
        clear();
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

public:
    template<typename... ConstructorArgs>
    Handle create(ConstructorArgs&&... args) {
        if (full()) {
            throw std::runtime_error("ObjectPool is full");
        }

        // 直接取得空闲槽位栈顶部的 index。
        // 不再遍历所有槽位，因此槽位查找为 O(1)。
        const std::size_t index =
            m_freeIndices[m_freeCount - 1];

        void* address =
            static_cast<void*>(&m_storage[index]);

        // 在已有槽位内存上构造 T。
        //
        // 先执行可能抛异常的构造操作，
        // 构造成功以后再修改 ObjectPool 状态。
        T* object = ::new (address) T(
            std::forward<ConstructorArgs>(args)...
        );

        m_occupied[index] = true;
        --m_freeCount;

        return Handle{object, index};
    }

    void destroy(Handle& handle) {
        if (handle.object == nullptr) {
            throw std::invalid_argument(
                "ObjectPool handle is empty"
            );
        }

        if (handle.index >= Capacity) {
            throw std::invalid_argument(
                "ObjectPool handle index is invalid"
            );
        }

        if (!m_occupied[handle.index]) {
            throw std::invalid_argument(
                "ObjectPool slot is not occupied"
            );
        }

        // 根据 index 直接定位槽位中的 T，
        // 不需要遍历，因此为 O(1)。
        T* object = objectAt(handle.index);

        // 检查 Handle 中的地址和槽位是否匹配。
        if (object != handle.object) {
            throw std::invalid_argument(
                "ObjectPool handle does not match slot"
            );
        }

        // 显式结束 T 的生命周期。
        object->~T();

        m_occupied[handle.index] = false;

        // 将释放出来的槽位重新压回空闲槽位栈。
        //
        // destroy 前：
        //
        // [0][1][2] ...
        //          ↑
        //      m_freeCount
        //
        // destroy Slot 3：
        //
        // [0][1][2][3]
        //             ↑
        //         ++m_freeCount
        m_freeIndices[m_freeCount] = handle.index;
        ++m_freeCount;

        // 当前 Handle 已经失效。
        handle.object = nullptr;
        handle.index = kInvalidIndex;
    }

    void clear() noexcept {
        // 找出所有当前存在活对象的槽位并进行析构。
        for (std::size_t i = 0; i < Capacity; ++i) {
            if (!m_occupied[i]) {
                continue;
            }

            objectAt(i)->~T();
            m_occupied[i] = false;
        }

        // 恢复初始空闲槽位栈。
        for (std::size_t i = 0; i < Capacity; ++i) {
            m_freeIndices[i] = i;
        }

        m_freeCount = Capacity;
    }

public:
    [[nodiscard]]
    static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return Capacity - m_freeCount;
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return m_freeCount == Capacity;
    }

    [[nodiscard]]
    bool full() const noexcept {
        return m_freeCount == 0;
    }

private:
    using StorageType =
        std::aligned_storage_t<sizeof(T), alignof(T)>;

    // 根据槽位下标获得当前存活在这块 Storage 上的 T*。
    T* objectAt(std::size_t index) noexcept {
        return std::launder(
            reinterpret_cast<T*>(&m_storage[index])
        );
    }

private:
    // 真正提供 Capacity 个 T 所需的底层原始存储。
    std::array<StorageType, Capacity> m_storage;

    // true：
    // m_storage[i] 上当前存在一个活着的 T。
    //
    // false：
    // m_storage[i] 当前只是可复用的裸存储。
    std::array<bool, Capacity> m_occupied{};

    // 空闲槽位下标栈。
    //
    // 只有 [0, m_freeCount) 范围内的数据有效。
    std::array<std::size_t, Capacity> m_freeIndices{};

    // 当前空闲槽位数量。
    // 同时也表示 m_freeIndices 的有效区域长度。
    std::size_t m_freeCount{Capacity};
};

} // namespace engineeringlab::infrastructure