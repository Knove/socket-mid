#pragma once
#include <WinSock2.h>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;
namespace driver
{
	void	initialize();
	void	deinitialize();

	SOCKET	connect();
	void	disconnect(SOCKET connection);

	uint32_t read_memory(SOCKET connection, uint32_t process_id, uintptr_t address, PVOID buffer, size_t size);
	uint64_t get_process_base_address(SOCKET connection, uint32_t process_id);

	uint64_t readChain(SOCKET connection, const uint32_t process_id, uint64_t base, const vector<uint64_t>& offsets);
	string GetUnicodeString(const SOCKET connection, const uint32_t process_id, uint64_t addr, int stringLength);

	template <typename T>
	T read(const SOCKET connection, const uint32_t process_id, const uint64_t address)
	{
		T buffer{ };
		read_memory(connection, process_id, address, &buffer, sizeof(T));

		return buffer;
	}

}