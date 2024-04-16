//
// Created by alber on 2024/2/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/credentials/client_credential.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "webpubsub/client/detail/client/group_context.hpp"
#include "webpubsub/client/detail/client/retry_policy.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/common/utils.hpp"
#include "webpubsub/client/detail/concepts/retry_policy_c.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include "webpubsub/client/models/client_options.hpp"
#include <optional>

namespace webpubsub {
namespace detail {
// TODO: rename websocket_service
template <webpubsub_protocol_t protocol_t, typename websocket_factory_t,
          typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>

class client_lifetime_service {
  using strand_t = io::strand<io::io_context::executor_type>;

public:
  client_lifetime_service(strand_t &strand, const client_credential &credential,
                          websocket_factory_t &websocket_factory,
                          const client_options<protocol_t> &options,
                          const log &log)
      : log_(log), strand_(strand), options_(options), credential_(credential),
        websocket_factory_(websocket_factory), websocket_(nullptr), groups_() {
    if (!options.auto_reconnect) {
      return;
    }
  }

  const char *RECOVER_CONNECTION_ID_QUERY = "awps_connection_id";
  const char *RECOVER_RECONNECTION_TOKEN_QUERY = "awps_reconnection_token";

  // TODO: IMPL
  template <transition_context_c transition_context_t>
  auto async_connect(transition_context_t *context) -> async_t<> {
    reset_connection(context);
    client_access_uri_ = co_await credential_.async_get_client_access_uri();
    co_await async_establish_new_websocket(client_access_uri_, context);
  }

  // TODO: IMPL
  template <transition_context_c transition_context_t>
  auto
  async_establish_new_websocket(std::string uri,
                                transition_context_t *context) -> async_t<> {
    spdlog::trace("async_connect_websocket -- beg");
    websocket_ =
        websocket_factory_.create(std::move(uri), options_.protocol.get_name());
    // TODO: actually connect
    spdlog::trace("async_connect_websocket -- end");
    co_return;
  }

  auto
  async_read_message(std::string &frame,
                     webpubsub::websocket_close_status &status) -> async_t<> {
    spdlog::trace("lifetime.async_read_message");
    co_await websocket_->async_read(frame, status);
  }

  auto async_write_message(std::string frame) -> async_t<> {
    spdlog::trace("lifetime.async_write_message");
    co_await websocket_->async_write(frame);
  }

  // TODO: dev
  auto test() {}

  auto update_connection_info(std::string connection_id,
                              std::optional<std::string> reconnection_token)
      -> void {
    connection_id_ = std::move(connection_id);
    reconnection_token_ = std::move(reconnection_token);
  }

  auto make_retry_context() -> retry_context {
    auto o = options_.reconnect_retry_options;
    return retry_context{o.max_delay, o.max_retry, o.retry_mode, 0, o.delay};
  }
  auto auto_reconnect() -> const bool & { return options_.auto_reconnect; }
  auto connection_id() -> const std::string & { return connection_id_; }
  auto reconnection_token() -> const std::optional<std::string> & {
    return reconnection_token_;
  }
  auto groups() -> const std::unordered_map<std::string, group_context> & {
    return groups_;
  }
  auto client_access_uri() -> const std::string & { return client_access_uri_; }

private:
  // TODO: move to each on_leave_state
  template <transition_context_c transition_context_t>
  auto reset_connection(transition_context_t *context) {
    context->send().reset();
    context->receive().reset();
    connection_id_.clear();
    client_access_uri_.clear();
    reconnection_token_ = std::nullopt;
  }

  const log &log_;
  strand_t &strand_;
  websocket_factory_t &websocket_factory_;
  const client_credential &credential_;
  std::unique_ptr<websocket_t> websocket_;
  const client_options<protocol_t> &options_;
  std::string connection_id_;
  std::optional<std::string> reconnection_token_;
  std::string client_access_uri_;
  const std::unordered_map<std::string, group_context> groups_;

  //  TODO: add connection lock?
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
