#include <windows.h>
#include <iostream>
#include <string>
#include <new>
#include "../CustomAllocator/IAllocator.h"

#define ALLOCATE_COUNT 100
#define ALLOCATE_SIZE 10

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
	std::string path("./CustomAllocator.dll");

	HMODULE hDLL = ::LoadLibraryA(path.c_str());

	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";

		return 0;
	}

	std::cout << "Create Stack Allocator : ";
	IAllocator* stackAllocator = ((IAllocator * (*)(void))GetProcAddress(hDLL, "CreateStackAllocator"))();
	if (stackAllocator)
	{
		std::cout << "Success \n";
	}

	std::cout << "Init Allocator : ";
	stackAllocator->Init(1024 * 1024, 16);
	std::cout << "Success \n";
	std::cout << "\n";
	PrintAllocatorState(stackAllocator);

	DataChunk* storage[ALLOCATE_COUNT];

	std::cout << "\n";
	std::cout << "Data Size : " << sizeof(DataChunk) << "\n\n";
	std::cout << "Data Size : " << sizeof(int) << "\n\n";
	std::cout << "Data Size : " << sizeof(char) << "\n\n";

	for (int i = 0; i < ALLOCATE_COUNT; i++)
	{
		void* ptr = stackAllocator->Allocate(sizeof(DataChunk));
		storage[i] = new(ptr) DataChunk();
		std::cout << "Allocated : " << storage[i] << "\n";
		std::cout << "Data : " << storage[i]->a << " " << storage[i]->b << " " << storage[i]->c << " " << storage[i]->d << " " << "\n";
		PrintAllocatorState(stackAllocator);
	}

	stackAllocator->Deallocate(storage[ALLOCATE_COUNT - 1]);
	stackAllocator->Deallocate(storage[ALLOCATE_COUNT - 2]);
	stackAllocator->Deallocate(storage[ALLOCATE_COUNT - 3]);

	stackAllocator->Deallocate(storage[0]);

	return 0;
}