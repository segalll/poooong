#pragma once

#include "Windows.h"
#include <stdio.h>

namespace net
{
	struct IPEndpoint
	{
		unsigned int address;
		unsigned short port;
	};

	struct Socket
	{
		SOCKET handle;
	};

	bool init();
	bool socketCreate(Socket* outSocket);
	bool socketSend(Socket* sock, unsigned char* packet, unsigned int packetSize, IPEndpoint* endpoint);
	bool socketReceive(Socket* sock, unsigned char* buffer, unsigned int bufferSize, IPEndpoint* outFrom, unsigned int* outBytesReceived);
	IPEndpoint ipEndpointCreate(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port);
}