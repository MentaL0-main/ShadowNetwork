#include <ShadowNetwork/ShadowNetwork.hpp>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <ostream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

#include <curl/curl.h>

namespace sn {

void ShadowNetwork::run() {
  choose_white_address();
  init_tun();
}

void ShadowNetwork::choose_white_address() {
  std::cout << "[*] Choising a white address..." << std::flush;

  std::ifstream file;
  file.open("../whitelist.txt");
  if (file.is_open()) {

    bool err_flag = true;
    while (std::getline(file, white_address_)) {
      if (try_connect(white_address_)) {
        err_flag = false;
        break;
      }
    }

    if (err_flag)
      throw std::runtime_error("Failed to find white address");

  } else {
    throw std::runtime_error("Failed to open whitelist.txt");
  }

  std::cout << "OK: " << white_address_ << std::endl;
}

void ShadowNetwork::init_tun() {
  std::cout << "[*] Creating tun interface..." << std::flush;

  ifreq ifr;
  int err;

  if ((tun_fd_ = open("/dev/net/tun", O_RDWR)) < 0) {
    throw std::runtime_error("Failed to open /dev/net/tun");
  }

  std::string tun_name = "tun%d";

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  if (tun_name.c_str()) {
    strncpy(ifr.ifr_name, tun_name.c_str(), IFNAMSIZ);
  }

  if ((err = ioctl(tun_fd_, TUNSETIFF, (void *)&ifr)) < 0) {
    close(tun_fd_);
    throw std::runtime_error("Error ioctl TUNSETIFF. Maybe try with root?");
  }

  tun_name = ifr.ifr_name;
  std::cout << "OK: " << tun_name << std::endl;
}

bool ShadowNetwork::try_connect(const std::string &url) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    throw std::runtime_error("Failed to init curl");
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);

  CURLcode res = curl_easy_perform(curl);
  long response_code = 0;

  if (res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  }

  return (res == CURLE_OK && response_code >= 200 && response_code < 400);
}

} // namespace sn
