#include "network_lynks.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

class lynks_server : public lynks::network::server_interface {
public:
    lynks_server(uint16_t port) : lynks::network::server_interface(port) {}

    bool on_client_connect(std::shared_ptr<lynks::network::connection> client) override {
        (void)client;
        return true;
    }
};

static uint16_t parse_port_or_default(const char* s, uint16_t def) {
    if (!s || *s == '\0') return def;
    try {
        int p = std::stoi(std::string(s));
        if (p < 1 || p > 65535) return def;
        return static_cast<uint16_t>(p);
    } catch (...) {
        return def;
    }
}

int main(int argc, char** argv) {
    uint16_t port = parse_port_or_default(std::getenv("PORT"), 60000);

    if (argc >= 2) {
        port = parse_port_or_default(argv[1], port);
    }

    lynks_server server(port);

    if (!server.start()) {
        std::cerr << "Unable to start server on port " << port << "\n";
        return 1;
    }

    while (true) {
        server.update(-1, true);
    }
}