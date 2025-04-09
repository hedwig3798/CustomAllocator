#include "pch.h"
#include "IAllocator.h"
#include "StackAllocator.h"

IAllocator::IAllocator()
{

}

IAllocator::~IAllocator()
{

}

ALLOCATOR_DLL IAllocator* CreateStackAllocator()
{
	return new StackAllocator();
}
