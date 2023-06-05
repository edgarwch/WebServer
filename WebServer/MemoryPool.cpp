#include "MemoryPool.h"


MemoryPool::MemoryPool(){
    _slotSize = 0;
    _freeSlot = NULL;
    _currentSlot = NULL;
    _firstBlock = NULL;
    _lastSlot = NULL;
}

MemoryPool::~MemoryPool(){
    // free all the memory
    Slot* cur = _firstBlock;
    while(cur){
        Slot* next = cur->next;
        // this will not call the destructor of the object, just free the memory
        operator delete(reinterpret_cast<void*>(cur));
        cur = next;
    }
}

void MemoryPool::init(int size){
    assert(size > 0);
    _slotSize = size;
}

inline size_t MemoryPool::padPointer(char* ptr, size_t align){
    size_t result = reinterpret_cast<size_t>(ptr);
    return ((align - result) & align);
}

Slot* MemoryPool::allocateBlock(){
    char* newBlock = reinterpret_cast<char*>(operator new(BLOCKSIZE));
    
    char* body = newBlock + sizeof(Slot*);
    size_t padding = padPointer(body, static_cast<size_t>(_slotSize));

    Slot* useSlot;
    {
        MutexLockGard Lock(_mutex_other);
        useSlot = _currentSlot;
        // slot size 8 16 32.....512
        _currentSlot += (_slotSize >> 3);
    }
    return useSlot;
}

Slot* MemoryPool::nofree_slove(){
    if(_currentSlot >= _lastSlot){
        return allocateBlock();
    }
    Slot* use;
    {
        MutexLockGard Lock(_mutex_other);
        use = _currentSlot;
        _currentSlot += (_slotSize >> 3);
    }
    return use;
}

Slot* MemoryPool::alloc(){
    if(_freeSlot){
        {
            MutexLockGard Lock(_mutex_free_slot);
            if(_freeSlot){
                Slot* res = _freeSlot;
                _freeSlot = _freeSlot->next;
                return res;
            }
        }
    }
    return nofree_slove();
}

inline void MemoryPool::dealloc(Slot* slot){
    if(slot){
        MutexLockGard Lock(_mutex_free_slot);
        slot->next = _freeSlot;
        _freeSlot = slot;
    }
}

MemoryPool& get_MemoryPool(int id){
    static MemoryPool memp[64];
    return memp[id];
}


void Init_MemoryPool(){
    for(int i = 0; i < 64; i++){
        get_MemoryPool(i).init((i+1) << 3);
    }
}

void* Use_Memory(size_t size){
    if(!size){
        return nullptr;
    }
    if(size > 512){
        return operator new(size);
    }
    return reinterpret_cast<void*>(get_MemoryPool(((size + 7) >> 3) - 1).alloc());

}
void Free_Memory(size_t size, void* p){
    if(!p) return;
    if(size > 32){
        operator delete (p);
        return;
    }
    get_MemoryPool(((size+7) >> 3) - 1).dealloc(reinterpret_cast<Slot*>(p));
}




