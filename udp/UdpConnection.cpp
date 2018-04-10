#include <cstring>
#include <netdb.h>
#include <unistd.h>

#include "UdpConnection.h"
#include "abort.h"

namespace net
{
	/**
	 * Constructor
	 */
	UdpConnection::UdpConnection()
		: _fd(::socket(AF_INET, SOCK_DGRAM, 0)), _is_connected(false),
		  _is_init(_fd)
	{
	}

	/**
	 * Destructor
	 */
	UdpConnection::~UdpConnection()
	{
	}

	/**
	 * Assign the port on which to listen for messages
	 *
	 * @param[in] port The port number
	 * @param[in] name A network interface name, e.g. "localhost", or
	 *                 an IP address, e.g. "127.0.0.1". This is
	 *                 optional; the default behavior is to bind() to
	 *                 all available interfaces
	 *
	 * @return True on success
	 */
	bool UdpConnection::bind(uint16 port, const std::string& name)
	{
		AbortIfNot(_is_init, false);

		struct sockaddr_in addr;
		AbortIfNot(_init_sockaddr(port, name, addr), false);

		AbortIf(::bind(_fd.get(), to_sockaddr(&addr), sizeof(addr)),
			false);

		return true;
	}

	/**
	 * Connect to a remote host
	 *
	 * @param[in] port The remote port number
	 * @param[in] host Connect to this host. This can be either an IP
	 *                 address or a host name
	 *
	 * @return True on success
	 */
	bool UdpConnection::connect(uint16 port, const std::string& host)
	{ 
		AbortIfNot(_is_init, false);

		AbortIfNot(_init_sockaddr(port, host, _remote_addr),
			false);

		socklen_t len = sizeof(_remote_addr);

		AbortIf(::connect(_fd.get(), to_sockaddr(&_remote_addr), len),
			false);

		_is_connected = true;
		return true;
	}

	/**
	 * Receive data from a remote node
	 *
	 * @param[in] buf     The buffer to write the received data into
	 * @param[in] size    Read at most this many bytes
	 * @param[in] timeout The timeout (in milliseconds) after which
	 *                    this call will return, even if no data was
	 *                    available for reading. Specifying -1 may
	 *                    block indefinitely
	 *
	 * @return The number of bytes received, or -1 on error
	 */
	int UdpConnection::recv(char* buf, int size, int timeout) const
	{
		AbortIfNot(_is_init, -1);

		if (!_fd.can_read(timeout)) return 0;

		struct sockaddr_in s_addr;

		socklen_t addrlen = sizeof(s_addr);
		int nbytes =
			::recvfrom(_fd.get(), buf, size, 0, to_sockaddr(&s_addr),
				&addrlen);

		AbortIf(nbytes < 0, -1);
		return nbytes;
	}

	/**
	 * Send data to a remote node
	 *
	 * @param[in] buf     The buffer containing the data to send
	 * @param[in] size    Write this many bytes
	 * @param[in] timeout The maximum number of milliseconds to wait
	 *                    for space to become available for writing;
	 *                    specifying -1 may block indefinitely
	 *
	 * @return The number of bytes written, or -1 on error
	 */
	int UdpConnection::send(const char* buf, int size, int timeout)
		const
	{
		AbortIfNot(_is_init, -1);
		AbortIfNot(_is_connected, -1,
					"send() is only allowed on connected sockets.");

		if (!_fd.can_write(timeout))
			return 0;

		int nbytes = ::write(_fd.get(), buf, size);
		AbortIf(nbytes < 0, -1);

		return nbytes;
	}

	/**
	 * Initialize a sockaddr_in struct
	 *
	 * @param[in]  port The port
	 * @param[in]  name A host name or IP address
	 * @oaram[out] addr The initialized struct
	 *
	 * @return True on success
	 */
	bool UdpConnection::_init_sockaddr(uint16 port,
		const std::string& name, struct sockaddr_in& addr) const
	{
		std::memset(reinterpret_cast<char*>(&addr), 0,
			sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_port   = ::htons(port);

		if (name.empty())
		{
			addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
		}
		else
		{
			struct hostent *he = ::gethostbyname(name.c_str());
			AbortIfNot(he, false);

			std::memcpy(&addr.sin_addr, he->h_addr_list[0],
				he->h_length);
		}

		return true;
	}
}