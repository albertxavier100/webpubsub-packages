//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_DATA_HANDLE_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_DATA_HANDLE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
template <webpubsub_protocol_t protocol_t> class client_data_handle_service {
public:
  template <typename data_t>
  using data_channel_t =
      io::experimental::channel<void(io::error_code, data_t)>;

  client_data_handle_service(strand_t &strand,
                             const client_options<protocol_t> &options,
                             const log &log)
      : group_data_loop_svc_("GROUP DATA LOOP", strand, log),
        server_data_loop_svc_("SERVER DATA LOOP", strand, log),
        options_(options),
        server_data_channel_(strand, options.max_buffer_size),
        group_data_channel_(strand, options.max_buffer_size) {}

  template <transition_context_c transition_context_t>
  auto spawn_data_loops_coro(transition_context_t *context) {
     server_data_loop_svc_.spawn_loop_coro(
         async_start_server_data_response_handler(context));
     group_data_loop_svc_.spawn_loop_coro(
         async_start_group_data_response_handler(context));
  }

  auto async_cancel_data_loops_coro() -> async_t<> {
    // TODO: wait for all channel data handled
     co_await server_data_loop_svc_.async_cancel_loop_coro();
     co_await group_data_loop_svc_.async_cancel_loop_coro();
  }

  auto reset() -> void {
     server_data_loop_svc_.reset();
     group_data_loop_svc_.reset();
  }

  auto async_queue_group_data(GroupMessageResponseV2 res) -> async_t<> {
     co_await group_data_channel_.async_send(io::error_code{}, std::move(res),
                                             io::use_awaitable);
  }

  auto async_queue_server_data(ServerMessageResponse res) -> async_t<> {
     co_await server_data_channel_.async_send(io::error_code{},
     std::move(res),
                                              io::use_awaitable);
  }

private:
  template <transition_context_c transition_context_t>
  auto async_start_group_data_response_handler(transition_context_t *context)
      -> async_t<> {
     for (;;) {
       auto cs = co_await io::this_coro::cancellation_state;
       if (cs.cancelled() != io::cancellation_type::none) {
         spdlog::trace("async_start_group_data_response_handler... break");
         break;
       }
       try {
         auto res =
             co_await group_data_channel_.async_receive(io::use_awaitable);
         context->safe_invoke_callback(group_data_context{res});
       } catch (const std::exception &ex) {
         spdlog::trace("failed to receive group data from channel. {0}",
                       ex.what());
       }
     }
  }

  template <transition_context_c transition_context_t>
  auto async_start_server_data_response_handler(transition_context_t *context)
      -> async_t<> {
     for (;;) {
       auto cs = co_await io::this_coro::cancellation_state;
       if (cs.cancelled() != io::cancellation_type::none) {
         spdlog::trace("async_start_server_data_response_handler... break");
         break;
       }
       try {
         auto res =
             co_await server_data_channel_.async_receive(io::use_awaitable);
         context->safe_invoke_callback(server_data_context{res});
       } catch (const std::exception &ex) {
         spdlog::trace("failed to receive group data from channel. {0}",
                       ex.what());
       }
     }
  }

  const client_options<protocol_t> &options_;
  client_loop_service group_data_loop_svc_;
  client_loop_service server_data_loop_svc_;
   data_channel_t<ServerMessageResponse> server_data_channel_;
   data_channel_t<GroupMessageResponseV2> group_data_channel_;
};
} // namespace detail
} // namespace webpubsub
#endif