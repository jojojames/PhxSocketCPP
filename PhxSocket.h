/**
 *   \file PhxSocket.h
 *   \brief A class to represent Phoenix Socket abstraction.
 *
 *  This class provides the Phoenix Socket abstraction sitting over Websockets.
 *
 */
#ifndef PhxSocketDelegate_H
#define PhxSocketDelegate_H

#include "PhxTypes.h"
#include "SocketDelegate.h"
#include "WebSocket.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class PhxSocketDelegate {
public:
    virtual void phxSocketDidOpen() = 0;
    virtual void phxSocketDidClose(const std::string& event) = 0;
    virtual void phxSocketDidReceiveError(const std::string& error) = 0;
};

#endif

// Forward Declares
class PhxChannel;
class WebSocket;

#ifndef PhxSocket_H
#define PhxSocket_H

#define RECONNECT_INTERVAL 5

class PhxSocket : public SocketDelegate {
private:
    /*! Delegate that can listen in on Phoenix related callbacks. */
    std::weak_ptr<PhxSocketDelegate> delegate;

    /*!
     * The underlying WebSocket interface. This can be used with a
     * different library provided the WebSocket interface is implemented.
     */
    std::shared_ptr<WebSocket> socket;

    /*!< Flag indicating whether or not to reconnect when socket errors out. */
    bool reconnectOnError;

    /*!< Websocket URL to connect to. */
    std::string url;

    /*!< The interval at which to send heartbeats to server. */
    int heartBeatInterval;

    /*!< The list of channels interested in sending messages over this socket.
     */
    std::vector<std::shared_ptr<PhxChannel>> channels;

    /*!< List of callbacks when socket opens. */
    std::vector<OnOpen> openCallbacks;

    /*!< List of callbacks when socket closes. */
    std::vector<OnClose> closeCallbacks;

    /*!< List of callbacks when socket errors out. */
    std::vector<OnError> errorCallbacks;

    /*!< List of callbacks when socket receives a messages. */
    std::vector<OnMessage> messageCallbacks;

    /*!< These params are used to pass arguments into the Websocket URL. */
    std::map<std::string, std::string> params;

    /*!< Ref to keep track of for each WebSocket message. */
    int ref = 0;

    /**
     *  \brief Stops the heartbeating.
     *
     *  \return void
     */
    void discardHeartBeatTimer();

    /*!< Mutex used when setting this->canSendHeartbeat. */
    std::mutex sendHeartbeatMutex;

    /*!< Flag indicating whether or not to continue sending heartbeats. */
    bool canSendHeartbeat;

    /**
     *  \brief Stops trying to reconnect the WebSocket.
     *
     *  \return void
     */
    void discardReconnectTimer();

    /*!< Mutex used when setting this->canReconnect. */
    std::mutex reconnectMutex;

    /*!< Flag indicating whether or not socket can reconnect to server. */
    bool canReconnect;

    /**
     *  \brief Disconnects the socket.
     *
     *  \return void
     */
    void disconnectSocket();

    /**
     *  \brief Function called when WebSocket opens.
     *
     *  \return void
     */
    void onConnOpen();

    /**
     *  \brief Function called when WebSocket closes.
     *
     *  \param event The event that caused the close.
     *  \return void
     */
    void onConnClose(const std::string& event);

    /**
     *  \brief Function called when there was an error with the connection.
     *
     *  \param error The error message.
     *  \return void
     */
    void onConnError(const std::string& error);

    /**
     *  \brief Function called when WebSocket receives a message.
     *
     *  \param rawMessage The message as a std::string.
     *  \return void
     */
    void onConnMessage(const std::string& rawMessage);

    /**
     *  \brief Triggers a "phx_error" event to all channels.
     *
     *  \param error The error message.
     *  \return void
     */
    void triggerChanError(const std::string& error);

    /**
     *  \brief Sends a heartbeat to keep Websocket connection alive.
     *
     *  \return void
     */
    void sendHeartbeat();

