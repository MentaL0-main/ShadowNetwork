#include <arpa/inet.h>
#include <cstdint>
#include <socket.hpp>
#include <stdexcept>
#include <vector>

namespace ShadowNetwork {

Socket::Socket(int port) {}

Socket::~Socket() {}

bool Socket::listen() {}

int Socket::accept() {}

std::vector<uint8_t> &receive(int client_fd, size_t size) {}

void Socket::send(int client_fd, const std::vector<uint8_t> &data) {}

void Socket::close(int fd) {}

} // namespace ShadowNetwork
