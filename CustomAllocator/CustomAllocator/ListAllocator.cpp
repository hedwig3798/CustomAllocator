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

	// ����� �ݳ��� ��忡 ���� ������
	AllocatorHeader* headerPtr = (AllocatorHeader*)((uintptr)(_ptr)-sizeof(AllocatorHeader));
	uintptr nodeStart = (uintptr)(_ptr) - headerPtr->m_size;
	size_t dataSize = headerPtr->m_size;
	uintptr nodeEnd = nodeStart + dataSize;

	// ���� ���� ���� ���
	uintptr prevNode = 0;
	uintptr currNode = m_freeNodeHead;

	// �ݳ��ϴ� ����� ���� ��带 �����´�.
	while (currNode != 0 && currNode < nodeEnd)
	{
		prevNode = currNode;
		currNode = ((FreeNode*)currNode)->m_next;
	}

	// �ݳ��ϴ� ��� ���� ��尡 ���� ���� �����
	if (prevNode == 0)
	{
		prevNode = nodeStart;
		((FreeNode*)prevNode)->m_size = dataSize;
		((FreeNode*)prevNode)->m_next = currNode;

		currNode = prevNode;
	}
	// ���� ���� �ݳ� ��尡 �޸𸮻� �پ��ִٸ�
	else if (prevNode + ((FreeNode*)prevNode)->m_size == nodeStart)
	{
		((FreeNode*)prevNode)->m_size += dataSize;
	}
	// �� �� ��� ���
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

	// ã�� ��尡 ���ٸ� �Ұ����ϴٴ� ��
	if (nodePtr == nullptr)
	{
		return nullptr;
	}

	// ���� ����ȭ ������ ���� ��� ������ �Ұ����ϸ� �׳� �� ��带 �ش�.
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
	// ����� ������.
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

	// ��� ��带 ��ȸ�ϸ鼭 ���� ������ ��带 �����´�.
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

	// ã�� ��尡 ���ٸ� �Ұ����ϴٴ� ��
	if (nodePtr == nullptr)
	{
		return nullptr;
	}

	currNode = (uintptr)nodePtr;
	// ���� ����ȭ ������ ���� ��� ������ �Ұ����ϸ� �׳� �� ��带 �ش�.
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
	// ����� ������.
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
