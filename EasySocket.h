/**
 *   \file EasySocket.h
 *   \brief A WebSocket implementation that wraps easywsclient.
 *
 *  easywsclient is relatively spartan, not passing callbacks when expected.
 *  It is wrapped to fake those callbacks.
 *
 */
#ifndef EasySocket_H
#define EasySocket_H

#include "SocketDelegate.h"
#include "WebSocket.h"
#include "easywsclient.hpp"
#include <mutex>
#include <string>

class EasySocket : public WebSocket {
private:
    /*!< The underlying socket EasySocket wraps. */
    easywsclient::WebSocket::pointer socket;

    /*!< The mutex used when sending messages over the socket. */
    std::mutex sendMutex;

    /*!< The mutex used when receiving messages from the socket. */
    std::mutex receiveMutex;

    /*!< Function used to trigger WebSocket::webSocketDidReceive. */
    void handleMessage(const std::string& message);

public:
    // Make sure to implement this constructor if you take out the
    // Base class constructor call.
    // Otherwise, it'll throw `symbol not found` exceptions when compiling.
    EasySocket(const std::string& url, SocketDelegate* delegate)
        : WebSocket(url, delegate) {
    }

    // WebSocket
    void open();
    void close();
    void send(const std::string& message);
    SocketState getSocketState();
    void setDelegate(SocketDelegate* delegate);
    SocketDelegate* getDelegate();
    void setURL(const std::string& url);
    // WebSocket
};

#endif
