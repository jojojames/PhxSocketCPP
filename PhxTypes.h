// FIXME: Add Documentation.
#ifndef PhxTypes_H
#define PhxTypes_H
#include "json.hpp"
#include <functional>
#include <string>

enum class ChannelState { CLOSED, ERRORED, JOINING, JOINED };

using OnOpen = std::function<void()>;
using OnClose = std::function<void(const std::string& event)>;
using OnError = std::function<void(const std::string& error)>;
using OnMessage = std::function<void(nlohmann::json json)>;
using OnReceive = std::function<void(nlohmann::json message, int64_t ref)>;
using After = std::function<void()>;

#endif
