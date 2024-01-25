#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <WebPubSub/client/async/task_cancellation/cancellation_token.hpp>
#include <WebPubSub/client/async/task_cancellation/cancellation_token_source.hpp>
#include <WebPubSub/client/concepts/web_socket_factory_t.hpp>
#include <WebPubSub/client/credentials/client_credential.hpp>
// #include <WebPubSub/client/detail/client/ack_entity.hpp>
#include <WebPubSub/client/detail/client/group_context.hpp>
#include <WebPubSub/client/detail/client/group_context_store.hpp>
#include <WebPubSub/client/detail/client/sequence_id.hpp>
#include <WebPubSub/client/models/client_options.hpp>
#include <WebPubSub/client/models/client_state.hpp>
#include <WebPubSub/client/models/io_service.hpp>
#include <WebPubSub/client/models/request_result.hpp>
#include <WebPubSub/client/policies/retry_policy.hpp>
#include <asio/awaitable.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/use_awaitable.hpp>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <webpubsub/client/common/scope/scope_guard.hpp>
#include <webpubsub/client/common/web_socket/web_socket_close_status.hpp>

namespace webpubsub {
template <web_socket_factory_t WebSocketFactory, web_socket_t WebSocket,
          typename WebPubSubProtocol = reliable_json_v1_protocol>
class client {
  using request_result_or_exception =
      std::variant<request_result, std::invalid_argument, std::exception>;

public:
  client(client_options<WebPubSubProtocol> &options,
         const client_credential &credential,
         const WebSocketFactory &web_socket_factory, io_service &io_service)
      : options_(options), credential_(credential), io_service_(io_service),
        stop_cts_(io_service_.get_io_context()),
        client_("", "", io_service_.get_io_context()),
        web_socket_factory_(web_socket_factory) {}

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

  asio::awaitable<request_result>
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

    client_access_uri_ = credential_.getClientAccessUri();
    co_await async_connect(client_access_uri_, cancellation_token);
  }

  asio::awaitable<void> async_connect(const std::string &uri,
                                      const std::optional<cancellation_token>
                                          &cancellation_token = std::nullopt) {
    auto client = web_socket_factory_.create(uri, options_.protocol.get_name());
    co_await client.async_connect(cancellation_token);
    client_ = client;
    client_state_.change_state(client_state::connected);
    asio::co_spawn(io_service_.get_io_context(),
                   async_start_listen_loop(stop_cts_.get_token()),
                   asio::detached);
  }

  asio::awaitable<void>
  handle_connection_close(const web_socket_close_status &web_socket_status,
                          const cancellation_token &cancellation_token) {
    // TODO
    co_return;
  }

  asio::awaitable<void>
  async_start_listen_loop(const cancellation_token &cancellation_token) {
    using namespace asio::experimental::awaitable_operators;

    web_socket_close_status web_socket_status = web_socket_close_status::empty;
    cancellation_token_source sequence_ack_cts(io_service_.get_io_context());

    if (options_.protocol.is_reliable()) {
      asio::co_spawn(
          io_service_.get_io_context(),
          async_start_sequence_ack_loop(sequence_ack_cts.get_token()) ||
              sequence_ack_cts.get_token().async_cancel(),
          [&web_socket_status, &cancellation_token, this](auto e1, auto e2) {
            // TODO handle connection close
            handle_connection_close(web_socket_status, cancellation_token);
          });
    }

    {
      scope_guard guard(io_service_.get_io_context(),
                        [&sequence_ack_cts]() -> void {
                          std::cout << "log web socket closed\n";
                          sequence_ack_cts.cancel();
                        });

      try {
        while (!cancellation_token.is_cancelled()) {
          uint64_t *start;
          uint64_t size;
          co_await (client_.async_read(start, size, web_socket_status) ||
                    cancellation_token.async_cancel());

          // TODO change int to enum
          if (web_socket_status != web_socket_close_status::empty) {
            break;
          }
          if (size > 0) {
            try {
              // TODO: .net receive multiple messages here, investigate later
            } catch (...) {
              // TODO: add raw exception in my own exception
              throw;
            }
          }
        }
      } catch (...) {
        std::cout << "log failed to receive exception\n";
      }
    }
  }

  asio::awaitable<void>
  async_start_sequence_ack_loop(const cancellation_token &cancellation_token) {
    using namespace std::chrono_literals;
    using namespace asio::experimental::awaitable_operators;

    while (!cancellation_token.is_cancelled()) {
      scope_guard guard(io_service_.get_io_context(),
                        [this, &cancellation_token]() -> asio::awaitable<void> {
                          co_await (webpubsub::async_delay(
                                        io_service_.get_io_context(), 1s) ||
                                    cancellation_token.async_cancel());
                        });
      try {
        uint64_t id;
        if (sequence_id_.try_get_sequence_id(id)) {
          auto req = SequenceAckSignal(id);
          auto payload = options_.protocol.write(std::move(req));
          co_await (async_send_core(
                        std::move(payload),
                        options_.protocol.get_webpubsub_protocol_message_type(),
                        cancellation_token) ||
                    cancellation_token.async_cancel());
        }
      } catch (...) {
      }
    }
  }

  asio::awaitable<void>
  async_send_core(std::string payload,
                  WebPubSubProtocolMessageType webpubsub_protocol_message_type,
                  const std::optional<cancellation_token> &cancellation_token =
                      std::nullopt) {
    using namespace asio::experimental::awaitable_operators;

    auto as_text =
        webpubsub_protocol_message_type == WebPubSubProtocolMessageText;
    auto size = payload.size() + (as_text ? 1 : 0);
    auto start = reinterpret_cast<const uint64_t *>(payload.c_str());
    co_await (client_.async_write(start, size, as_text) ||
              cancellation_token.value().async_cancel());
  }

private:
  io_service &io_service_;
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
  std::string client_access_uri_; // TODO: use a URI class?
  std::string connection_id_;
  std::string reconnection_token_;
  bool is_initial_connected_ = false;
  std::optional<DisconnectedResponse> latest_disconnected_response_;
  // TODO: change

  std::unordered_map<uint16_t, int /* TODO: only for dev */
                     /*task_completion_source<request_result_or_exception>*/>
      ack_cache_;
#pragma endregion

#pragma region fields per web socket
  WebSocket client_;
  WebSocketFactory web_socket_factory_;
#pragma endregion
};
} // namespace webpubsub