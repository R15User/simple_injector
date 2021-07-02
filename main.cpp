#include<Windows.h>
#include<tlhelp32.h>
#include<stdio.h>

/*
	simple function to get the process id of a process

	proc: process executable name (ex: "game.exe")
*/
DWORD get_process_id(const char* proc)
{
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD process;
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);

	do
	{
		if (!strcmp(pEntry.szExeFile, proc))
		{
			process = pEntry.th32ProcessID;
			CloseHandle(hProcessId);
		}

	} while (Process32Next(hProcessId, &pEntry));
	return process;
}

/*
	inject dll into a process

	process: process executable name (ex: "game.exe")
	dll_path: FULL PATH to dll file (must be full path or it will not work!)
*/
void inject(const char* process, const char* dll_path)
{
	//get process id and open process
	DWORD proc_id = get_process_id(process);
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);
	if (handle == NULL)
	{
		printf("couldnt open process!\n");
		return;
	}

	//allocate some space in the process for the dll path
	LPVOID allocated = VirtualAllocEx(handle, NULL, strlen(dll_path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	//write the dll path into process memory
	WriteProcessMemory(handle, allocated, dll_path, strlen(dll_path), NULL);



	//get address for LoadLibraryA in kernel32.dll
	LPVOID load_library_addy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

	//create remote thread and call LoadLibraryA (this is what injects the dll)
	CreateRemoteThread(handle, NULL, NULL, (LPTHREAD_START_ROUTINE)load_library_addy, allocated, NULL, NULL);

	//close handle as we no longer need it
	CloseHandle(handle);
}




int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		printf("injecting %s into %s\n", argv[2], argv[1]);
		inject(argv[1], argv[2]);
		goto end;
	}
	printf("please specify process and file path (in that order)\n", argv[1], argv[2]);


end:
	system("PAUSE");//shh i know this is bad practice
	return 0;
}