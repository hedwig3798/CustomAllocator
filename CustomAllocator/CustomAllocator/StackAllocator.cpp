#include "pch.h"
#include "StackAllocator.h"

StackAllocator::StackAllocator()
	: m_top(0)
	, m_base(0)
{

}

StackAllocator::~StackAllocator()
{

}

bool StackAllocator::Init(size_t _totalByte, size_t _alignment)
{
	void* temp = ::operator new(_totalByte);
	m_startPointer = reinterpret_cast<uintptr>(temp);

	m_alignment = _alignment;

	if (m_startPointer)
	{
		m_top = m_startPointer;
		m_base = m_top;
		return true;
	}

	return false;
}

void* StackAllocator::Allocate(size_t _size)
{
	if (_size == 0)
	{
		return nullptr;
	}

	uint8 adjustment = PointerMath::GetForwardAdjustment(m_top, m_alignment, sizeof(AllocatorHeader));

	if (m_usedMemory + adjustment + _size > m_size)
	{
		return nullptr;
	}

	uintptr adressPtr = (m_top + adjustment);

	uintptr headerPtr = adressPtr - sizeof(AllocatorHeader);

	AllocatorHeader* header = reinterpret_cast<AllocatorHeader*>(headerPtr);
	header->m_adjustment = adjustment;

	m_base = m_top;
	m_top = adressPtr + _size;
	m_usedMemory += _size + adjustment;
	m_allcations++;

	return nullptr;
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

	m_usedMemory -= m_top - ptr + header->m_adjustment;
	m_top = ptr - header->m_adjustment;
	m_allcations--;

	return true;
}
