#include "pch.h"
#include "StackAllocator.h"

StackAllocator::StackAllocator(size_t _totalByte, size_t _alignment)
	: m_top(0)
	, m_base(0)
{
	void* temp = ::operator new(_totalByte);
	m_startPointer = reinterpret_cast<uintptr>(temp);

	m_alignment = _alignment;

	m_memorySize = _totalByte;

	if (m_startPointer)
	{
		m_top = m_startPointer;
		m_base = m_top;
	}
}

StackAllocator::~StackAllocator()
{

}

void* StackAllocator::Allocate(size_t _size)
{
	if (_size == 0)
	{
		return nullptr;
	}

	uint8 adjustment = PointerMath::GetForwardAdjustment(m_top, m_alignment, sizeof(AllocatorHeader));

	if (m_usedMemory + adjustment + _size > m_memorySize)
	{
		return nullptr;
	}

	uintptr adressPtr = (m_top + adjustment);

	uintptr headerPtr = adressPtr - sizeof(AllocatorHeader);

	AllocatorHeader* header = reinterpret_cast<AllocatorHeader*>(headerPtr);
	header->m_adjustment = adjustment;
	header->m_base = m_base;

	m_base = adressPtr;
	m_top = adressPtr + _size;
	m_usedMemory += _size + adjustment;
	m_allocationCount++;

	return reinterpret_cast<void*>(adressPtr);
}

bool StackAllocator::Deallocate(void* _ptr)
{
	uintptr ptr = reinterpret_cast<uintptr>(_ptr);
	if (ptr != m_base)
	{
		return false;
	}

	uintptr headerPtr = ptr - sizeof(AllocatorHeader);
	AllocatorHeader* header = reinterpret_cast<AllocatorHeader*>(headerPtr);

	m_base = header->m_base;
	m_usedMemory -= (m_top - ptr) + header->m_adjustment;
	m_top = ptr - header->m_adjustment;
	m_allocationCount--;

	return true;
}

bool StackAllocator::Clear()
{
	m_top = m_startPointer;
	m_base = m_top;
	m_allocationCount = 0;
	m_usedMemory = 0;
	return true;

}
