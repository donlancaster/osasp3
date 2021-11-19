#include <iostream>
#include <thread>
#include <chrono>
#include <Windows.h>

#define OUTPUT_PERIOD_S 1
#define OUTPUT_STRING "text text text"
#define MAX_STRING_SIZE 256

int main()
{
	std::cout << "pid: " << GetCurrentProcessId() << std::endl;
	char outputString[MAX_STRING_SIZE];
	strcpy_s(outputString, OUTPUT_STRING);
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(OUTPUT_PERIOD_S));
		std::cout << outputString << std::endl;
	}
}