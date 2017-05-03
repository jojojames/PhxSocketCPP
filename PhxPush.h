/**
 *   \file PhxPush.h
 *   \brief A class to represent a singular message/event pushed to a Phoenix
 * Channel.
 *
 *  Detailed description
 *
 */

#ifndef PhxPush_H
#define PhxPush_H
#include "PhxTypes.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class PhxChannel;

class PhxPush : public std::enable_shared_from_this<PhxPush> {
private:
    /*!< The Phoenix Channel messages are pushed to. */
    std::shared_ptr<PhxChannel> channel;

    /*!< The event name the server listens on. */
    std::string event;

    /*!< Name of event for the message. */
    std::string refEvent;

    /*!< Holds the payload that will be sent to the server. */
    std::map<std::string, std::string> payload;

    /*!< The callback to trigger if event is not returned from server. */
    After afterHook;

    /*!< The interval to wait before triggering afterHook. */
    int afterInterval;

    /*!<
     * recHooks contains a list of tuples where Item 1 is the Status
     * and Item 2 is the callback.
     */
    std::vector<std::tuple<std::string, OnMessage>> recHooks;

    /*!< The response from server if server responded to sent message. */
    nlohmann::json receivedResp;

    /*!< Flag determining whether or not the message was sent through Sockets.
     */
    bool sent;

    /*!< Mutex used when setting this->shouldContinueAfterCallback. */
    std::mutex afterTimerMutex;

    /*!< Flag that determines if After callback will be triggered. */
    bool shouldContinueAfterCallback;

    /**
     *  \brief Stops listening for this event.
     *
     *  \return void
     */
    void cancelRefEvent();

    /**
     *  \brief Cancels After callback from possibly triggering.
     *
     *  \return void
     */
    void cancelAfter();

    /**
     *  \brief Starts the timer until After Callback is triggered.
     *
     *  \return void
     */
    void startAfter();

    /**
     *  \brief Central function that kicks off OnMessage callbacks.
     *
     *  \param payload Payload to match against.
     *  \return void
     */
    void matchReceive(nlohmann::json payload);

public:
    /**
     *  \brief Sets the payload that this class will push out through
     * Websockets.
     *
     *  \param payload
     *  \return void
     */
    void setPayload(std::map<std::string, std::string> payload);

    /**
     *  \brief Constructor
     *
     *  \param channel The Phoenix Channel to send to.
     *  \param event The Phoenix Event to post to.
     *  \param payload The Payload to send.
     *  \return PhxPush
     */
    PhxPush(std::shared_ptr<PhxChannel> channel, std::string event,
        std::map<std::string, std::string> payload);

    /**
     *  \brief Sends Phoenix Formatted message with payload through Websockets.
     *
     *  \return void
     */
    void send();

    /**
     *  \brief Adds a callback to be triggered for status.
     *
     *  Adds a callback to be triggered when message matching status is posted.
     *
     *  \param status The status that callback should respond to.
     *  \param callback The callback triggered when status message is posted.
     *  \return std::shared_ptr<PhxPush>
     */
    std::shared_ptr<PhxPush> onReceive(
        const std::string& status, OnMessage callback);

    /**
     *  \brief Adds a callback to be triggered if event doesn't come back.
     *
     *  Adds a callback to be triggered after ms if event is not `replied back`
     * to.
     *  If PhxPush receives a message with matching event, callback will not
     *  be called.
     *
     *  \param ms Milliseconds to wait before triggering callback.
     *  \param callback Callback to be triggered after ms has passed.
     *  \return std::shared_ptr<PhxPush>
     */
    std::shared_ptr<PhxPush> after(int ms, After callback);
};

#endif // PhxPush_H
