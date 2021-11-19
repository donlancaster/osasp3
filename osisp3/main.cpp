//#define STATIC

//#define DYNAMIC

#define REMOTE_THREAD



/*
статический внедрение - подключаю библиотеку, когда я созаю приложение

динамический  в рпоцессе работы приложения

удаленным потоком = динамичексое, но в другой процесс



*/



#include <Windows.h>
#include <iostream>
#include <string>




#define OUTPUT_STRING "text text text"
#define REPLACE_STRING "REPLACED"
#define MAX_STRING_SIZE 256

#define REMOTE_PROCESS_PATH L"G:\\BSUIR\\5 sem\\OSaSP\\osisp3\\Debug\\RemoteProcess.exe"
#define LIB_PATH "G:\\BSUIR\\5 sem\\OSaSP\\osisp3\\Debug\\osisp3DLL.dll"
#define FUNC_NAME "replaceString"
#define MAX_PATH_SIZE 256

extern "C" void __cdecl replaceString(std::string, std::string); //чтобы не происходит мэнглинг : в си не ыло перегрузки функций, в с++ можно. длл идут на си
typedef void(_cdecl* importedFuncPointer)(std::string, std::string); // тип функции ко

PROCESS_INFORMATION pi;

void processCleanup(PROCESS_INFORMATION pi) {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}
bool consoleCloseHandler(DWORD dwCtrlType) {
    TerminateProcess(pi.hProcess, 0);
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    return true;
}




template <class T>
class MyClass {
public:
    T obj;
    void bar() {
        obj.foo();
    }
};

class FooClass {
public:
    void foo() { std::cout << "foo" << std::endl; }
};




int main()
{
    
#ifdef STATIC
    char outputString[MAX_STRING_SIZE];
    strcpy_s(outputString, OUTPUT_STRING);
    std::cout << outputString << std::endl;

    replaceString(OUTPUT_STRING, REPLACE_STRING);
#endif



#ifdef DYNAMIC
    HINSTANCE stringReplaceDll; //хэндл на библиотеку
    importedFuncPointer replaceStringPointer;// указатель на функцию

    stringReplaceDll = LoadLibraryA(LIB_PATH);
    replaceStringPointer = (importedFuncPointer)GetProcAddress(stringReplaceDll, FUNC_NAME);// указатель на функцию, которую хочу вызвать (хэндл на длл, которую загрузил, имя функции)

    char outputString[MAX_STRING_SIZE];
    strcpy_s(outputString, OUTPUT_STRING);
    std::cout << outputString << std::endl;

    replaceStringPointer(OUTPUT_STRING, REPLACE_STRING);//вызвал функцию
    FreeLibrary(stringReplaceDll);//выгружаю библиотеку
#endif



#if defined(STATIC) || defined(DYNAMIC)
    std::cout << outputString << std::endl;//еще раз вывожу измененную строку
#endif

    
    
#ifdef REMOTE_THREAD
    TCHAR ProcessName[MAX_PATH_SIZE]; 
    wcscpy_s(ProcessName, REMOTE_PROCESS_PATH);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCloseHandler, true); //если приложение закроется, вызовется функция consoleCloseHandler 
    CreateProcess(ProcessName, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    
    int pid;
    std::cout << "enter pid: ";
    std::cin >> pid;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid); 
    

    char libPath[MAX_PATH_SIZE];
    strcpy_s(libPath, LIB_PATH);

    void* pLibRemote = VirtualAllocEx(hProcess, NULL, sizeof(libPath), MEM_COMMIT, PAGE_READWRITE);//выделил память в удаленном процессе
    bool i = WriteProcessMemory(hProcess, pLibRemote, libPath, sizeof(libPath), NULL);

    void* startAddr = GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryA"); //loadLibraryA возвращает handle на загруженную библиотеку

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,(LPTHREAD_START_ROUTINE) startAddr, pLibRemote, 0, NULL);//создал поток в удаленном процессе, котрый будет выполнять LoadLibraryA
    WaitForSingleObject(hThread, INFINITE); //жду пока поток отработает
    // тк поток выполнял функцию loadLibraryA , он вернет то, что вернул LoadLibraryA

    DWORD loadedLib;    //хэднл на библиотеку в удаленном процессе
    GetExitCodeThread(hThread, &loadedLib); //достаем код завершения потока
    CloseHandle(hThread); //закрыл хэндл на опток, который мне больше не нужен
    VirtualFreeEx(hProcess, pLibRemote, sizeof(libPath), MEM_RELEASE);//очистил амять, которую выделил под путь к процессу


    hThread = CreateRemoteThread(hProcess, NULL, 0,  //создаю еще один поток, чтобы выгрузить библиотеку из удаленного процесса
        (LPTHREAD_START_ROUTINE)GetProcAddress(
            GetModuleHandle(L"Kernel32"), "FreeLibrary"),
        (void*)loadedLib,
        0, NULL);

    WaitForSingleObject(hThread, INFINITE); //жду пока поток завершится
    
    CloseHandle(hThread); //закрыл хэндл на поток
    CloseHandle(hProcess); //закрыл хэндл на удаленный процесс

    WaitForSingleObject(pi.hProcess,INFINITE); //жду пока завершится процесс, который сам создал  (112 строка)
    processCleanup(pi);// почистил хэндлы для процесса сверху

#endif
}
