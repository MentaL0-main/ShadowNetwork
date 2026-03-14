#include <ShadowNetwork/socket.hpp>

#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace sn {

int Socket::connect_to_server(const char *ip, int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &addr.sin_addr);

  if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    return -1;
  }

  return sock;
}

ssize_t Socket::write_all(int fd, const void *buf, size_t len) {
  size_t total = 0;
  const char *p = (const char *)buf;

  while (total < len) {
    ssize_t n = write(fd, p + total, len - total);
    if (n <= 0)
      return n;
    total += n;
  }
  return total;
}

ssize_t Socket::read_all(int fd, void *buf, size_t len) {
  size_t total = 0;
  char *p = (char *)buf;

  while (total < len) {
    ssize_t n = read(fd, p + total, len - total);
    if (n <= 0)
      return n;
    total += n;
  }
  return total;
}

} // namespace sn
