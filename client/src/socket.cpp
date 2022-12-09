#include "socket.h"

// TODO: fill these in and move them
const std::string IP_ADDRESS = "";
const unsigned short PORT = 0;

namespace net::socket
{
    SocketData init() {
        std::unique_ptr<boost::asio::io_context> ioContext;

        return {
            .ioContext = std::move(ioContext),
            .socket = udp::socket(
                *ioContext,
                udp::endpoint(boost::asio::ip::address::from_string(IP_ADDRESS), PORT)
            )
        };
    }

    void send(SocketData& socketData, const std::string& message) {
        boost::asio::const_buffer buffer = boost::asio::buffer(message);
        socketData.socket.send(buffer);
    }

    std::string receive(SocketData& socketData) {
        std::array<char, 1024> buffer;

        size_t bytesReceived = socketData.socket.receive(boost::asio::buffer(buffer));

        return std::string(buffer.data(), bytesReceived);
    }
}