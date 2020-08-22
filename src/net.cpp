#include "net.h"

namespace net
{
	bool init()
	{
		WORD winsockVersion = 0x202;
		WSADATA winsockData;
		if (WSAStartup(winsockVersion, &winsockData))
		{
			printf("WSAStartup failed: %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool socketCreate(Socket* outSocket)
	{
		int addressFamily = AF_INET;
		int type = SOCK_DGRAM;
		int protocol = IPPROTO_UDP;
		SOCKET sock = socket(addressFamily, type, protocol);

		if (sock == INVALID_SOCKET)
		{
			printf("socket failed: %d\n", WSAGetLastError());
			return false;
		}

		// put socket in non-blocking mode
		u_long enabled = 1;
		int result = ioctlsocket(sock, FIONBIO, &enabled);
		if (result == SOCKET_ERROR)
		{
			printf("ioctlsocket failed: %d\n", WSAGetLastError());
			return false;
		}

		*outSocket = {};
		outSocket->handle = sock;
		return true;
	}

	bool socketSend(Socket* sock, unsigned char* packet, unsigned int packetSize, IPEndpoint* endpoint)
	{
		SOCKADDR_IN serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.S_un.S_addr = htonl(endpoint->address);
		serverAddress.sin_port = htons(endpoint->port);
		int server_address_size = sizeof(serverAddress);

		if (sendto(sock->handle, (const char*)packet, packetSize, 0, (SOCKADDR*)&serverAddress, server_address_size) == SOCKET_ERROR)
		{
			printf("sendto failed: %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool socketReceive(Socket* sock, unsigned char* buffer, unsigned int bufferSize, IPEndpoint* outFrom, unsigned int* outBytesReceived)
	{
		int flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytesReceived = recvfrom(sock->handle, (char*)buffer, bufferSize, flags, (SOCKADDR*)&from, &from_size);

		if (bytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", error);
			}

			return false;
		}

		*outFrom = {};
		outFrom->address = ntohl(from.sin_addr.S_un.S_addr);
		outFrom->port = ntohs(from.sin_port);

		*outBytesReceived = bytesReceived;

		return true;
	}

	IPEndpoint ipEndpointCreate(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port)
	{
		IPEndpoint ipEndpoint = {};
		ipEndpoint.address = (a << 24) | (b << 16) | (c << 8) | d;
		ipEndpoint.port = port;
		return ipEndpoint;
	}
} // namespace net