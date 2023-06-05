#pragma once
#define BLOCKSIZE 4096
#include "../utils/noncopyable.h"
#include "../utils/MutexLock.h"
#include <cassert>
#include <utility>

struct Slot{
    Slot* next;
};

class MemoryPool:noncopyable{
    public:
        MemoryPool();
        ~MemoryPool();

        void init(int size);

        Slot* alloc();
        void dealloc(Slot* slot);

    private:
        int _slotSize;
        
        Slot* _freeSlot;
        Slot* _currentSlot;
        Slot* _firstBlock;
        Slot* _lastSlot;

        MutexLock _mutex_free_slot;;
        MutexLock _mutex_other;

        size_t padPointer(char* ptr, size_t align);;
        Slot* allocateBlock();
        Slot* nofree_slove();

};

MemoryPool& get_MemoryPool(int id);

void Init_MemoryPool();

void* Use_Memory(size_t size);
void Free_Memory(size_t size, void* p);

template<typename T, typename... Args>
T* NewElement(Args&&... args){
    T* p;
    if((p = reinterpret_cast<T*>(Use_Memory(sizeof(T))))){
        new(p) T(std::forward<Args>(args)...);
    }
    return p;
}

template<typename T>
void DeleteElement(T *p){
    if(p){
        p->~T();
    }
    Free_Memory(sizeof(T), reinterpret_cast<void*>(p));
}