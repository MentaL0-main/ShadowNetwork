#include <ShadowNetwork/ShadowNetwork.hpp>

#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

#include <curl/curl.h>

namespace sn {

void ShadowNetwork::run() { choose_white_address(); }

void ShadowNetwork::choose_white_address() {
  std::cout << "[*] Choosing a white address..." << std::flush;

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
