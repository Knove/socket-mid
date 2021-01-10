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
	uint32_t	cache(uint32_t pid);

	SOCKET	connect();
	void	disconnect(SOCKET connection);

	uint32_t read_memory(uintptr_t address, PVOID buffer, size_t size);
	uint64_t get_process_base_address(const SOCKET connection, const uint32_t process_id);
	uint64_t get_process_base_address();

	uint64_t get_process_peb();

	uint64_t readChain(SOCKET connection, const uint32_t process_id, uint64_t base, const vector<uint64_t>& offsets);
	uint64_t readEFTChain(uint64_t base, const std::vector<uint64_t>& offsets);
	string GetUnicodeString(uint64_t addr, int stringLength);

	template <typename T>
	T read(const uint64_t address)
	{
		T buffer{ };
		read_memory(address, &buffer, sizeof(T));

		return buffer;
	}


}