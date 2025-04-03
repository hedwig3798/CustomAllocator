#pragma once

/// <summary>
/// �Ҵ��� �������̽� Ŭ����
/// </summary>
class IAllocator
{
public:
	IAllocator();
	virtual ~IAllocator();

	virtual bool Init(size_t _totalByte, size_t _alignment) abstract;
	virtual void* Allocate(size_t _size) abstract;
	virtual bool Deallocate(void *) abstract;

	virtual void* GetStartPointer() abstract;

	virtual size_t GetUsedMemory() abstract;
	virtual size_t GetAllocations() abstract;
	virtual size_t GetSize() abstract;
};

