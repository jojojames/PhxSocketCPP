#include "PhxChannel.h"
#include "PhxPush.h"
#include "PhxSocket.h"

PhxChannel::PhxChannel(std::shared_ptr<PhxSocket> socket,
    std::string topic,
    std::map<std::string, std::string> params) {
    this->state = ChannelClosed;
    this->topic = topic;
    this->params = params;
    this->socket = socket;
    this->joinedOnce = false;
}

void PhxChannel::bootstrap() {
    // NOTE: This can't be done in the constructor.
    // So we bootstrap the connection here.
    this->socket->addChannel(this->shared_from_this());

    this->socket->onOpen([this]() { this->rejoin(); });

    this->socket->onClose([this](const std::string& event) {
        this->state = ChannelClosed;
        this->socket->removeChannel(this->shared_from_this());
    });

    this->socket->onError(
        [this](const std::string& error) { this->state = ChannelErrored; });

    std::shared_ptr<PhxPush> n = std::make_shared<PhxPush>(
        this->shared_from_this(), "phx_join", this->params);
    this->joinPush = std::move(n);

    this->joinPush->onReceive(
        "ok", [this](nlohmann::json message) { this->state = ChannelJoined; });

    this->onEvent("phx_reply", [this](nlohmann::json message, int64_t ref) {
        this->triggerEvent(this->replyEventName(ref), message, ref);
    });
}

std::shared_ptr<PhxPush> PhxChannel::join() {
    if (this->joinedOnce) {
        // ERROR
    } else {
        this->joinedOnce = true;
    }

    this->sendJoin();
    return this->joinPush;
}

void PhxChannel::rejoin() {
    if (this->joinedOnce && this->state != ChannelJoining
        && this->state != ChannelJoined) {
        this->sendJoin();
    }
}

void PhxChannel::sendJoin() {
    this->state = ChannelJoining;
    this->joinPush->setPayload(this->params);
    this->joinPush->send();
}

void PhxChannel::leave() {
    this->state = ChannelClosed;
    std::map<std::string, std::string> payload;
    this->pushEvent("phx_leave", payload)
        ->onReceive("ok", [this](nlohmann::json message) {
            this->triggerEvent("phx_close", "leave", -1);
        });
}

void PhxChannel::onClose(OnClose callback) {
    this->onEvent(
        "phx_close", [this, callback](nlohmann::json message, int64_t ref) {
            callback(message);
        });
}

void PhxChannel::onError(OnError callback) {
    this->onEvent("phx_error",
        [callback](nlohmann::json error, int64_t ref) { callback(error); });
}

void PhxChannel::onEvent(const std::string& event, OnReceive callback) {
    this->bindings.emplace_back(event, callback);
}

void PhxChannel::offEvent(const std::string& event) {
    // Remove all Event bindings that match event.
    std::vector<std::tuple<std::string, OnReceive>> v = this->bindings;
    v.erase(std::remove_if(v.begin(),
                v.end(),
                [this, event](std::tuple<std::string, OnReceive> x) {
                    return std::get<0>(x) == event;
                }),
        v.end());
}

bool PhxChannel::isMemberOfTopic(const std::string& topic) {
    return this->topic == topic;
}

void PhxChannel::triggerEvent(
    const std::string& event, nlohmann::json message, int64_t ref) {
    // Trigger OnReceive callbacks that match event.
    std::vector<std::tuple<std::string, OnReceive>> v = this->bindings;
    for (std::tuple<std::string, OnReceive>& it : v) {
        if (std::get<0>(it) == event) {
            std::get<1>(it)(message, ref);
        }
    }
}

std::shared_ptr<PhxPush> PhxChannel::pushEvent(
    const std::string& event, std::map<std::string, std::string> payload) {
    std::shared_ptr<PhxPush> p
        = std::make_shared<PhxPush>(this->shared_from_this(), event, payload);
    p->send();
    return p;
}

std::shared_ptr<PhxSocket> PhxChannel::getSocket() {
    return this->socket;
}

std::string PhxChannel::replyEventName(int64_t ref) {
    std::string text = "chan_reply_";
    text += std::to_string(ref);
    return text;
}

std::string PhxChannel::getTopic() {
    return this->topic;
}
