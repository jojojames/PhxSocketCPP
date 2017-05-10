#include "PhxPush.h"
#include "PhxChannel.h"
#include "PhxSocket.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>

PhxPush::PhxPush(std::shared_ptr<PhxChannel> channel,
    std::string event,
    std::map<std::string, std::string> payload) {
    this->channel = channel;
    this->event = event;
    this->payload = payload;

    this->receivedResp = nullptr;
    this->afterHook = nullptr;
    this->sent = false;
}

void PhxPush::send() {
    int64_t ref = this->channel->getSocket()->makeRef();
    this->refEvent = this->channel->replyEventName(ref);
    this->receivedResp = nullptr;
    this->sent = false;

    // FIXME: Should this be weak?
    this->channel->onEvent(
        this->refEvent, [this](nlohmann::json message, int64_t ref) {
            this->receivedResp = message;
            this->matchReceive(message);
            this->cancelRefEvent();
            this->cancelAfter();
        });

    this->startAfter();
    this->sent = true;

    // clang-format off
    this->channel->getSocket()->push(
        { { "topic", this->channel->getTopic() },
          { "event", this->event },
          { "payload", this->payload },
          { "ref", ref }
        });
    // clang-format on
}

std::shared_ptr<PhxPush> PhxPush::onReceive(
    const std::string& status, OnMessage callback) {
    // receivedResp could actually be a std::string.
    if (this->receivedResp.is_object()
        && this->receivedResp["status"] == status) {
        callback(this->receivedResp);
    }

    this->recHooks.emplace_back(status, callback);
    return this->shared_from_this();
}

std::shared_ptr<PhxPush> PhxPush::after(int ms, After callback) {
    if (this->afterHook) {
        // ERROR
    }

    this->afterInterval = ms;
    this->afterHook = callback;
    return this->shared_from_this();
}

void PhxPush::cancelRefEvent() {
    this->channel->offEvent(this->refEvent);
}

void PhxPush::cancelAfter() {
    if (!this->afterHook) {
        return;
    }

    std::thread thread([this]() {
        std::lock_guard<std::mutex> guard(this->afterTimerMutex);
        this->shouldContinueAfterCallback = false;
    });

    thread.detach();
}

void PhxPush::startAfter() {
    if (!this->afterHook) {
        return;
    }

    // FIXME: Should this be weak?
    int interval = this->afterInterval;
    std::thread thread([this, interval]() {
        // Use sleep_for to wait specified time (or sleep_until).
        this->shouldContinueAfterCallback = true;
        std::this_thread::sleep_for(std::chrono::seconds{ interval });
        std::lock_guard<std::mutex> guard(this->afterTimerMutex);
        if (this->shouldContinueAfterCallback) {
            this->cancelRefEvent();
            this->afterHook();
            this->shouldContinueAfterCallback = false;
        }
    });

    thread.detach();
}

void PhxPush::matchReceive(nlohmann::json payload) {
    for (int i = 0; i < this->recHooks.size(); i++) {
        std::tuple<std::string, OnMessage> tuple = this->recHooks.at(i);
        if (std::get<0>(tuple) == payload["status"]) {
            std::get<1>(tuple)(payload["response"]);
        }
    }
}

void PhxPush::setPayload(std::map<std::string, std::string> payload) {
    this->payload = payload;
}
