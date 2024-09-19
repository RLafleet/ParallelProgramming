#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	int threadNumber = (int)lpParam;
	std::string output = "The flow number " + std::to_string(threadNumber) + " is running\n";
	std::cout << output;

	ExitThread(0); 
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Invalid argument count" << std::endl;
		return EXIT_FAILURE;
	}

	std::istringstream ss(argv[1]);
	int n;

	if (!(ss >> n))
	{
		std::cout << "Invalid number passed" << std::endl;
	}

	HANDLE* handles = new HANDLE[n];

	for (int i = 1; i <= n; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, reinterpret_cast<LPVOID>(i), CREATE_SUSPENDED, NULL);
	}

	for (int i = 1; i <= n; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(n, handles, true, INFINITE);

	return EXIT_SUCCESS;
}
