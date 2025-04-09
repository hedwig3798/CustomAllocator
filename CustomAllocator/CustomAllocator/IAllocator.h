#pragma once

#ifdef ALLOCATOR_EXPORTS 
#define ALLOCATOR_DLL __declspec(dllexport)
#else
#define ALLOCATOR_DLL __declspec(dllimport)
#endif

class StackAllocator;

/// <summary>
/// �Ҵ��� �������̽� Ŭ����
/// </summary>
class IAllocator
{
public:
	IAllocator();
	virtual ~IAllocator();

	/// <summary>
	/// �Ҵ��� �ʱ�ȭ
	/// </summary>
	/// <param name="_totalByte">�Ҵ��ڰ� ���� �� �ִ� �޸� ũ��</param>
	/// <param name="_alignment">���İ�, �⺻ 8����Ʈ ����</param>
	/// <returns>���� ����</returns>
	virtual bool Init(size_t _totalByte, size_t _alignment) = 0;

	/// <summary>
	/// �޸� �Ҵ�
	/// </summary>
	/// <param name="_size">�Ҵ� �� �޸� ũ��</param>
	/// <returns>�Ҵ�� �޸� ���� �ּ�. ���н� nullptr</returns>
	virtual void* Allocate(size_t _size) = 0;

	/// <summary>
	/// �޸� ����
	/// </summary>
	/// <param name="">���� �� �޸�</param>
	/// <returns>���� ����</returns>
	virtual bool Deallocate(void * _ptr) = 0;

	virtual void* GetStartPointer() = 0;
	virtual size_t GetUsedMemory() = 0;
	virtual size_t GetAllocations() = 0;
	virtual size_t GetSize() = 0;
};

/// <summary>
/// ���� �Ҵ��� ����
/// </summary>
/// <returns> ���� �Ҵ���</returns>
extern "C" ALLOCATOR_DLL IAllocator* CreateStackAllocator();