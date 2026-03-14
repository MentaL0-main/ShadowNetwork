#pragma once

#include "socket.hpp"

#include <string>

namespace sn {

class ShadowNetwork {
public:
  void run();

private:
  Socket socket_;

  std::string white_address_;
  void choose_white_address();

  int tun_fd_ = -1;
  std::string tun_name_;
  void init_tun();
  void activate_tun(const std::string &ip_addr);
  void start_polling();
  void start_proxy(int sock_fd);

  bool try_connect(const std::string &url);
};

} // namespace sn
