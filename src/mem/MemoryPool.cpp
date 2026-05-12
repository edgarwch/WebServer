#include "MemoryPool.h"
#include <cassert>
#include <cstddef>

MemoryPool::MemoryPool() = default;

MemoryPool::~MemoryPool() {
    Slot* cur = currentBlock_;
    while (cur) {
        Slot* next = cur->next;
        ::operator delete(static_cast<void*>(cur));
        cur = next;
    }
}

void MemoryPool::init(size_t size) {
    assert(size > 0);
    slotSize_ = size;
    freeSlot_ = nullptr;
    currentSlot_ = nullptr;
    currentBlock_ = nullptr;
    lastSlot_ = nullptr;
}

size_t MemoryPool::padPointer(char* p, size_t align) {
    size_t result = reinterpret_cast<size_t>(p);
    return ((align - result) % align);
}

Slot* MemoryPool::allocateBlock() {
    auto* newBlock = static_cast<char*>(::operator new(kBlockSize));
    char* body = newBlock + sizeof(Slot*);
    size_t padding = padPointer(body, slotSize_);

    Slot* useSlot;
    {
        std::lock_guard lock(allocMutex_);
        reinterpret_cast<Slot*>(newBlock)->next = currentBlock_;
        currentBlock_ = reinterpret_cast<Slot*>(newBlock);
        currentSlot_ = reinterpret_cast<Slot*>(body + padding);
        lastSlot_ = reinterpret_cast<Slot*>(newBlock + kBlockSize - slotSize_ + 1);
        useSlot = currentSlot_;
        // Advance by slotSize_ in units of Slot*
        currentSlot_ += (slotSize_ >> 3);
    }
    return useSlot;
}

Slot* MemoryPool::nofreeSlot() {
    if (currentSlot_ >= lastSlot_) {
        return allocateBlock();
    }
    Slot* use;
    {
        std::lock_guard lock(allocMutex_);
        use = currentSlot_;
        currentSlot_ += (slotSize_ >> 3);
    }
    return use;
}

Slot* MemoryPool::allocate() {
    if (freeSlot_) {
        std::lock_guard lock(freeSlotMutex_);
        if (freeSlot_) {
            Slot* result = freeSlot_;
            freeSlot_ = freeSlot_->next;
            return result;
        }
    }
    return nofreeSlot();
}

void MemoryPool::deallocate(Slot* slot) {
    if (slot) {
        std::lock_guard lock(freeSlotMutex_);
        slot->next = freeSlot_;
        freeSlot_ = slot;
    }
}

MemoryPool& getMemoryPool(int id) {
    static MemoryPool pools[64];
    return pools[id];
}

void initMemoryPools() {
    for (int i = 0; i < 64; ++i) {
        getMemoryPool(i).init(static_cast<size_t>(i + 1) << 3);
    }
}

void* useMemory(size_t size) {
    if (size == 0) return nullptr;
    if (size > 512) return ::operator new(size);
    // Round up to nearest multiple of 8, map to pool index
    return static_cast<void*>(getMemoryPool(static_cast<int>(((size + 7) >> 3) - 1)).allocate());
}

void freeMemory(size_t size, void* p) {
    if (!p) return;
    if (size > 512) {
        ::operator delete(p);
        return;
    }
    getMemoryPool(static_cast<int>(((size + 7) >> 3) - 1)).deallocate(static_cast<Slot*>(p));
}
