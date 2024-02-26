
#pragma once

#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <asio/awaitable.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/experimental/channel.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <eventpp/callbacklist.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <webpubsub/client/common/constants.hpp>
#include <webpubsub/client/common/uri/uri.hpp>
#include <webpubsub/client/common/websocket/websocket_close_status.hpp>
#include <webpubsub/client/concepts/websocket_factory_c.hpp>
#include <webpubsub/client/credentials/client_credential.hpp>
#include <webpubsub/client/detail/async/exclusion_lock.hpp>
#include <webpubsub/client/detail/async/utils.hpp>
#include <webpubsub/client/detail/client/client_context.hpp>
#include <webpubsub/client/detail/client/group_context.hpp>
#include <webpubsub/client/detail/client/group_context_store.hpp>
#include <webpubsub/client/detail/client/sequence_id.hpp>
#include <webpubsub/client/models/callback_contexts.hpp>
#include <webpubsub/client/models/client_options.hpp>
#include <webpubsub/client/models/client_state.hpp>
#include <webpubsub/client/models/io_service.hpp>
#include <webpubsub/client/models/request_result.hpp>
#include <webpubsub/client/policies/retry_policy.hpp>

namespace webpubsub {
template <typename WebSocketFactory, typename WebSocket,
          typename WebPubSubProtocol = reliable_json_v1_protocol>
  requires websocket_factory_t<WebSocketFactory, WebSocket>
class client {
  using task = asio::awaitable<void>;
  static constexpr asio::steady_timer::time_point max_time_point =
      asio::steady_timer::time_point::max();

private:
  enum class coro_result { cancelled, completed, running };

public:
#pragma region callbacks
  eventpp::CallbackList<void(const connected_context)> on_connected;
  eventpp::CallbackList<void(const disconnected_context)> on_disconnected;
  eventpp::CallbackList<void(const group_data_context)> on_group_data;
  eventpp::CallbackList<void(const server_data_context)> on_server_data;
  eventpp::CallbackList<void(const rejoin_group_failed_context)>
      on_rejoin_group_failed;
  eventpp::CallbackList<void(const stopped_context)> on_stopped;
#pragma endregion

public:
  client(client_options<WebPubSubProtocol> &options,
         const client_credential &credential,
         const WebSocketFactory &web_socket_factory, io_service &io_service)
      : logger_(init_logger()), options_(options), credential_(credential),
        io_service_(io_service), client_(nullptr),
        web_socket_factory_(web_socket_factory),
        stop_lock_(io_service_.get_io_context()),
        listen_loop_stop_notice_(io_service_.get_io_context(), max_time_point),
        sequence_id_ack_loop_stop_notice_(io_service_.get_io_context(),
                                          max_time_point) {}

public:
#pragma region webpubsub api for awaitable

  task async_stop() { co_await async_stop_core(); }
  task async_start() {
    try {
      auto slot = listen_loop_cancel_signal_.slot();
      auto token = asio::bind_cancellation_slot(slot, asio::use_awaitable);
      auto &ioc = io_service_.get_io_context();
      co_await asio::co_spawn(ioc, async_start_internal(), token);
    } catch (const asio::system_error &error) {
      listen_loop_cancel_signal_.emit(asio::cancellation_type::all);
    }
  }
  task async_start_internal() {
    try {
      co_await stop_lock_.async_lock();
      co_await stop_lock_.async_release();

      if (client_state_.get_state() != client_state::stopped) {
        throw std::invalid_argument(
            "Client can be only started when the state is Stopped");
      }

      try {
        co_await async_start_core();
      } catch (...) {
      }
    } catch (const asio::system_error &error) {
      throw std::invalid_argument("Can not start a client during stopping");
    }
  }

  asio::awaitable<request_result>
  async_join_group(const std::string &group,
                   const std::optional<uint64_t> &ack_id) {
    co_return;
  }

#pragma endregion

#pragma region getters and setters

  const std::string &get_connection_id() { return connection_id_; }

#pragma endregion

#pragma region asio

  io_service &get_io_service() { return io_service_; }

#pragma endregion

private:
  task async_stop_core() {
    try {
      co_await stop_lock_.async_lock();

      try {
        co_await client_->async_close();
      } catch (...) {
        std::cout << "here\n";
      }

      try {
        listen_loop_cancel_signal_.emit(asio::cancellation_type::all);
        std::cout << "listen_loop_stop_notice_.async_wait start\n";

        try {
          co_await listen_loop_stop_notice_.async_wait(asio::use_awaitable);
        } catch (...) {
        }
        std::cout << "listen_loop_stop_notice_.async_wait finish\n";
      } catch (...) {
        std::cout << "here\n";
      }
    } catch (...) {
      std::cout << "here\n";
    }
    /* finally */ {
      co_await stop_lock_.async_release();
      std::cout << "release stop lock\n";
    }
  };

