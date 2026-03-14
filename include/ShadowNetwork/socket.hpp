#pragma once

#include <sys/types.h>

namespace sn {

struct Socket {
  int connect_to_server(const char *ip, int port);

  ssize_t write_all(int fd, const void *buf, size_t len);
  ssize_t read_all(int fd, void *buf, size_t len);
};

} // namespace sn
