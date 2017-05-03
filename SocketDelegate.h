/**
 *   \file SocketDelegate.h
 *   \brief Delegate interface classes to receive Websocket callbacks.
 *
 *  Detailed description
 *
 */

#ifndef SocketDelegate_H
#define SocketDelegate_H
#include <string>

class WebSocket;

class SocketDelegate {
public:
    /**
     *  \brief Callback received when Websocket is connected.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void webSocketDidOpen(WebSocket* socket) = 0;

    /**
     *  \brief Callback received when Websocket receives a message.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void webSocketDidReceive(
        WebSocket* socket, const std::string& message)
        = 0;

    /**
     *  \brief Callback received when Websocket has an error.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void webSocketDidError(WebSocket* socket, const std::string& error)
        = 0;

    /**
     *  \brief Callback received when Websocket closes.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void webSocketDidClose(
        WebSocket* socket, int code, std::string reason, bool wasClean)
        = 0;
};

#endif