    /**
     *  \brief Sets this->canSendHeartbeat.
     *
     *  This is intended to be a semi-thread safe way to set this flag.
     *  The current thread must lock on this->sendHeartbeatMutex to set this
     * variable.
     *
     *  \param canSendHeartbeat Indicating whether or not this socket can
     *  continue sending heartbeats.
     *  \return void
     */
    void setCanSendHeartBeat(bool canSendHeartbeat);

    /**
     *  \brief Sets this->canReconnect.
     *
     *  This is intended to be a semi-thread safe way to set this flag.
     *  The current thread must lock on this->reconnectMutex to set this
     * variable.
     *
     *  \param canReconnect Indicating whether or not the socket can reconnect.
     *  \return void
     */
    void setCanReconnect(bool canReconnect);

    // SocketDelegate
    void webSocketDidOpen(WebSocket* socket);
    void webSocketDidReceive(WebSocket* socket, const std::string& message);
    void webSocketDidError(WebSocket* socket, const std::string& error);
    void webSocketDidClose(
        WebSocket* socket, int code, std::string reason, bool wasClean);
    // SocketDelegate

public:
    /**
     *  \brief Constructor
     *
     *  \param url The URL to connect to.
     *  \param interval The heartbeat interval.
     *  \return PhxSocket
     */
    PhxSocket(const std::string& url, int interval);

    /**
     *  \brief Constructor
     *
     *  \param url The URL to connect to.
     *  \return PhxSocket
     */
    PhxSocket(const std::string& url);

    /**
     *  \brief Constructor with custom WebSocket implementation.
     *
     *  \param url The URL to connect to.
     *  \param interval The heartbeat interval.
     *  \param socket the Custom WebSocket implementation.
     *  \return return type
     */
    PhxSocket(const std::string& url,
        int interval,
        std::shared_ptr<WebSocket> socket);

    /**
     *  \brief Connects the Websocket.
     *
     *  \return void
     */
    void connect();

    /**
     *  \brief Connects the Websocket.
     *
     *  \param params List of params to be formatted into Websocket URL.
     *  \return void
     */
    void connect(std::map<std::string, std::string> params);

    /**
     *  \brief Disconnects the socket connection.
     *
     *  \return void
     */
    void disconnect();

    /**
     *  \brief Reconnects the socket after disconnection.
     *
     *  The reconnection happens on a timer controlled by RECONNECT_INTERVAL.
     *
     *  \return void
     */
    void reconnect();

    /**
     *  \brief Adds a callback on open.
     *
     *  \param callback
     *  \return void
     */
    void onOpen(OnOpen callback);

    /**
     *  \brief Adds a callback on close.
     *
     *  \param callback
     *  \return void
     */
    void onClose(OnClose callback);

    /**
     *  \brief Adds a callback on error.
     *
     *  \param callback
     *  \return void
     */
    void onError(OnError callback);

    /**
     *  \brief Adds a callback on message.
     *
     *  \param callback
     *  \return void
     */
    void onMessage(OnMessage callback);

    /**
     *  \brief Flag indicating whether or not socket is connected.
     *
     *  \return bool Indicating connected status.
     */
    bool isConnected();

    /**
     *  \brief Make a unique reference per message sent to Phoenix Server.
     *
     *  \return int64_t
     */
    int64_t makeRef();

    /**
     *  \brief The current state of the socket connection.
     *
     *  \return SocketState
     */
    SocketState socketState();

    /**
     *  \brief Send data through websockets.
     *
     *  \param data The json data to send.
     *  \return void
     */
    void push(nlohmann::json data);

    /**
     *  \brief Adds PhxChannel to list of channels.
     *
     *  \return void
     */
    void addChannel(std::shared_ptr<PhxChannel> channel);

    /**
     *  \brief Removes PhxChannel from list of channels.
     *
     *  \return void
     */
    void removeChannel(std::shared_ptr<PhxChannel> channel);

    /**
     *  \brief Sets the PhxSocketDelegate.
     *
     *  this->delegate will be weakly held by PhxSocket.
     */
    void setDelegate(std::shared_ptr<PhxSocketDelegate> delegate);
};

#endif
