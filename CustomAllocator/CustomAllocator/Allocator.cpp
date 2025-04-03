#include "pch.h"
#include "Allocator.h"

Allocator::Allocator()
	: m_startPointer(0)
	, m_size(0)
	, m_usedMemory(0)
	, m_allcations(0)
	, m_alignment(8)
{

}

Allocator::~Allocator()
{
	if (m_startPointer != 0)
	{
		delete reinterpret_cast<void*>(m_startPointer);
		m_startPointer = 0;
	}
}
