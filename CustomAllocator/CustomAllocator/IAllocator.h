#pragma once

/// <summary>
/// 할당자 인터페이스 클래스
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

