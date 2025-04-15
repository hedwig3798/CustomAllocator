#include "pch.h"
#include "ListAllocator.h"
#include "PointerMath.h"

ListAllocator::ListAllocator(size_t _totalSize, size_t _alignment, uint8 _findMode)
	: m_findMode(_findMode)
{
	m_alignment = _alignment;
	m_memorySize = _totalSize;
	m_startPointer = (uintptr)::operator new(_totalSize);

	FreeNode* headPtr = (FreeNode*)(m_startPointer);
	headPtr->m_next = 0;
	headPtr->m_size = _totalSize;

	m_freeNodeHead = m_startPointer;
}

ListAllocator::~ListAllocator()
{

}

void* ListAllocator::Allocate(size_t _size)
{
	if (_size == 0)
	{
		return nullptr;
	}

	switch (m_findMode)
	{
	case 0:
		return FindFirstFit(_size);
		break;
	case 1:
		return FindBestFit(_size);
		break;
	default:
		return nullptr;
		break;
	}
}

bool ListAllocator::Deallocate(void* _ptr)
{
	if (_ptr == nullptr)
	{
		return false;
	}

	// 헤더와 반납할 노드에 대한 데이터
	AllocatorHeader* headerPtr = (AllocatorHeader*)((uintptr)(_ptr)-sizeof(AllocatorHeader));
	uintptr nodeStart = (uintptr)(_ptr) - headerPtr->m_size;
	size_t dataSize = headerPtr->m_size;
	uintptr nodeEnd = nodeStart + dataSize;

	// 이전 노드와 현재 노드
	uintptr prevNode = 0;
	uintptr currNode = m_freeNodeHead;

	// 반납하는 노드의 다음 노드를 가져온다.
	while (currNode != 0 && currNode < nodeEnd)
	{
		prevNode = currNode;
		currNode = ((FreeNode*)currNode)->m_next;
	}

	// 반납하는 노드 다음 노드가 현재 시작 노드라면
	if (prevNode == 0)
	{
		prevNode = nodeStart;
		((FreeNode*)prevNode)->m_size = dataSize;
		((FreeNode*)prevNode)->m_next = currNode;

		currNode = prevNode;
	}
	// 이전 노드와 반납 노드가 메모리상 붙어있다면
	else if (prevNode + ((FreeNode*)prevNode)->m_size == nodeStart)
	{
		((FreeNode*)prevNode)->m_size += dataSize;
	}
	// 그 외 모든 경우
	else
	{
		FreeNode* temp = (FreeNode*)nodeStart;
		temp->m_size = dataSize;
		temp->m_next = ((FreeNode*)prevNode)->m_next;
		((FreeNode*)prevNode)->m_next = (uintptr)temp;
		prevNode = (uintptr)temp;
	}


	if (m_freeNodeHead != 0 && m_freeNodeHead == nodeEnd)
	{
		((FreeNode*)prevNode)->m_size += ((FreeNode*)m_freeNodeHead)->m_size;
		((FreeNode*)prevNode)->m_next = ((FreeNode*)m_freeNodeHead)->m_next;
	}

	m_allocationCount--;
	m_usedMemory -= dataSize;

	return true;
}

bool ListAllocator::Clear()
{
	m_freeNodeHead = 0;
	m_usedMemory = 0;
	m_allocationCount = 0;
	return true;
}

