/**
 *   \file WebSocket.h
 *   \brief Websocket interface PhxSocket uses.
 *
 *  There's a couple different C++ Websocket libraries that can be used.
 *  In order to not get tied at the hips to any one of them, make it so any
 *  implementing users can pick the one they want to use.
 */
#ifndef WebSocket_H
#define WebSocket_H
#include <string>

class SocketDelegate;

typedef enum {
    SocketConnecting,
    SocketOpen,
    SocketClosing,
    SocketClosed
} SocketState;

class WebSocket {
protected:
    std::string url;
    SocketDelegate* delegate;

public:
    WebSocket() {
    }

    /**
     *  \brief Constructor.
     *
     *  This constructor will require url and delegate to be set.
     *
     *  \param url The url to connect to.
     *  \param delegate The delegate to receive WebSocket callbacks.
     *  \return Websocket
     */
    WebSocket(const std::string& url, SocketDelegate* delegate) {
        this->url = url;
        this->delegate = delegate;
    }

    /**
     *  \brief Open the websocket connection.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void open() = 0;

    /**
     *  \brief Close the websocket connection.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void close() = 0;

    /**
     *  \brief Send a message over websockets.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void send(const std::string& message) = 0;

    // // Send a Data
    // - (void)sendData:(nullable NSData *)data error:(NSError **)error;

    /**
     *  \brief Get SocketState
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual SocketState getSocketState() = 0;

    /**
     *  \brief Set the SocketDelegate.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual void setDelegate(SocketDelegate* delegate) = 0;

    /**
     *  \brief Get the SocketDelegate.
     *
     *  Detailed description
     *
     *  \param param
     *  \return return type
     */
    virtual SocketDelegate* getDelegate() = 0;

    /**
     *  \brief Sets the WebSocket URL.
     *
     *  \param url The url to connect to.
     *  \return void
     */
    virtual void setURL(const std::string& url) = 0;
};

#endif
