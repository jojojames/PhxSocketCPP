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
#include "ThreadPool.h"
#include "WebSocket.h"
#include "easywsclient.hpp"
#include <string>

class EasySocket : public WebSocket {
private:
    /*!< Queue used for synchronization when sending messages. */
    ThreadPool sendingQueue;

    /*!< Queue used for synchronization when receiving messages. */
    ThreadPool receiveQueue;

    /*!< The underlying socket EasySocket wraps. */
    easywsclient::WebSocket::pointer socket;

    /*!< Keep track of Socket State.
      This is used instead of easywsclient's SocketState. */
    SocketState state;

    /**
     *  \brief Function used to trigger WebSocket::webSocketDidReceive.
     *
     *  \param message received.
     *  \return void
     */
    void handleMessage(const std::string& message);
public:
    // Make sure to implement this constructor if you take out the
    // Base class constructor call.
    // Otherwise, it'll throw `symbol not found` exceptions when compiling.
    EasySocket(const std::string& url, SocketDelegate* delegate);

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
