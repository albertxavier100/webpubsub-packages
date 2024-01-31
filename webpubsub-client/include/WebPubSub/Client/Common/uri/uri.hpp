#pragma once

#include <map>
#include <string>

// TODO: use a professional one
class uri {
public:
  uri(const std::string &uri_str) {
    size_t protocol_end = uri_str.find("://");
    if (protocol_end != std::string::npos) {
      protocol_ = uri_str.substr(0, protocol_end);
      protocol_end += 3; // skip "://"
    } else {
      protocol_end = 0;
    }

    size_t hostname_end = uri_str.find(":", protocol_end);
    if (hostname_end != std::string::npos) {
      host_ = uri_str.substr(protocol_end, hostname_end - protocol_end);
      protocol_end = hostname_end + 1; // skip ":"
      hostname_end = uri_str.find("/", protocol_end);
      port_ = uri_str.substr(protocol_end, hostname_end - protocol_end);
      protocol_end = hostname_end;
    } else {
      hostname_end = uri_str.find("/", protocol_end);
      host_ = uri_str.substr(protocol_end, hostname_end - protocol_end);
      protocol_end = hostname_end;
    }

    hostname_end = uri_str.find("?", protocol_end);
    if (hostname_end != std::string::npos) {
      path_ = uri_str.substr(protocol_end, hostname_end - protocol_end);
      protocol_end = hostname_end + 1;
    } else {
      path_ = uri_str.substr(protocol_end);
      return;
    }

    while ((hostname_end = uri_str.find("=", protocol_end)) != std::string::npos) {
      std::string key = uri_str.substr(protocol_end, hostname_end - protocol_end);
      protocol_end = hostname_end + 1;
      hostname_end = uri_str.find("&", protocol_end);
      if (hostname_end != std::string::npos) {
        std::string value = uri_str.substr(protocol_end, hostname_end - protocol_end);
        protocol_end = hostname_end + 1;
        query_params_.insert({key, value});
      } else {
        std::string value = uri_str.substr(protocol_end);
        query_params_.insert({key, value});
        break;
      }
    }
  }

  std::string get_protocol() { return protocol_; }
  std::string get_host() { return host_; }
  std::string get_path() { return path_; }
  std::string get_port() { return port_; }
  std::multimap<std::string, std::string> get_query_params() {
    return query_params_;
  }

  void set_protocol(const std::string &new_protocol) {
    protocol_ = new_protocol;
  }
  void set_host(const std::string &new_host) { host_ = new_host; }
  void set_path(const std::string &new_path) { path_ = new_path; }
  void set_port(const std::string &new_port) { port_ = new_port; }
  void set_query_params(
      const std::multimap<std::string, std::string> &new_query_params) {
    query_params_ = new_query_params;
  }

  void add_query_param(const std::string &key, const std::string &value) {
    query_params_.insert({key, value});
  }
  void remove_query_param(const std::string &key) { query_params_.erase(key); }

  std::string to_string() {
    std::string uri = protocol_ + "://" + host_;

    if (!port_.empty())
      uri += ":" + port_;

    uri += path_;

    if (!query_params_.empty()) {
      uri += "?";

      for (auto it = query_params_.begin(); it != query_params_.end(); ++it) {
        if (it != query_params_.begin())
          uri += "&";

        uri += it->first + "=" + it->second;
      }
    }

    return uri;
  }

private:
  std::string protocol_;
  std::string host_;
  std::string path_;
  std::string port_;
  std::multimap<std::string, std::string> query_params_;
};