#include <windows.h>
#include <iostream>
#include <string>
#include <new>
#include <assert.h>
#include <iomanip>
#include "../CustomAllocator/IAllocator.h"

#define ALLOCATE_COUNT 100'000

#define ALLOCATE_COUNT_16b 10000
#define ALLOCATE_COUNT_256b 1000
#define ALLOCATE_COUNT_2mb 50

#define ALLOCATE_SIZE 10

#define MEM_SIZE 1'048'576'000 //1GB

class DataChunk
{
public:
	virtual bool CheckData() = 0;
};

class DataChunkDefault : public DataChunk
{
public:

	int a;
	int b;
	char c;
	char d;

	DataChunkDefault() { a = 1; b = 2; c = 'A'; d = 'B'; };

	bool CheckData() override
	{
		if (this->a == 1 && this->b == 2 && this->c == 'A' && this->d == 'B')
		{
			return true;
		}
		return false;
	}
};

class DataChunk16b : public DataChunk
{
public:
	uint16_t m_data;
	DataChunk16b() { m_data = 1; }
	virtual bool CheckData() override
	{
		if (this->m_data == 1)
		{
			return true;
		}
		return false;
	}
};

class DataChunk256b : public DataChunk
{
public:
	uint64_t m_data[8];
	DataChunk256b() 
		: m_data{0 , }
	{ 
	}
	virtual bool CheckData() override
	{
		for (int i = 0; i < 8 ; i++)
		{
			if (m_data[i] != 0)
			{
				return false;
			}
		}
		return true;
	}
};

class DataChunk2mb : public DataChunk
{
public:
	uint64_t m_data[1024];
	DataChunk2mb()
		: m_data{ 0 , }
	{
	}
	virtual bool CheckData() override
	{
		for (int i = 0; i < 1024; i++)
		{
			if (m_data[i] != 0)
			{
				return false;
			}
		}
		return true;
	}
};

bool CheckAllData(DataChunk** _array, int _count)
{
	for (int i = 0; i < _count; i++)
	{
		if (!_array[i]->CheckData())
		{
			return false;
		}
	}
	return true;
}

void PrintAllocatorState(IAllocator* _allocator)
{
	std::cout << "Start At : " << _allocator->GetStartPointer() << "\n";
	std::cout << "Used : " << _allocator->GetUsedMemory() << "\n";
	std::cout << "Allocate Count : " << _allocator->GetAllocations() << "\n";
	std::cout << "Size : " << _allocator->GetSize() << "\n";
	std::cout << '\n';
}


void TestSameAllocateTime()
{
#pragma region DLL Ready
	std::string path("./CustomAllocator.dll");

	HMODULE hDLL = ::LoadLibraryA(path.c_str());
	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";
		return;
	}

	DataChunk** storage = new DataChunk * [ALLOCATE_COUNT];

#pragma endregion

#pragma region Time Set
	unsigned long long TPS;
	unsigned long long start;
	unsigned long long end;

	QueryPerformanceFrequency((LARGE_INTEGER*)&TPS);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	bool check = false;
#pragma endregion


#pragma region Stack Allocator
	/// 스택 할당자 연산 시작
	std::cout << "===============================\n";
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
		void* ptr = stackAllocator->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Stack allocte elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";

	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	std::cout << "===============================\n\n";
	stackAllocator->Clear();
	delete stackAllocator;
	/// 스택 할당자 연산 끝
#pragma endregion

#pragma region Pool Allocator
/// 풀 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create Pool Allocator : ";
	IAllocator* poolAllocator = ((IAllocator * (*)(size_t, size_t, size_t))GetProcAddress(hDLL, "CreatePoolAllocator"))(MEM_SIZE, sizeof(DataChunkDefault), 16);
	if (poolAllocator)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = poolAllocator->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Pool allocate elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";
	poolAllocator->Clear();
	delete poolAllocator;
	/// 풀 할당자 연산 끝
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
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
		void* ptr = listAllocator0->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "List allocate First Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";
	listAllocator0->Clear();
	delete listAllocator0;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
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
		void* ptr = listAllocator1->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "List allocate Best Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";
	listAllocator1->Clear();
	delete listAllocator1;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region C++ new
	/// C++ new 할당 시작
	std::cout << "===============================\n";
	std::cout << "start New Allocate\n";

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		storage[i] = new DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		delete storage[i];
	}
	/// C++ new 할당 끝
