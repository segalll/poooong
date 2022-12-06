#pragma once

#include <boost/asio.hpp>

namespace net
{
    using boost::asio::ip::udp;

    // IPEndpoint represents an IP address and port number pair
    struct IPEndpoint {
        std::string address;
        unsigned short port;
    };

    // Initialize the networking library
    bool init();

    // Create a new IPEndpoint with the given address and port
    IPEndpoint ipEndpointCreate(const std::string& address, unsigned short port);

    // Create a new socket using the UDP protocol
    std::unique_ptr<udp::socket> socketCreate();

    // Send a packet to the given endpoint using the given socket
    void send(const std::string& packet, const IPEndpoint& endpoint, const udp::socket& socket);

    // Receive a packet from the given endpoint using the given socket
    std::pair<std::string, IPEndpoint> receive(const udp::socket& socket);
}