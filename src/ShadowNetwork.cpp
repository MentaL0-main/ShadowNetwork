#include <ShadowNetwork/ShadowNetwork.hpp>

#include <ShadowNetwork/socket.hpp>

#include <arpa/inet.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <ostream>
#include <poll.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

#include <curl/curl.h>

namespace sn {

void ShadowNetwork::run() {
  choose_white_address();
  init_tun();
  activate_tun("10.0.0.5");

  int sock_fd = socket_.connect_to_server("127.0.0.1", 4433);
  if (sock_fd < 0)
    return;

  start_proxy(sock_fd);
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

  tun_name_ = "tun%d";

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  if (tun_name_.c_str()) {
    strncpy(ifr.ifr_name, tun_name_.c_str(), IFNAMSIZ);
  }

  if ((err = ioctl(tun_fd_, TUNSETIFF, (void *)&ifr)) < 0) {
    close(tun_fd_);
    throw std::runtime_error("Error ioctl TUNSETIFF. Maybe try with root?");
  }

  tun_name_ = ifr.ifr_name;
  std::cout << "OK: " << tun_name_ << std::endl;
}

void ShadowNetwork::activate_tun(const std::string &ip_addr) {
  std::cout << "[*] Activating tun interface..." << std::flush;
  std::string up_cmd = "ip link set " + tun_name_ + " up";
  if (system(up_cmd.c_str()) != 0) {
    throw std::runtime_error("Failed to UP tun");
  }

  std::string addr_cmd = "ip addr add " + ip_addr + "/24 dev " + tun_name_;
  if (system(addr_cmd.c_str()) != 0) {
    throw std::runtime_error("Failed to set IP");
  }

  std::cout << "OK: Interface " << tun_name_ << " is UP with IP " << ip_addr
            << std::endl;
}

void ShadowNetwork::start_polling() {
  unsigned char buffer[2048];
  std::cout << "[*] Waiting packets..." << std::endl;

  while (true) {
    size_t nread = read(tun_fd_, buffer, sizeof(buffer));

    std::cout << "Read " << nread << " bytes. IP Version: " << (buffer[0] >> 4)
              << std::endl;
  }
}

void ShadowNetwork::start_proxy(int sock_fd) {
  pollfd fds[2];
  fds[0].fd = tun_fd_;
  fds[0].events = POLLIN;
  fds[1].fd = sock_fd;
  fds[1].events = POLLIN;

  std::vector<uint8_t> buffer(2048);

  std::cout << "[*] Proxy loop started..." << std::endl;

  while (true) {
    int ret = poll(fds, 2, -1);
    if (ret < 0) {
      perror("poll");
      break;
    }

    if (fds[0].revents & POLLIN) {
      ssize_t nread = read(tun_fd_, buffer.data(), buffer.size());
      if (nread > 0) {
        uint16_t len = htons((uint16_t)nread);
        socket_.write_all(sock_fd, &len, 2);
        socket_.write_all(sock_fd, buffer.data(), nread);
      }
    }

    if (fds[1].revents & POLLIN) {
      uint16_t len_net;
      if (socket_.read_all(sock_fd, &len_net, 2) <= 0)
        break;

      uint16_t len = ntohs(len_net);
      if (len > buffer.size())
        buffer.resize(len);

      if (socket_.read_all(sock_fd, buffer.data(), len) > 0) {
        write(tun_fd_, buffer.data(), len);
      }
    }

    if (fds[0].revents & (POLLHUP | POLLERR) ||
        fds[1].revents & (POLLHUP | POLLERR)) {
      std::cout << "[!] Connection closed" << std::endl;
      break;
    }
  }
}

bool ShadowNetwork::try_connect(const std::string &url) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    throw std::runtime_error("Failed to init curl");
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);

  CURLcode res = curl_easy_perform(curl);
  long response_code = 0;

  if (res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  }

  return (res == CURLE_OK && response_code >= 200 && response_code < 400);
}

} // namespace sn
