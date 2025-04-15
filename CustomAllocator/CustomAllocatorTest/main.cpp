#include <windows.h>
#include <iostream>
#include <string>
#include <new>

#include "../CustomAllocator/IAllocator.h"

#define ALLOCATE_COUNT 100'000
#define ALLOCATE_SIZE 10

#define MEM_SIZE 1'048'576'000 //1GB

class DataChunk
{
public:

	int a;
	int b;
	char c;
	char d;

	DataChunk() { a = 1; b = 2; c = 'A'; d = 'B'; };
};

void PrintAllocatorState(IAllocator* _allocator)
{
	std::cout << "Start At : " << _allocator->GetStartPointer() << "\n";
	std::cout << "Used : " << _allocator->GetUsedMemory() << "\n";
	std::cout << "Allocate Count : " << _allocator->GetAllocations() << "\n";
	std::cout << "Size : " << _allocator->GetSize() << "\n";
	std::cout << '\n';
}

int main()
{
#pragma region DLL Ready
	std::string path("./CustomAllocator.dll");

	HMODULE hDLL = ::LoadLibraryA(path.c_str());
	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";

		return 0;
	}
#pragma endregion

#pragma region Time Set
	unsigned long long TPS;
	unsigned long long start;
	unsigned long long end;

	QueryPerformanceFrequency((LARGE_INTEGER*)&TPS);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
#pragma endregion

	DataChunk* storage[ALLOCATE_COUNT] = { 0, };

#pragma region Stack Allocator
	/// ���� �Ҵ��� ���� ����
	std::cout << "Create Stack Allocator : ";
	IAllocator* stackAllocator = ((IAllocator * (*)(size_t, size_t))GetProcAddress(hDLL, "CreateStackAllocator"))(MEM_SIZE, 16);
	if (stackAllocator)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk));
		storage[i] = new(ptr) DataChunk();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Stack allocte elapse : " << (end - start) / (double)TPS << "\n\n";
	stackAllocator->Clear();
	delete stackAllocator;
	/// ���� �Ҵ��� ���� ��
#pragma endregion

#pragma region Pool Allocator
/// Ǯ �Ҵ��� ���� ����
	std::cout << "Create Pool Allocator : ";
	IAllocator* poolAllocator = ((IAllocator * (*)(size_t, size_t, size_t))GetProcAddress(hDLL, "CreatePoolAllocator"))(MEM_SIZE, sizeof(DataChunk), 16);
	if (poolAllocator)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = poolAllocator->Allocate(sizeof(DataChunk));
		storage[i] = new(ptr) DataChunk();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Pool allocate elapse : " << (end - start) / (double)TPS << "\n\n";
	poolAllocator->Clear();
	delete poolAllocator;
	/// Ǯ �Ҵ��� ���� ��
#pragma endregion

#pragma region List Allocator Mode First Fit
/// ����Ʈ �Ҵ��� ���� ����
	std::cout << "Create List Allocator : ";
	IAllocator* listAllocator0 = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 0, 16);
	if (listAllocator0)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk));
		storage[i] = new(ptr) DataChunk();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "List allocate First Fit elapse : " << (end - start) / (double)TPS << "\n\n";
	listAllocator0->Clear();
	delete listAllocator0;
/// ����Ʈ �Ҵ��� ���� ��
#pragma endregion

#pragma region List Allocator Mode First Fit
/// ����Ʈ �Ҵ��� ���� ����
	std::cout << "Create List Allocator : ";
	IAllocator* listAllocator1 = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 1, 16);
	if (listAllocator1)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk));
		storage[i] = new(ptr) DataChunk();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "List allocate Best Fit elapse : " << (end - start) / (double)TPS << "\n\n";
	listAllocator1->Clear();
	delete listAllocator1;
/// ����Ʈ �Ҵ��� ���� ��
#pragma endregion

#pragma region C++ new
	/// C++ new �Ҵ� ����
	std::cout << "start New Allocate\n";

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		storage[i] = new DataChunk();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "elapse : " << (end - start) / (double)TPS << "\n";
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		delete storage[i];
	}
	/// C++ new �Ҵ� ��
#pragma endregion

	return 0;
}