#pragma endregion

	::FreeLibrary(hDLL);
	delete[] storage;
}

void TestSameAllocateAndDeleteTime()
{
#pragma region DLL Ready
	std::string path("./CustomAllocator.dll");

	HMODULE hDLL = ::LoadLibraryA(path.c_str());
	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";
		return;
	}

	DataChunk** storage = new DataChunk * [ALLOCATE_COUNT];

#pragma endregion

#pragma region Time Set
	unsigned long long TPS;
	unsigned long long start;
	unsigned long long end;

	QueryPerformanceFrequency((LARGE_INTEGER*)&TPS);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	bool check = false;
#pragma endregion


#pragma region Stack Allocator
	/// 스택 할당자 연산 시작
	std::cout << "===============================\n";
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
		void* ptr = stackAllocator->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Stack allocte elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";

	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = ALLOCATE_COUNT - 1; i >= 0; i--)
	{
		stackAllocator->Deallocate(storage[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "Lest Memory : " << stackAllocator->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "===============================\n\n";

	stackAllocator->Clear();
	delete stackAllocator;
	/// 스택 할당자 연산 끝
#pragma endregion

#pragma region Pool Allocator
/// 풀 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create Pool Allocator : ";
	IAllocator* poolAllocator = ((IAllocator * (*)(size_t, size_t, size_t))GetProcAddress(hDLL, "CreatePoolAllocator"))(MEM_SIZE, sizeof(DataChunkDefault), 16);
	if (poolAllocator)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = poolAllocator->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Pool allocate elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		poolAllocator->Deallocate(storage[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "Lest Memory : " << poolAllocator->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";

	poolAllocator->Clear();
	delete poolAllocator;
	std::cout << "===============================\n\n";
	/// 풀 할당자 연산 끝
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
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
		void* ptr = listAllocator0->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "List allocate First Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		listAllocator0->Deallocate(storage[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "Lest Memory : " << listAllocator0->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "===============================\n\n";

	listAllocator0->Clear();
	delete listAllocator0;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
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
		void* ptr = listAllocator1->Allocate(sizeof(DataChunkDefault));
		storage[i] = new(ptr) DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "List allocate Best Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		listAllocator1->Deallocate(storage[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "Lest Memory : " << listAllocator1->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "===============================\n\n";

	listAllocator1->Clear();
	delete listAllocator1;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region C++ new
	/// C++ new 할당 시작
	std::cout << "===============================\n";
	std::cout << "start New Allocate\n";

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		storage[i] = new DataChunkDefault();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "new elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "Checking Data : ";
	if (CheckAllData(storage, ALLOCATE_COUNT))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		delete storage[i];
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "delete elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "===============================\n\n";
	/// C++ new 할당 끝
#pragma endregion

	::FreeLibrary(hDLL);
	delete[] storage;
}

void TestDiffrentBlockAllocateTime()
{
#pragma region DLL Ready
	std::string path("./CustomAllocator.dll");

	HMODULE hDLL = ::LoadLibraryA(path.c_str());
	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";
		return;
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

#pragma region DataSet
	DataChunk** storage16b = new DataChunk * [ALLOCATE_COUNT_16b];
	DataChunk** storage256b = new DataChunk * [ALLOCATE_COUNT_256b];
	DataChunk** storage2mb = new DataChunk * [ALLOCATE_COUNT_2mb];
#pragma endregion


#pragma region Stack Allocator
	/// 스택 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create Stack Allocator : ";
	IAllocator* stackAllocator = ((IAllocator * (*)(size_t, size_t))GetProcAddress(hDLL, "CreateStackAllocator"))(MEM_SIZE, 16);
	if (stackAllocator)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk16b));
		storage16b[i] = new(ptr) DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk256b));
		storage256b[i] = new(ptr) DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk2mb));
		storage2mb[i] = new(ptr) DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Stack allocte elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";

	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";

	stackAllocator->Clear();
	delete stackAllocator;
	/// 스택 할당자 연산 끝
