#include "pch.h"
#include "PoolAllocator.h"

PoolAllocator::PoolAllocator(size_t _totalSize, size_t _objectSize, size_t _alignment)
	: m_objectSize(_objectSize)
{
	m_alignment = _alignment;
	m_size = _totalSize;
	m_startPointer = (uintptr)::operator new(_totalSize);

	uint8 adjustment = PointerMath::GetForwardAdjustment(m_startPointer, _alignment);

	m_listHead = m_startPointer + adjustment;

	size_t objectCount = (m_size - adjustment) / _objectSize;

	void** listHeadPtr = (void**)m_listHead;
	for (size_t i = 0; i < objectCount - 1; i++)
	{
		*listHeadPtr = (void*)((uintptr)listHeadPtr + _objectSize);
		listHeadPtr = (void**)(*listHeadPtr);
	}

	*listHeadPtr = nullptr;
}

PoolAllocator::~PoolAllocator()
{
}

void* PoolAllocator::Allocate(size_t _size)
{
	if (m_objectSize != _size || m_listHead == 0)
	{
		return nullptr;
	}
	void** listHeadPtr = (void**)m_listHead;

	void* adress = (void*)listHeadPtr;
	m_listHead = (uintptr)(*(void**)listHeadPtr);

	m_usedMemory += _size;
	m_allcations++;

	return adress;
}

bool PoolAllocator::Deallocate(void* _ptr)
{
	void** listHeadPtr = (void**)m_listHead;

	*((void**)_ptr) = (void*)listHeadPtr;
	listHeadPtr = (void**)_ptr;

	m_listHead = (uintptr)listHeadPtr;

	m_usedMemory -= m_objectSize;
	m_allcations--;

	return true;
}
