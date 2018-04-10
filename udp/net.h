#ifndef __NET_H__
#define __NET_H__

#include <sys/socket.h>

namespace net
{
	/**
	 * Determine whether or not the CPU architecture is big endian
	 *
	 * @return True if it's big endian
	 */
	inline bool is_big_endian()
	{
	    const int number = 0x1;
	    const char *ptr  = reinterpret_cast<const char* >(&number);

	    return !(ptr[0]);
	}

	/**
	 * Type conversion to the generic struct sockaddr*
	 *
	 * @param[in] sa A struct sockaddr_in*
	 *
	 * @return reinterpret_cast<struct sockaddr*>(sa)
	 */
	inline struct sockaddr* to_sockaddr(struct sockaddr_in* sa)
	{
		return reinterpret_cast<struct sockaddr*>(sa);
	}

	/**
	 * Convert a given data element to network byte order
	 *
	 * @note Only works with basic C++ types
	 *
	 * @param[in,out] data Convert this data element
	 */
	template <typename T>
	inline void to_network_order(T& data)
	{
		if (!is_big_endian())
		{
			auto swap = [] (char a, char b) {
				a ^= b;
				b ^= a;
				a ^= b;
			};

			char* cp = &data;
			const int N = sizeof(T);

			for (int i = 0; i < N / 2; i++)
				swap(cp[i], cp[N-i-1]);
		}
	}
}

#endif
