#pragma once
#include "Allocator.h"

class StackAllocator :
    public Allocator
{
private:
	uintptr m_top;
	uintptr m_base;

	struct AllocatorHeader
	{
		uintptr m_base;
		uint8 m_adjustment;
	};

public:
	StackAllocator();
	virtual ~StackAllocator();

	virtual bool Init(size_t _totalByte, size_t _alignment);
	virtual void* Allocate(size_t _size);
	virtual bool Deallocate(void* _ptr);
};