#pragma endregion

#pragma region Pool Allocator
// 풀 할당자는 모든 할당의 크기가 같아야 하므로 생략함
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create List Allocator : ";
	IAllocator* listAllocator0 = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 0, 16);
	if (listAllocator0)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk16b));
		storage16b[i] = new(ptr) DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk256b));
		storage256b[i] = new(ptr) DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk2mb));
		storage2mb[i] = new(ptr) DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "List allocate First Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";

	listAllocator0->Clear();
	delete listAllocator0;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create List Allocator : ";
	IAllocator* listAllocator1 = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 1, 16);
	if (listAllocator1)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk16b));
		storage16b[i] = new(ptr) DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk256b));
		storage256b[i] = new(ptr) DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk2mb));
		storage2mb[i] = new(ptr) DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "List allocate Best Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	std::cout << "===============================\n\n";

	listAllocator1->Clear();
	delete listAllocator1;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region C++ new
	/// C++ new 할당 시작
	std::cout << "===============================\n";
	std::cout << "start New Allocate\n";

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		storage16b[i] = new DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		storage256b[i] = new DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		storage2mb[i] = new DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "new elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "Checking Data : ";
	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		delete storage16b[i];
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		delete storage256b[i];
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		delete storage2mb[i];
	}
	std::cout << "===============================\n\n";
	/// C++ new 할당 끝
#pragma endregion

	::FreeLibrary(hDLL);
	delete[] storage16b;
	delete[] storage256b;
	delete[] storage2mb;
}

void TestDiffrentBlockAllocateAndDeleteTime()
{
#pragma region DLL Ready
	std::string path("./CustomAllocator.dll");

	HMODULE hDLL = ::LoadLibraryA(path.c_str());
	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";
		return;
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

#pragma region DataSet
	DataChunk** storage16b = new DataChunk * [ALLOCATE_COUNT_16b];
	DataChunk** storage256b = new DataChunk * [ALLOCATE_COUNT_256b];
	DataChunk** storage2mb = new DataChunk * [ALLOCATE_COUNT_2mb];
#pragma endregion


#pragma region Stack Allocator
	/// 스택 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create Stack Allocator : ";
	IAllocator* stackAllocator = ((IAllocator * (*)(size_t, size_t))GetProcAddress(hDLL, "CreateStackAllocator"))(MEM_SIZE, 16);
	if (stackAllocator)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk16b));
		storage16b[i] = new(ptr) DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk256b));
		storage256b[i] = new(ptr) DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk2mb));
		storage2mb[i] = new(ptr) DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Stack allocte elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";

	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = ALLOCATE_COUNT_16b - 1; i >= 0; i--)
	{
		stackAllocator->Deallocate(storage16b[i]);
	}
	for (int i = ALLOCATE_COUNT_256b - 1; i >= 0; i--)
	{
		stackAllocator->Deallocate(storage256b[i]);
	}
	for (int i = ALLOCATE_COUNT_2mb - 1; i >= 0; i--)
	{
		stackAllocator->Deallocate(storage2mb[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Lest Memory : " << stackAllocator->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "===============================\n\n";

	stackAllocator->Clear();
	delete stackAllocator;
	/// 스택 할당자 연산 끝
#pragma endregion

#pragma region Pool Allocator
// 풀 할당자는 모든 할당의 크기가 같아야 하므로 생략함
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create List Allocator : ";
	IAllocator* listAllocator0 = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 0, 16);
	if (listAllocator0)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk16b));
		storage16b[i] = new(ptr) DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk256b));
		storage256b[i] = new(ptr) DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		void* ptr = listAllocator0->Allocate(sizeof(DataChunk2mb));
		storage2mb[i] = new(ptr) DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "List allocate First Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		listAllocator0->Deallocate(storage16b[i]);
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		listAllocator0->Deallocate(storage256b[i]);
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		listAllocator0->Deallocate(storage2mb[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Lest Memory : " << listAllocator0->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "===============================\n\n";

	listAllocator0->Clear();
	delete listAllocator0;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region List Allocator Mode First Fit
/// 리스트 할당자 연산 시작
	std::cout << "===============================\n";
	std::cout << "Create List Allocator : ";
	IAllocator* listAllocator1 = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 1, 16);
	if (listAllocator1)
	{
		std::cout << "Success \n";
	}
	// PrintAllocatorState(stackAllocator);

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk16b));
		storage16b[i] = new(ptr) DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk256b));
		storage256b[i] = new(ptr) DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		void* ptr = listAllocator1->Allocate(sizeof(DataChunk2mb));
		storage2mb[i] = new(ptr) DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);

	std::cout << "List allocate Best Fit elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "Checking Data : ";
	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		listAllocator1->Deallocate(storage16b[i]);
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		listAllocator1->Deallocate(storage256b[i]);
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		listAllocator1->Deallocate(storage2mb[i]);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "Lest Memory : " << listAllocator1->GetUsedMemory() << "\n";
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";
	std::cout << "===============================\n\n";

	listAllocator1->Clear();
	delete listAllocator1;
	/// 리스트 할당자 연산 끝
