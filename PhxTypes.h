// FIXME: Add Documentation.
#ifndef PhxTypes_H
#define PhxTypes_H
#include "json.hpp"
#include <functional>
#include <string>

typedef enum {
    ChannelClosed,
    ChannelErrored,
    ChannelJoining,
    ChannelJoined
} ChannelState;

typedef std::function<void()> OnOpen;
typedef std::function<void(const std::string& event)> OnClose;
typedef std::function<void(const std::string& error)> OnError;
typedef std::function<void(nlohmann::json json)> OnMessage;
typedef std::function<void(nlohmann::json message, int64_t ref)> OnReceive;
typedef std::function<void()> After;

#endif