  // TODO: allow add customize sink std::shared_ptr<spdlog::logger>
  std::shared_ptr<spdlog::logger> init_logger() {
    std::string logger_name("__webpubsub_client_logger__");
    spdlog::init_thread_pool(8192, 1);
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/webpubsub_client.log", 1024 * 1024 * 10, 3);
    std::vector<spdlog::sink_ptr> sinks{rotating_sink};
    auto logger = std::make_shared<spdlog::async_logger>(
        logger_name, sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    // spdlog::register_logger(logger);
    return logger;
  }

  task async_start_core() {
    client_state_.change_state(client_state::connecting);
    std::cout << "log conn start" << std::endl;

    sequence_id_.reset();
    is_initial_connected_ = false;
    latest_disconnected_response_.reset();
    reconnection_token_.clear();
    connection_id_.clear();

    client_access_uri_ = credential_.getClientAccessUri();
    co_await async_connect(client_access_uri_);
  }

  task async_connect(const std::string &uri) {
    using namespace asio::experimental::awaitable_operators;
    auto client = web_socket_factory_.create(uri, options_.protocol.get_name());
    co_await client->async_connect();
    client_ = std::move(client);
    client_state_.change_state(client_state::connected);

    auto &ioc = io_service_.get_io_context();
    auto token = asio::bind_cancellation_slot(listen_loop_cancel_signal_.slot(),
                                              asio::detached);
    asio::co_spawn(ioc, async_run_listen_loop(), token);
  }

  task async_handle_connection_close(
      const websocket_close_status &web_socket_status) {
    // TODO: handle ack cache
    // for (auto &[ack_id, ack_completion_source] : ack_cache_) {
    //  if (ack_cache_.find(ack_id) != ack_cache_.end()) {
    //    auto message = std::format("Connection is disconnected before receive
    //    "
    //                               "ack from the service. Ack ID: {}",
    //                               ack_id);
    //    // TODO use my own exception
    //    if (!ack_completion_source->set_value_once(
    //            std::invalid_argument(message))) {
    //      std::cout << "log already set ack id tcs \n";
    //    }
    //  }
    //}

    // close connection without recover
    std::string recovery_uri;
    {
      using namespace std::chrono_literals;

      bool should_recover = true;
      if (web_socket_status == websocket_close_status::policy_violation) {
        std::cout << "log The web socket close with status: policy_violation\n";
        should_recover = false;
      } else if (co_await detail::async_is_coro_cancelled()) {
        std::cout << "log The client is stopped\n";
        should_recover = false;
      } else if (!options_.protocol.is_reliable()) {
        std::cout << "The protocol is not reliable, recovery is not "
                     "applicable \n";
        should_recover = false;
      } else if (!try_build_recovery_uri(recovery_uri)) {
        std::cout << "Connection id or reconnection token is not available\n";
        should_recover = false;
      }
      if (!should_recover) {
        co_await handle_connection_close_and_no_recovery();
        co_return;
      }
    }

    // recover
    {
      using namespace std::chrono_literals;

      bool recovered = false;
      client_state_.change_state(client_state::recovering);
      // TODO: add cts here
      try {
        // TODO: error: cannot link them in this way
        for (auto is_canceled = co_await detail::async_is_coro_cancelled();
             !is_canceled;
             is_canceled = co_await detail::async_is_coro_cancelled()) {
          try {
            co_await async_connect(recovery_uri);
            recovered = true;
            co_return;
          } catch (const std::exception &e) {
            std::cout << "log fail to recover connection \n";
            std::cerr << e.what() << '\n';
          }
          co_await detail::async_delay(
              io_service_.get_io_context(),
              recover_delay_); // TODO: add cancellation token here
        }
      } catch (const std::exception &e) {
      }
      /* finally */ {
        if (!recovered) {
          std::cout << "log recovery attempts failed more than 30s or the "
                       "client is stopped. \n";
          /* TODO should be linked token */
          co_await this->handle_connection_close_and_no_recovery();
        }
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

  task handle_connection_close_and_no_recovery() {
    using namespace asio::experimental::awaitable_operators;

    client_state_.change_state(client_state::disconnected);
    co_spawn(io_service_.get_io_context(), async_safe_invoke_disconnected(),
             asio::detached);
    if (options_.auto_reconnect) {
      co_await async_auto_reconnect();
    }
    handle_client_stopped();
    co_return;
  }

  // TODO: impl
  task async_safe_invoke_disconnected() { co_return; }

  void handle_client_stopped() {
    client_state_.change_state(client_state::stopped);
    async_safe_invoke_stopped();
  }

  // TODO: impl
  void async_safe_invoke_stopped() {}

  task async_auto_reconnect() {
    bool is_success = false;
    uint64_t retry_attempt = 0;
    try {
      for (auto is_canceled = co_await detail::async_is_coro_cancelled();
           !is_canceled;
           is_canceled = co_await detail::async_is_coro_cancelled()) {
        bool catched = false;
        try {
          using namespace asio::experimental::awaitable_operators;
          co_await async_start_by_reconnection();
          is_success = true;
          co_return;
        } catch (const std::exception &e) {
          std::cout << "log failed to reconnect";
          retry_attempt++;
          catched = true;
        }
        /* finally */ {
          if (!catched) {
            co_return;
          }
          retry_context retry_context{retry_attempt};
          auto delay = this->reconnect_retry_policy_.next_retry_delay(
              std::move(retry_context));
          if (!delay.has_value()) {
            co_return;
          }
          if (delay.has_value()) {
            co_await async_delay(io_service_.get_io_context(), delay.value());
          }
        }
      }

    } catch (...) {
    }
    /* finally */ {
      if (!is_success) {
        this->handle_client_stopped();
      }
    }
    co_return;
  }

  task async_start_by_reconnection() {
    // TODO: bind listen_loop_cancel_signal_
    //    if (stop_cts_.is_cancellation_requested()) {
    //      throw std::invalid_argument("Can not start a client during
    //      stopping");
    //    }

    if (client_state_.get_state() != client_state::disconnected) {
      throw std::invalid_argument("Client restart should happen only when "
                                  "the state is Disconnected");
    }

    try {
      co_await async_start();
    } catch (...) {
      client_state_.change_state(client_state::disconnected);
      throw;
    }
  }

  task async_run_sequence_ack_loop() {
    using namespace asio::experimental::awaitable_operators;
    try {
      for (;;) {
        auto cs = co_await asio::this_coro::cancellation_state;
        if (cs.cancelled() != asio::cancellation_type::none) {
          std::cout << "async_run_sequence_ack_loop break \n";
          break;
        }
        try {
          uint64_t id;
          if (sequence_id_.try_get_sequence_id(id)) {
            auto req = SequenceAckSignal(id);
            auto payload = options_.protocol.write(std::move(req));
            auto protocol = options_.protocol;
            auto ws_msg_type = protocol.get_webpubsub_protocol_message_type();
            co_await async_send_core(std::move(payload), ws_msg_type);
          }
        } catch (const std::exception &e) {
          std::cout << "[in] seq loop get exception\n";
        }
        /* finally */ {
          std::cout << "[in] seq loop delay in finally\n";
          co_await detail::async_delay(io_service_.get_io_context(),
                                       asio::chrono::seconds(1));
        }
      }
    } catch (...) {
    }
    /* finally */
    {
      std::cout << "sequence_id_ack_loop_stop_notice_.stop start\n";
      sequence_id_ack_loop_stop_notice_.cancel();
      std::cout << "sequence_id_ack_loop_stop_notice_.stop finish\n";
    }
  }

  void async_detach_run_sequence_ack_loop() {
    auto &ioc = io_service_.get_io_context();
    auto slot = sequence_id_loop_cancel_signal_.slot();
    auto token = asio::bind_cancellation_slot(slot, asio::detached);
    asio::co_spawn(ioc, async_run_sequence_ack_loop(), token);
  }

  // TODO: final check
  // main loop -> sequence id loop -> close connection
  task async_run_listen_loop() {
    using ct = asio::cancellation_type;

    websocket_close_status web_socket_close_status =
        websocket_close_status::empty;

    if (options_.protocol.is_reliable()) {
      async_detach_run_sequence_ack_loop();
    }

    try {
      for (;;) {
        auto cs = co_await asio::this_coro::cancellation_state;
        if (cs.cancelled() != ct::none) {
          break;
        }
        std::string payload;
        co_await client_->async_read(payload, web_socket_close_status);

        // TODO: handle the payload
        if (web_socket_close_status != websocket_close_status::empty) {
          break;
        }

        if (payload.size() > 0) {
          try {
            // TODO: impl

            // TODO: protocol only support text, not support binary here,
            // due to avoid copy. The web socket client can return binary
            // here.
            auto response = options_.protocol.read(std::move(payload));
            if (response) {
              co_await async_handle_response(std::move(*response));

            } else {
              std::cout << "log failed to parse message\n";
            }

          } catch (const std::exception &e) {
            std::cout << "log failed to process response";
            throw;
          }
        }
        // std::cout << "[in-listen-loop] get message in receive loop\n";
      }
    } catch (const std::exception &_) {
      std::cout << "log failed to receive bytes.\n";
    };
    /* finally */ {
      std::cout << "log web socket closed\n";
      sequence_id_loop_cancel_signal_.emit(ct::terminal);
      std::cout << "sequence_id_ack_loop_stop_notice_.async_wait start\n";
      try {
        co_await sequence_id_ack_loop_stop_notice_.async_wait(
            asio::use_awaitable);
        std::cout << "sequence_id_ack_loop_stop_notice_.async_wait finish\n";
      } catch (...) {
      }
      std::cout << "async_handle_connection_close start\n";
      // TODO: [DEBUG] add back later
      // co_await async_handle_connection_close(websocket_close_status);
      std::cout << "async_handle_connection_close finish\n";

      std::cout << "listen_loop_stop_notice_.stop start\n";
      listen_loop_stop_notice_.cancel();
      std::cout << "listen_loop_stop_notice_.stop finish\n";
    };
  }

  task async_handle_response(ResponseVariant response) {
    if (auto resp = std::get_if<ConnectedResponse>(&response)) {
      co_await async_handle_connected_response(*resp);
    } else if (auto resp = std::get_if<DisconnectedResponse>(&response)) {
      handle_disconnected_response(*resp);
    } else if (auto resp = std::get_if<AckResponse>(&response)) {
      handle_ack_response(*resp);
    } else if (auto resp = std::get_if<GroupMessageResponseV2>(&response)) {
      handle_group_data_response(*resp);
    } else if (auto resp = std::get_if<ServerMessageResponse>(&response)) {
      handle_server_data_response(*resp);
    } else {
      throw std::invalid_argument("Received unknown type of message");
    }

    co_return;
  }

  task async_handle_connected_response(ConnectedResponse response) {
    connection_id_ = response.getConnectionId();
    // TODO: check if reconnection token always has value or not
    reconnection_token_ = response.getReconnectionToken().value();

    if (!is_initial_connected_) {
      is_initial_connected_ = true;
      co_await async_handle_connection_connected(response);
    }
  }

  task async_handle_connection_connected(ConnectedResponse response) {
    if (options_.auto_rejoin_groups) {
      // TODO: impl
    }
    co_await asio::co_spawn(io_service_.get_io_context(),
                            async_safe_invoke_connected(response),
                            asio::use_awaitable);
  }

  task async_safe_invoke_connected(ConnectedResponse response) {
    std::cout << "log connection connected\n";
    try {
      on_connected({response.getConnectionId(), response.getUserId(),
                    response.getReconnectionToken()});
    } catch (const std::exception &e) {
      std::cout << "log failed to invoke event\n";
    }

    co_return;
  }

  // TODO: impl
  void handle_disconnected_response(DisconnectedResponse response) { return; }

  // TODO: impl
  void handle_ack_response(AckResponse response) { return; }

  // TODO: impl
  void handle_group_data_response(GroupMessageResponseV2 response) { return; }

  // TODO: impl
  void handle_server_data_response(ServerMessageResponse response) { return; }

  task async_send_core(
      std::string payload,
      WebPubSubProtocolMessageType webpubsub_protocol_message_type) {

    auto as_text =
        webpubsub_protocol_message_type == WebPubSubProtocolMessageText;
    co_await client_->async_write(std::move(payload));
  }

private:
  std::shared_ptr<spdlog::logger> logger_;
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

#pragma region fields per connection id
  std::string client_access_uri_; // TODO: use a URI class?
  std::string connection_id_;
  std::string reconnection_token_;
  bool is_initial_connected_ = false;
  std::optional<DisconnectedResponse> latest_disconnected_response_;
// TODO: change

// TODO: add ack cache
/* std::unordered_map<
     uint16_t,
     std::unique_ptr<task_completion_source<request_result_or_exception>>>
     ack_cache_;*/
#pragma endregion

// TODO: add connection lock, start lock, stop lock
#pragma region locks
#pragma endregion

#pragma region fields per web socket
  std::unique_ptr<WebSocket> client_;
  WebSocketFactory web_socket_factory_;

#pragma endregion

  // TODO: reset them when connect/reconnect
  asio::cancellation_signal listen_loop_cancel_signal_;
  asio::cancellation_signal sequence_id_loop_cancel_signal_;
  detail::exclusion_lock stop_lock_;
  asio::steady_timer listen_loop_stop_notice_;
  asio::steady_timer sequence_id_ack_loop_stop_notice_;
};
} // namespace webpubsub