#pragma endregion

#pragma region C++ new
	/// C++ new 할당 시작
	std::cout << "===============================\n";
	std::cout << "start New Allocate\n";

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		storage16b[i] = new DataChunk16b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		storage256b[i] = new DataChunk256b();
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		storage2mb[i] = new DataChunk2mb();
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "new elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "Checking Data : ";
	if (CheckAllData(storage16b, ALLOCATE_COUNT_16b) &&
		CheckAllData(storage256b, ALLOCATE_COUNT_256b) &&
		CheckAllData(storage2mb, ALLOCATE_COUNT_2mb))
	{
		std::cout << "Success\n";
	}
	else
	{
		std::cout << "Fail\n";
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	for (int i = 0; i < ALLOCATE_COUNT_16b; i++)
	{
		delete storage16b[i];
	}
	for (int i = 0; i < ALLOCATE_COUNT_256b; i++)
	{
		delete storage256b[i];
	}
	for (int i = 0; i < ALLOCATE_COUNT_2mb; i++)
	{
		delete storage2mb[i];
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	std::cout << "deallocate elapse : " << (end - start) / (double)TPS << "\n";

	std::cout << "===============================\n\n";
	/// C++ new 할당 끝
#pragma endregion

	::FreeLibrary(hDLL);
	delete[] storage16b;
	delete[] storage256b;
	delete[] storage2mb;
}

int main()
{
	std::cout << std::fixed << std::setprecision(6);

	int menu = 1;
	while (menu != 0)
	{
		std::cout << "===============================\n";
		std::cout << "Test Case\n";
		std::cout << " 1. Same Block, Allocate Only\n";
		std::cout << " 2. Same Block, Allocate And Deallocate\n";
		std::cout << " 3. Diffrent Block, Allocate Only\n";
		std::cout << " 4. Diffrent Block, Allocate And Deallocate\n";
		std::cout << "===============================\n";

		std::cout << "Choose Test Case : ";
		std::cin >> menu;

		switch (menu)
		{
		case 1:
		{
			TestSameAllocateTime();
			break;
		}
		case 2:
		{
			TestSameAllocateAndDeleteTime();
			break;
		}
		case 3:
		{
			TestDiffrentBlockAllocateTime();
			break;
		}
		case 4:
		{
			TestDiffrentBlockAllocateAndDeleteTime();
			break;
		}
		case 0:
		{
			std::cout << "Program Exit\n";
			break;
		}
		default:
		{
			std::cout << "Worng Option\n";
			break;
		}
		}
	}

	return 0;
}