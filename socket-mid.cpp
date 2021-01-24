// socket-mid.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma warning(disable:4996)

#include "driver.h"
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include "eftstructs.h"

using namespace std;
#pragma comment(lib, "Ws2_32")

string wstring2string(wstring wstr)
{
	string result;
	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}


std::uint32_t find_process_by_id(const std::string& name)
{
	const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	PROCESSENTRY32 proc_entry{};
	proc_entry.dwSize = sizeof proc_entry;

	auto found_process = false;
	if (!!Process32First(snap, &proc_entry)) {
		do {

			std::wstring s(proc_entry.szExeFile);
			if (name == wstring2string(s)) {
				found_process = true;
				break;
			}
		} while (!!Process32Next(snap, &proc_entry));
	}

	CloseHandle(snap);
	return found_process
		? proc_entry.th32ProcessID
		: 0;
}


//std::string getUnicodeString(uint64_t addr, int stringLength)
//{
//	char16_t wcharTemp[64] = { '\0' };
//	driver::read(addr, wcharTemp, stringLength * 2);
//	std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(wcharTemp);
//	return u8_conv;
//}
//


auto gameData = EFTData::Instance();

int main()
{

	
	//while (true) {
	//	int target;
	//	cin >> hex >> target;

	//	//const auto sth = driver::read<uint32_t>(connection, pid, target);
	//	cout << "看看获取到了个啥:  \n" << hex << target << "\n";

	//}

    


	
	uint32_t pid = find_process_by_id("EscapeFromTarkov.exe");
	//uint32_t pid = find_process_by_id("readm.exe");

	cout << "成功寻找到目标进程！ PID: " << pid << "\n";

	if (!pid) return 0;

	driver::initialize();
	if (!driver::cache(pid)) return 0;


	while (TRUE)
	{
		if (!gameData->Read())
		{
			gameData->InitOffsets();
			Sleep(4000);
		}

		//Sleep(2);
	}

	
	//while(true) {
	//	int commond;
	//	cin >> hex >> commond;

	//	const auto sth = driver::read<uint32_t>(connection, pid, commond);
	//	cout << "看看获取到了个啥:  \n" << sth << "\n";
	//}


	//const auto sth = driver::read<uint32_t>(0x42F918);
	//cout << "看看获取到了个啥:  \n" << sth << "\n";

	//const auto sth1 = driver::GetUnicodeString(connection, pid, 0x8FF8B0, 8);
	//cout << "看看获取到了个啥:  \n" << sth1 << "\n";

	//
	//const auto jz = driver::get_process_base_address(connection, pid);
	//cout << "获取基址:  \n" << hex << jz << "\n";

	driver::deinitialize();
	/*
	*/
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单
