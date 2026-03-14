#pragma once

#include <string>

namespace sn {

class ShadowNetwork {
public:
  void run();

private:
  std::string white_address_;
  void choose_white_address();

  bool try_connect(const std::string &url);
};

} // namespace sn
