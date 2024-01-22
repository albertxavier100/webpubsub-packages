#pragma once
#include <WebPubSub/Client/Common/Macros.hpp>
#include <WebPubSub/Client/Credentials/WebPubSubCredential.hpp>
#include <WebPubSub/Client/async/task_cancellation/cancellation_token.hpp>
#include <WebPubSub/Client/async/task_cancellation/cancellation_token_source.hpp>
#include <WebPubSub/Client/concepts/web_socket_factory_t.hpp>
#include <WebPubSub/Client/detail/client/group_context.hpp>
#include <WebPubSub/Client/detail/client/group_context_store.hpp>
#include <WebPubSub/Client/detail/client/sequence_id.hpp>
#include <WebPubSub/Client/models/client_options.hpp>
#include <WebPubSub/Client/models/client_state.hpp>
#include <WebPubSub/Client/models/io_service.hpp>
#include <WebPubSub/Client/models/result.hpp>
#include <WebPubSub/Client/policies/RetryPolicy.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <asio/awaitable.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/use_awaitable.hpp>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

namespace webpubsub {
template <web_socket_factory_t WebSocketFactory, web_socket_t WebSocket,
          typename WebPubSubProtocol = reliable_json_v1_protocol>
class client {
public:
  client(client_options<WebPubSubProtocol> &options,
         const client_credential &credential,
         const WebSocketFactory &web_socket_factory)
      : options_(options), credential_(credential), io_service_(),
        stop_cts_(io_service_.get_io_context()),
        web_socket_(io_service_.get_io_context()) {}

#pragma region webpubsub api for awaitable
  asio::awaitable<void> async_start(const std::optional<cancellation_token>
                                        &cancellation_token = std::nullopt) {
    if (stop_cts_.is_cancellation_requested()) {
      throw std::invalid_argument("Can not start a client during stopping");
    }

    if (client_state_.get_state() != client_state::stopped) {
      throw std::invalid_argument(
          "Client can be only started when the state is Stopped");
    }

    try {
      co_await async_start_core(cancellation_token);
    } catch (...) {
    }
  }

  asio::awaitable<result>
  async_join_group(const std::string &group,
                   const std::optional<uint64_t> &ack_id,
                   const std::optional<cancellation_token> &cancellation_token =
                       std::nullopt) {}
#pragma endregion

#pragma region getters and setters
  const std::string &get_connection_id() { return connection_id_; }
#pragma endregion

#pragma region asio
  io_service &get_io_service() { return io_service_; }
#pragma endregion

private:
  asio::awaitable<void>
  async_start_core(const std::optional<cancellation_token> &cancellation_token =
                       std::nullopt) {
    client_state_.change_state(client_state::connecting);
    std::cout << "log conn start" << std::endl;

    sequence_id_.reset();
    is_initial_connected_ = false;
    latest_disconnected_response_.reset();
    reconnection_token_.clear();
    connection_id_.clear();
    co_return;
  }

private:
  io_service io_service_;
  const client_options<WebPubSubProtocol> &options_;
  const client_credential &credential_;
  detail::sequence_id sequence_id_;
  detail::group_context_store groups_;
  retry_policy reconnect_retry_policy_;
  retry_policy message_retry_policy_;
  client_state client_state_;
  uint64_t next_ack_id_;

#pragma region Fields per start stop
  cancellation_token_source stop_cts_;
#pragma endregion

#pragma region fields per connection id
  std::string uri_; // TODO: use a URI class?
  std::string connection_id_;
  std::string reconnection_token_;
  bool is_initial_connected_ = false;
  std::optional<DisconnectedResponse> latest_disconnected_response_;
  // TODO: change
  std::unordered_map<uint16_t, int> ack_cache_;
#pragma endregion

#pragma region fields per web socket
  WebSocket web_socket_;
#pragma endregion
};
} // namespace webpubsub