#pragma once

#ifdef ALLOCATOR_EXPORTS 
#define ALLOCATOR_DLL __declspec(dllexport)
#else
#define ALLOCATOR_DLL __declspec(dllimport)
#endif

class StackAllocator;

/// <summary>
/// 할당자 인터페이스 클래스
/// </summary>
class IAllocator
{
public:
	IAllocator();
	virtual ~IAllocator();

	/// <summary>
	/// 할당자 초기화
	/// </summary>
	/// <param name="_totalByte">할당자가 가질 수 있는 메모리 크기</param>
	/// <param name="_alignment">정렬값, 기본 8바이트 정렬</param>
	/// <returns>성공 여부</returns>
	virtual bool Init(size_t _totalByte, size_t _alignment) = 0;

	/// <summary>
	/// 메모리 할당
	/// </summary>
	/// <param name="_size">할당 할 메모리 크기</param>
	/// <returns>할당된 메모리 시작 주소. 실패시 nullptr</returns>
	virtual void* Allocate(size_t _size) = 0;

	/// <summary>
	/// 메모리 해제
	/// </summary>
	/// <param name="">해제 할 메모리</param>
	/// <returns>성공 여부</returns>
	virtual bool Deallocate(void * _ptr) = 0;

	virtual void* GetStartPointer() = 0;
	virtual size_t GetUsedMemory() = 0;
	virtual size_t GetAllocations() = 0;
	virtual size_t GetSize() = 0;
};

/// <summary>
/// 스택 할당자 생성
/// </summary>
/// <returns> 스택 할당자</returns>
extern "C" ALLOCATOR_DLL IAllocator* CreateStackAllocator();