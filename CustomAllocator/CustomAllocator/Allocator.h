#pragma once
#include "IAllocator.h"
#include "PointerMath.h"

class Allocator :
    public IAllocator
{
protected:
	uintptr m_startPointer;
	size_t m_memorySize;
	size_t m_usedMemory;
	size_t m_allocationCount;
	size_t m_alignment;

public:
	Allocator();
	virtual ~Allocator();

	virtual void* Allocate(size_t _size) abstract;
	virtual bool Deallocate(void*) abstract;


	virtual void* GetStartPointer() override { return reinterpret_cast<void*>(m_startPointer); };
	virtual size_t GetUsedMemory() override { return m_usedMemory; };
	virtual size_t GetAllocations() override { return m_allocationCount; };
	virtual size_t GetSize() override { return m_memorySize; };
};

