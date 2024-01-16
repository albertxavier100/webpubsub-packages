#pragma once
#include <string>
//
// #define WEBPUBSUB_CUSTOM_NLOHMANN_JSON_TO(v1) \
//  if (nlohmann_json_t.v1 == ) \
//    nlohmann_json_j[#v1] = nlohmann_json_t.v1;

// #define WEBPUBSUB_CUSTOM_NLOHMANN_DEFINE_TYPE_INTRUSIVE_ONLY_SERIALIZE(Type,   \
//                                                                        ...)    \
//   friend void to_json(nlohmann::json &nlohmann_json_j,                         \
//                       const Type &nlohmann_json_t) {                           \
//     NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))   \
//   }

namespace WebPubSub {
static std::string EMPTY_STRING = std::string("");
} // namespace WebPubSub