#include "network_lynks.hpp"
#include <csignal>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class lynks_server : public lynks::network::server_interface {
    public:
        lynks_server(uint16_t port) : lynks::network::server_interface(port) {}

        virtual bool on_client_connect(std::shared_ptr<lynks::network::connection> client) {
            return true;
        }
};

std::atomic<bool> stopping{false};

void on_sigint(int) {
    if (!stopping.exchange(true)) {
        lynks::network::janus_request::shutdown();
    }
    std::_Exit(0);
}

int main() {
    std::signal(SIGINT, on_sigint);

    lynks_server server(60000);

    if (!server.start()) {
        std::cerr << "Unable to start server\n";
        return 1;
    }

    while (!stopping.load()) {
        server.update(-1, true);
    }

    return 0;
}