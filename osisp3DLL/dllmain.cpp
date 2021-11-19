// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <iostream>
#include <vector>


#define TARGET_STRING "text text text" 
#define REPLACE_STRING "REPLACED"

#define MAX_STRING_LENGTH 256



#ifdef __cplusplus    
extern "C" {         
#endif
    __declspec(dllexport) bool  __cdecl replaceString(std::string targetString, std::string replaceString) {
        if (replaceString.length() > MAX_STRING_LENGTH - 1) return false;
        SYSTEM_INFO si;//тут есть адрес памяти, с котрого начинается приложение
        GetSystemInfo(&si);

        
        MEMORY_BASIC_INFORMATION mi;//информация о секторе памяти
        for (LPVOID lpAddress = si.lpMinimumApplicationAddress;//минимальный адрес, доступный процессу
                lpAddress <= si.lpMaximumApplicationAddress;//макс 
                lpAddress =(LPVOID)((SIZE_T) lpAddress + mi.RegionSize)) { //размер региона, о котором я хочу получить информацию


            if (!VirtualQuery(lpAddress, &mi, sizeof(mi))) break; //запрашиваю информацию о регионе памяти (адрес, указатель на структуру, в которую записать, размер структуры)
            if (!(mi.Protect & PAGE_READWRITE) || (mi.Protect & PAGE_WRITECOPY)) continue; //есть ли у меня права на чтение/запись. если нет, перехожу к следующему региону

            std::vector<char> buffer(mi.RegionSize); //буфер, куда я записываю виртуальную память
            SIZE_T bytes;
            ReadProcessMemory(GetCurrentProcess(), lpAddress, &buffer[0], mi.RegionSize, &bytes); //прочитал память (хэндл  текущего процесса, адрес откуда читаю, адрес, куда записываю, размер региона чтения, указатель на количество прочтенных байт


            for (int i = 0; i < buffer.size(); i++) {//иду по прочтенному буферу
                if ((char*)lpAddress + i == targetString.c_str()) continue;//проверяю, если адрес совпадает с тем, что я передал, то скип
                if (strcmp(targetString.c_str(), &buffer[i]) == 0) { //сравниваю строки (таргет и то, что в буфере)
                    WriteProcessMemory(GetCurrentProcess(), (LPVOID)((char*)lpAddress + i), replaceString.c_str(), replaceString.length() + 1, &bytes);//перезапись текста(хэндл на процесс, адрес начала региона + сколько я прошел в цикле, на что заменяю, ее длина + /0, сколько записал )

                }
            }
        }
    }
#ifdef __cplusplus
}
#endif



//автоматически вызывается при подключении к другому процессу


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) //когда я подключаю библиотеку, вызывается функция dllMain
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: 
        replaceString(TARGET_STRING, REPLACE_STRING);
        break;
    case DLL_THREAD_ATTACH:
        replaceString(TARGET_STRING, REPLACE_STRING);
        break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


