#pragma once

#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <asio/awaitable.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <webpubsub/client/async/task_cancellation/cancellation_token.hpp>
#include <webpubsub/client/async/task_cancellation/cancellation_token_source.hpp>
#include <webpubsub/client/async/task_completion/task_completion_source.hpp>
#include <webpubsub/client/common/constants.hpp>
#include <webpubsub/client/common/scope/scope_guard.hpp>
#include <webpubsub/client/common/uri/uri.hpp>
#include <webpubsub/client/common/web_socket/web_socket_close_status.hpp>
#include <webpubsub/client/concepts/web_socket_factory_t.hpp>
#include <webpubsub/client/credentials/client_credential.hpp>
#include <webpubsub/client/detail/client/group_context.hpp>
#include <webpubsub/client/detail/client/group_context_store.hpp>
#include <webpubsub/client/detail/client/sequence_id.hpp>
#include <webpubsub/client/models/client_options.hpp>
#include <webpubsub/client/models/client_state.hpp>
#include <webpubsub/client/models/io_service.hpp>
#include <webpubsub/client/models/request_result.hpp>
#include <webpubsub/client/policies/retry_policy.hpp>

namespace webpubsub {
template <typename WebSocketFactory, typename WebSocket,
          typename WebPubSubProtocol = reliable_json_v1_protocol>
  requires web_socket_factory_t<WebSocketFactory, WebSocket>
class client {
  using request_result_or_exception =
      std::variant<request_result, std::invalid_argument, std::exception>;

public:
  client(client_options<WebPubSubProtocol> &options,
         const client_credential &credential,
         const WebSocketFactory &web_socket_factory, io_service &io_service)
      : options_(options), credential_(credential), io_service_(io_service),
        stop_cts_(io_service_.get_io_context()), client_(nullptr),
        web_socket_factory_(web_socket_factory) {}

#pragma region webpubsub api for awaitable
  asio::awaitable<void> async_start(const std::optional<cancellation_token>
                                        &cancellation_token = std::nullopt) {
    const auto token = cancellation_token.has_value()
                           ? cancellation_token.value()
                           : stop_cts_.get_token();
    if (stop_cts_.is_cancellation_requested()) {
      throw std::invalid_argument("Can not start a client during stopping");
    }

    if (client_state_.get_state() != client_state::stopped) {
      throw std::invalid_argument(
          "Client can be only started when the state is Stopped");
    }

    try {
      co_await async_start_core(token);
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
  async_start_core(const cancellation_token &cancellation_token) {
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

  asio::awaitable<void>
  async_connect(const std::string &uri,
                const cancellation_token &cancellation_token) {
    using namespace asio::experimental::awaitable_operators;
    auto client = web_socket_factory_.create(uri, options_.protocol.get_name());
    co_await client->async_connect(cancellation_token);
    client_ = std::move(client);
    client_state_.change_state(client_state::connected);
    asio::co_spawn(
        io_service_.get_io_context(),
        async_start_listen_loop(cancellation_token) ||
            cancellation_token.async_cancel(), // TODO: consider std::nullopt
        asio::detached);
  }

  asio::awaitable<void>
  handle_connection_close(const web_socket_close_status &web_socket_status,
                          const cancellation_token &cancellation_token) {
    for (auto &[ack_id, ack_completion_source] : ack_cache_) {
      if (ack_cache_.find(ack_id) != ack_cache_.end()) {
        auto message = std::format("Connection is disconnected before receive "
                                   "ack from the service. Ack ID: {}",
                                   ack_id);
        // TODO use my own exception
        if (!ack_completion_source->set_value_once(
                std::invalid_argument(message))) {
          std::cout << "log already set ack id tcs \n";
        }
      }
    }

    // close connection without recover
    std::string recovery_uri;
    {
      using namespace std::chrono_literals;

      bool should_recover = true;
      if (web_socket_status == web_socket_close_status::policy_violation) {
        std::cout << "log The web socket close with status: policy_violation\n";
        should_recover = false;
      } else if (cancellation_token.is_cancelled()) {
        std::cout << "log The client is stopped\n";
        should_recover = false;
      } else if (!options_.protocol.is_reliable()) {
        std::cout
            << "The protocol is not reliable, recovery is not applicable \n";
        should_recover = false;
      } else if (!try_build_recovery_uri(recovery_uri)) {
        std::cout << "Connection id or reconnection token is not availble\n";
        should_recover = false;
      }
      if (!should_recover) {
        co_await handle_connection_close_and_no_recovery(cancellation_token);
        co_return;
      }
    }

    // recover
    {
      using namespace std::chrono_literals;

      bool recovered = false;
      client_state_.change_state(client_state::recovering);
      cancellation_token_source cts(io_service_.get_io_context());
      cts.cancel_after(30s);
      try {
        scope_guard guard(
            io_service_.get_io_context(),
            [&recovered, this, &cancellation_token]() -> asio::awaitable<void> {
              if (!recovered) {
                std::cout
                    << "log recovery attempts failed more than 30s or the "
                       "client is stopped. \n";
                co_await handle_connection_close_and_no_recovery(
                    cancellation_token /* TODO should be linked token */);
              }
            });
        for (; !cts.is_cancellation_requested() ||
               !cancellation_token.is_cancelled();) {
          try {
            cancellation_token_source cts(io_service_.get_io_context());
            co_await async_connect(recovery_uri, cts.get_token());
            recovered = true;
            co_return;
          } catch (const std::exception &e) {
            std::cout << "log fail to recover connection \n";
            std::cerr << e.what() << '\n';
          }
          co_await async_delay(
              io_service_.get_io_context(),
              recover_delay_); // TODO: add cancellation token here
        }
      } catch (const std::exception &e) {
      }

      co_return;
    }
  }

  bool try_build_recovery_uri(std::string &recovery_uri) {
    namespace q = webpubsub::constants::query;
    if (!connection_id_.empty() && !reconnection_token_.empty()) {
      uri uri(client_access_uri_);
      uri.add_query_param(q::recover_connection_id_query_, connection_id_);
      uri.add_query_param(q::recover_connection_token_query_,
                          reconnection_token_);
      recovery_uri = uri.to_string();
      return true;
    }
    recovery_uri = "";
    return false;
  }

  asio::awaitable<void> handle_connection_close_and_no_recovery(
      const cancellation_token &cancellation_token) {
    using namespace asio::experimental::awaitable_operators;

    client_state_.change_state(client_state::disconnected);
    co_spawn(io_service_.get_io_context(),
             async_safe_invoke_disconnected_async(), asio::detached);
    if (options_.auto_reconnect) {
      co_await (async_auto_reconnect(cancellation_token) ||
                cancellation_token.async_cancel());
    }
    handle_client_stopped();
    co_return;
  }

  // TODO: impl
  asio::awaitable<void> async_safe_invoke_disconnected_async() { co_return; }

  // TODO: impl
  void handle_client_stopped() {}

  // TODO: impl
  asio::awaitable<void>
  async_auto_reconnect(const cancellation_token &cancellation_token) {
    co_return;
  }

  asio::awaitable<void>
  async_start_listen_loop(const cancellation_token &cancellation_token) {
    using namespace asio::experimental::awaitable_operators;

    web_socket_close_status web_socket_close_status =
        web_socket_close_status::empty;
    cancellation_token_source sequence_ack_cts(io_service_.get_io_context());

    if (options_.protocol.is_reliable()) {
      asio::co_spawn(
          io_service_.get_io_context(),
          async_start_sequence_ack_loop(sequence_ack_cts.get_token()) ||
              sequence_ack_cts.get_token().async_cancel(),
          [&web_socket_close_status, &cancellation_token, this](auto e1,
                                                                auto e2) {
            asio::co_spawn(io_service_.get_io_context(),
                           handle_connection_close(web_socket_close_status,
                                                   cancellation_token) ||
                               cancellation_token.async_cancel(),
                           asio::detached);
            ;
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
          co_await (client_->async_read(start, size, web_socket_close_status) ||
                    cancellation_token.async_cancel());

          // TODO change int to enum
          if (web_socket_close_status != web_socket_close_status::empty) {
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
    co_await (client_->async_write(start, size, as_text) ||
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
  asio::steady_timer::duration recover_delay_ = std::chrono::seconds(1);

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

  std::unordered_map<
      uint16_t,
      std::unique_ptr<task_completion_source<request_result_or_exception>>>
      ack_cache_;
#pragma endregion

#pragma region fields per web socket
  std::unique_ptr<WebSocket> client_;
  WebSocketFactory web_socket_factory_;
#pragma endregion
};
} // namespace webpubsub
