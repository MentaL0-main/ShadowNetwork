#include <ShadowNetwork/ShadowNetwork.hpp>

#include <exception>
#include <iostream>

int main() {
  sn::ShadowNetwork shnet;

  try {
    shnet.run();
  } catch (std::exception &error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }

  return 0;
}
