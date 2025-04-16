#pragma once
#include "Allocator.h"
class ListAllocator :
    public Allocator
{
private:
	struct FreeNode
	{
		uintptr m_next;
		size_t m_size;
	};

	struct AllocatorHeader
	{
		size_t m_size;
		size_t m_adjustment;
	};

	uintptr m_freeNodeHead;

	uint8 m_findMode;
public:
	ListAllocator(size_t _totalSize, size_t _alignment, uint8 _findMode);
	virtual ~ListAllocator();

	void* Allocate(size_t _size) override;
	bool Deallocate(void* _ptr) override;
	virtual bool Clear() override;

private:
	void* FindFirstFit(size_t _size);
	void* FindBestFit(size_t _size);
};

