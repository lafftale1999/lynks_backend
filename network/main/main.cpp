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

int main() {

    lynks_server server(60000);

    if (!server.start()) {
        std::cerr << "Unable to start server\n";
        return 1;
    }

    while (1) {
        server.update(-1, true);
    }

    return 0;
}