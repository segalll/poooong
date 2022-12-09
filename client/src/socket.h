#pragma once

#include <boost/asio.hpp>

namespace net::socket
{
    using boost::asio::ip::udp;

    struct SocketData {
        std::unique_ptr<boost::asio::io_context> ioContext;
        udp::socket socket;
    };

    SocketData init();
    void send(const SocketData& socketData, const std::string& message);
    std::string receive(const SocketData& socketData);
}