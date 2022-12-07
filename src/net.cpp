#include "net.h"

#include <boost/asio.hpp>

namespace net
{
    IPEndpoint ipEndpointCreate(const std::string& address, unsigned short port) {
        // Create a new IPEndpoint with the given address and port
        IPEndpoint endpoint;
        endpoint.address = address;
        endpoint.port = port;
        return endpoint;
    }

    std::unique_ptr<udp::socket> socketCreate() {
        // Create a socket using the UDP protocol
        boost::asio::io_service ioService;
        return std::make_unique<udp::socket>(ioService, udp::endpoint(udp::v4(), 0));
    }

    void send(const std::string& packet, const IPEndpoint& endpoint, const udp::socket& socket) {
        // Send the packet to the given endpoint using the given socket
        udp::endpoint remoteEndpoint(boost::asio::ip::address::from_string(endpoint.address), endpoint.port);
        const_cast<udp::socket&>(socket).send_to(boost::asio::buffer(packet), remoteEndpoint);
    }

    std::pair<std::string, IPEndpoint> receive(const udp::socket& socket) {
        // Receive a packet from the given endpoint using the given socket
        udp::endpoint remoteEndpoint;
        std::array<char, 4096> recvBuffer;
        std::size_t bytesReceived = const_cast<udp::socket&>(socket).receive_from(boost::asio::buffer(recvBuffer), remoteEndpoint);

        // Return the received packet and endpoint as output values
        std::string packet(recvBuffer.data(), bytesReceived);
        IPEndpoint endpoint;
        endpoint.address = remoteEndpoint.address().to_string();
        endpoint.port = remoteEndpoint.port();
        return std::make_pair(packet, endpoint);
    }
}