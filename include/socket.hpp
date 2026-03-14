#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <vector>

namespace ShadowNetwork {

class Socket {
public:
  Socket(int port);
  ~Socket();

  bool listen();
  int accept();
  std::vector<uint8_t> receive(int client_fd, size_t size);
  void send(int client_fd, const std::vector<uint8_t> &data);
  void close(int fd);

private:
  int server_fd_ = -1;
  sockaddr_in server_addr_{}, client_addr_{};
};

} // namespace ShadowNetwork
