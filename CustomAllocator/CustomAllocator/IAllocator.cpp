#include "pch.h"
#include "IAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "ListAllocator.h"
#include <assert.h>

IAllocator::IAllocator()
{

}

IAllocator::~IAllocator()
{

}

ALLOCATOR_DLL IAllocator* CreateStackAllocator(size_t _totalByte, size_t _alignement)
{
	return new StackAllocator(_totalByte, _alignement);
}

ALLOCATOR_DLL IAllocator* CreatePoolAllocator(size_t _totalByte, size_t _objectSize, size_t _alignement)
{
	assert(_objectSize >= sizeof(void*));
	return new PoolAllocator(_totalByte, _objectSize, _alignement);
}

ALLOCATOR_DLL IAllocator* CreateListAllocator(size_t _totalByte, unsigned char _mode, size_t _alignement)
{
	return new ListAllocator(_totalByte, _alignement, _mode);
}