void* ListAllocator::FindFirstFit(size_t _size)
{
	uintptr prevNode = 0;
	uintptr currNode = m_freeNodeHead;

	FreeNode* nodePtr = 0;
	size_t finalSize = 0;
	uint8 adjustment = 0;

	while (currNode != 0)
	{
		adjustment = PointerMath::GetForwardAdjustment(currNode, m_alignment, sizeof(AllocatorHeader));
		finalSize = _size + adjustment;

		nodePtr = (FreeNode*)currNode;
		if (nodePtr->m_size < finalSize)
		{
			prevNode = currNode;
			currNode = (uintptr)nodePtr->m_next;
			continue;
		}
		break;
	}

	// 찾은 노드가 없다면 불가능하다는 뜻
	if (nodePtr == nullptr)
	{
		return nullptr;
	}

	// 내부 단편화 방지를 위해 노드 분할이 불가능하면 그냥 그 노드를 준다.
	if (nodePtr->m_size - finalSize <= sizeof(AllocatorHeader))
	{
		finalSize = nodePtr->m_size;
		if (prevNode == 0)
		{
			m_freeNodeHead = nodePtr->m_next;
		}
		else
		{
			FreeNode* prevNodePtr = (FreeNode*)prevNode;
			prevNodePtr->m_next = nodePtr->m_next;
		}
	}
	// 블록을 나눈다.
	else
	{
		FreeNode* nextBlock = (FreeNode*)(currNode + finalSize);
		nextBlock->m_size = nodePtr->m_size - finalSize;
		nextBlock->m_next = nodePtr->m_next;

		if (prevNode == 0)
		{
			m_freeNodeHead = (uintptr)nextBlock;
		}
		else
		{
			FreeNode* prevNodePtr = (FreeNode*)prevNode;
			prevNodePtr->m_next = (uintptr)nextBlock;
		}
	}

	uintptr adress = currNode + adjustment;
	AllocatorHeader* headerPtr = (AllocatorHeader*)(adress - sizeof(AllocatorHeader));
	headerPtr->m_size = finalSize;

	m_usedMemory += finalSize;
	m_allocationCount++;

	return (void*)adress;
	return nullptr;
}

void* ListAllocator::FindBestFit(size_t _size)
{
	uintptr prevNode = 0;
	uintptr currNode = m_freeNodeHead;

	FreeNode* nodePtr = 0;
	size_t finalSize = 0;
	size_t nodeSize = m_memorySize + 1;
	uint8 adjustment = 0;

	// 모든 노드를 순회하면서 가장 적합한 노드를 가져온다.
	while (currNode != 0)
	{
		adjustment = PointerMath::GetForwardAdjustment(currNode, m_alignment, sizeof(AllocatorHeader));
		finalSize = _size + adjustment;

		FreeNode* temp = (FreeNode*)currNode;
		if (temp->m_size < finalSize)
		{
			prevNode = currNode;
			currNode = (uintptr)temp->m_next;
			continue;
		}

		if (temp->m_size < nodeSize)
		{
			nodePtr = temp;
			nodeSize = temp->m_size;
		}
		currNode = (uintptr)temp->m_next;
	}

	// 찾은 노드가 없다면 불가능하다는 뜻
	if (nodePtr == nullptr)
	{
		return nullptr;
	}

	currNode = (uintptr)nodePtr;
	// 내부 단편화 방지를 위해 노드 분할이 불가능하면 그냥 그 노드를 준다.
	if (nodePtr->m_size - finalSize <= sizeof(AllocatorHeader))
	{
		finalSize = nodePtr->m_size;
		if (prevNode == 0)
		{
			m_freeNodeHead = nodePtr->m_next;
		}
		else
		{
			FreeNode* prevNodePtr = (FreeNode*)prevNode;
			prevNodePtr->m_next = nodePtr->m_next;
		}
	}
	// 블록을 나눈다.
	else
	{
		FreeNode* nextBlock = (FreeNode*)(currNode + finalSize);
		nextBlock->m_size = nodePtr->m_size - finalSize;
		nextBlock->m_next = nodePtr->m_next;

		if (prevNode == 0)
		{
			m_freeNodeHead = (uintptr)nextBlock;
		}
		else
		{
			FreeNode* prevNodePtr = (FreeNode*)prevNode;
			prevNodePtr->m_next = (uintptr)nextBlock;
		}
	}

	uintptr adress = currNode + adjustment;
	AllocatorHeader* headerPtr = (AllocatorHeader*)(adress - sizeof(AllocatorHeader));
	headerPtr->m_size = finalSize;

	m_usedMemory += finalSize;
	m_allocationCount++;

	return (void*)adress;
}
