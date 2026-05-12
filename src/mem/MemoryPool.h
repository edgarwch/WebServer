#pragma once

#include <cstddef>
#include <mutex>
#include <utility>

struct Slot {
    Slot* next = nullptr;
};

class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    void init(size_t slotSize);

    Slot* allocate();
    void deallocate(Slot* slot);

private:
    static constexpr size_t kBlockSize = 4096;

    size_t padPointer(char* p, size_t align);
    Slot* allocateBlock();
    Slot* nofreeSlot();

    size_t slotSize_ = 0;

    Slot* freeSlot_ = nullptr;
    Slot* currentSlot_ = nullptr;
    Slot* currentBlock_ = nullptr;
    Slot* lastSlot_ = nullptr;

    std::mutex freeSlotMutex_;
    std::mutex allocMutex_;
};

MemoryPool& getMemoryPool(int id);
void initMemoryPools();

void* useMemory(size_t size);
void freeMemory(size_t size, void* p);

template<typename T, typename... Args>
T* NewElement(Args&&... args) {
    T* p = static_cast<T*>(useMemory(sizeof(T)));
    if (p) {
        new(p) T(std::forward<Args>(args)...);
    }
    return p;
}

template<typename T>
void DeleteElement(T* p) {
    if (p) {
        p->~T();
        freeMemory(sizeof(T), static_cast<void*>(p));
    }
}
