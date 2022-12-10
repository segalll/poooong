#include "socket.h"

// TODO: fill these in and move them
const std::string IP_ADDRESS = "";
const unsigned short PORT = 0;

namespace net::socket
{
    SocketData init() {
        std::unique_ptr<boost::asio::io_context> ioContext;

        udp::socket socket(*ioContext, udp::endpoint(boost::asio::ip::address::from_string(IP_ADDRESS), PORT));
        socket.non_blocking(true);

        return {
            .ioContext = std::move(ioContext),
            .socket = std::move(socket)
        };
    }

    bool send(SocketData& socketData, const std::string& message) {
        boost::asio::const_buffer buffer = boost::asio::buffer(message);

        boost::system::error_code errorCode;
        socketData.socket.send(buffer, 0, errorCode);

        return errorCode == boost::system::errc::success;
    }

    std::pair<bool, std::string> receive(SocketData& socketData) {
        std::array<char, 1024> buffer;

        boost::system::error_code errorCode;
        size_t bytesReceived = socketData.socket.receive(boost::asio::buffer(buffer), 0, errorCode);

        return std::pair(errorCode == boost::system::errc::success, std::string(buffer.data(), bytesReceived));
    }
}