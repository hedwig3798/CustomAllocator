#pragma once
#include "Allocator.h"

class PoolAllocator :
	public Allocator

{
private:
	size_t m_objectSize;
	uintptr m_listHead;

public:
	PoolAllocator(size_t _totalSize, size_t _objectSize, size_t _alignment);
	virtual ~PoolAllocator();

	virtual void* Allocate(size_t _size) override;
	virtual bool Deallocate(void* _ptr) override;
	virtual bool Clear() override;
};

