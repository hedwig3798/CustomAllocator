#include "pch.h"
#include "Allocator.h"
#include <assert.h>

Allocator::Allocator()
	: m_startPointer(0)
	, m_memorySize(0)
	, m_usedMemory(0)
	, m_allocationCount(0)
	, m_alignment(8)
{

}

Allocator::~Allocator()
{
	assert(GetUsedMemory() == 0);
	if (m_startPointer != 0)
	{
		delete reinterpret_cast<void*>(m_startPointer);
		m_startPointer = 0;
	}
}
