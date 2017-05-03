/**
 *   \file PhxChannel.h
 *   \brief A class to represent Phoenix Channel abstraction.
 *
 *  Detailed description
 *
 */
#ifndef PhxChannel_H
#define PhxChannel_H

#include "PhxTypes.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class PhxSocket;
class PhxChannel;
class PhxPush;

class PhxChannelDelegate {
public:
    virtual void phxChannelClosed() = 0;
    virtual void phxChannelDidReceiveError(void* error) = 0;
};

class PhxChannel : public std::enable_shared_from_this<PhxChannel> {
private:
    /*!<
     * bindings contains a list of tuples where Item 1 is the Event
     * and Item 2 is the callback.
     */
    std::vector<std::tuple<std::string, OnReceive>> bindings;

    /*!< A flag indicating whether there has been an attempt to join channel. */
    bool joinedOnce;

    /*!< The PhxPush object that is responsible for joining a channel. */
    std::shared_ptr<PhxPush> joinPush;

    /*!< Unused, supposed to be used for PhxChannelDelegate callbacks. */
    PhxChannelDelegate* delegate;

    /*!< The socket connection to send and receive data over. */
    std::shared_ptr<PhxSocket> socket;

    /*!< The current state of the channel. */
    ChannelState state;

    /*!< The topic of this channel. */
    std::string topic;

    /*! Params that will be sent up as a payload to Phoenix Channel. */
    std::map<std::string, std::string> params;

    /**
     *  \brief Trigger joining of channel.
     *
     *  \return void
     */
    void sendJoin();

    /**
     *  \brief Wrap sendJoin.
     *
     *  \return void
     */
    void rejoin();

    /**
     *  \brief Determines if Channel is part of topic.
     *
     *  \param topic The topic to check against.
     *  \return bool Indicating if member of topic.
     */
    bool isMemberOfTopic(const std::string& topic);

public:
    /**
     *  \brief Trigger callbacks that match event.
     *
     *  \param event The event to trigger callbacks for.
     *  \param message The message to forward to callback.
     *  \param ref The ref of the message.
     *  \return void
     */
    void triggerEvent(
        const std::string& event, nlohmann::json message, int64_t ref);

    /**
     *  \brief Getter for socket.
     *
     *  \return std::shared_ptr<PhxSocket>
     */
    std::shared_ptr<PhxSocket> getSocket();

    /**
     *  \brief Creates a event named for a reply using ref.
     *
     *  \param ref Phoenix ref.
     *  \return std::string
     */
    std::string replyEventName(int64_t ref);

    /**
     *  \brief Constructor
     *
     *  \param socket The socket connection PhxChannel sends messages over.
     *  \param topic The topic this Channel sends and receives messages for.
     *  \param params Params to send up to channel.
     *  \return PhxChannel
     */
    PhxChannel(std::shared_ptr<PhxSocket> socket, std::string topic,
        std::map<std::string, std::string> params);

    /**
     *  \brief Called to bootstrap PhxChannel and PhxSocket together.
     *
     *  This MUST be called for PhxChannel communication to work.
     *
     *  The reason for the existance of this function is because of the need
     *  to set up shared_ptrs between PhxChannel and PhxSocket.
     *  This initialization code can't be in the constructor because we're
     *  trying to use this->shared_from_this() which can only be used after a
     *  shared_ptr is already created which won't work if we're calling
     *  this->shared_from_this from the constructor.
     *
     *  \return void
     */
    void bootstrap();

    /**
     *  \brief Sends a join message to Phoenix Channel.
     *
     *  \return std::shared_ptr<PhxPush>
     */
    std::shared_ptr<PhxPush> join();

    /**
     *  \brief Closes the Phoenix Channel connection.
     *
     *  \return void
     */
    void leave();

    /**
     *  \brief Adds event and callback to this->bindings.
     *
     *  Adding an event to bindings causes its callback to get triggered
     *  when its corresponding event is posted.
     *
     *  \param event The event to listen to.
     *  \param callback The callback to trigger if event is posted.
     *  \return void
     */
    void onEvent(const std::string& event, OnReceive callback);

    /**
     *  \brief Removes event from this->bindings.
     *
     *  Removing event from bindings skips any callback associated with that
     *  event from triggering.
     *
     *  \param event The event to unsubscribe.
     *  \return void
     */
    void offEvent(const std::string& event);

    /**
     *  \brief Adds a callback that will get triggered on close.
     *
     *  \param callback The callback triggered on close.
     *  \return void
     */
    void onClose(OnClose callback);

    /**
     *  \brief Adds a callback that will get triggered on error.
     *
     *  \param callback The callback triggered on error.
     *  \return void
     */
    void onError(OnError callback);

    /**
     *  \brief Pushes an event over Websockets.
     *
     *  \param event The event to push to server.
     *  \param payload Payload to push to server.
     *  \return std::shared_ptr<PhxPush>
     */
    std::shared_ptr<PhxPush> pushEvent(
        const std::string& event, std::map<std::string, std::string> payload);

    /**
     *  \brief Gets the topic of the channel.
     *
     *  \return std::string topic
     */
    std::string getTopic();
};

#endif
