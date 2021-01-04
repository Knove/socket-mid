#pragma once
#include <WinSock2.h>
#include <cstdint>

namespace driver
{
	void	initialize();
	void	deinitialize();

	SOCKET	connect();
	void	disconnect(SOCKET connection);

	uint32_t read_memory(SOCKET connection, uint32_t process_id, uintptr_t address, uintptr_t buffer, size_t size);

	template <typename T>
	T read(const SOCKET connection, const uint32_t process_id, const uintptr_t address)
	{
		T buffer{ };
		read_memory(connection, process_id, address, uint64_t(&buffer), sizeof(T));

		return buffer;
	}

}