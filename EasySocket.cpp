#include "EasySocket.h"
#include "SocketDelegate.h"
#include "easylogging++.h"
#include <iostream>
#include <thread>

// Make sure to implement this constructor if you take out the
// Base class constructor call.
// Otherwise, it'll throw `symbol not found` exceptions when compiling.
EasySocket::EasySocket(const std::string& url, SocketDelegate* delegate)
    : WebSocket(url, delegate) {
}

void EasySocket::open() {
    easywsclient::WebSocket::pointer socket
        = easywsclient::WebSocket::from_url(this->url);

    if (!socket) {
        std::thread errorThread([this]() {
            SocketDelegate* d = this->delegate;
            if (d) {
                d->webSocketDidError(this, "");
            }
        });
        errorThread.detach();

        this->socket = nullptr;
        return;
    }

    this->socket = socket;

    std::thread worker([this]() {
        easywsclient::WebSocket::pointer ws = this->socket;
        // This worker thread will continue to loop as long as the Websocket
        // is connected. Once we get a CLOSED message, this will be set to
        // false and the loop (and thread) will be exited.
        bool shouldContinueLoop = true;

        // We use this flag to track if we've triggered the webSocketDidOpen
        // yet. The first time we encounter CONNECTED while looping, trigger
        // the callback and then set this to true so we only do it once.
        bool triggeredWebsocketJoinedCallback = false;

        using std::placeholders::_1;
        std::function<void(std::string)> callable
            = std::bind(&EasySocket::handleMessage, this, _1);

        while (shouldContinueLoop) {
            switch (ws->getReadyState()) {
            case easywsclient::WebSocket::CLOSED: {
                std::thread closeThread([this]() {
                    SocketDelegate* d = this->delegate;
                    if (d) {
                        d->webSocketDidClose(this, 0, "", true);
                    }
                });
                closeThread.detach();

                // We got a CLOSED so the loop should stop.
                shouldContinueLoop = false;
                break;
            }
            case easywsclient::WebSocket::CLOSING: {
                ws->poll();
                ws->dispatch(callable);
                break;
            }
            case easywsclient::WebSocket::CONNECTING: {
                ws->poll();
                ws->dispatch(callable);
                break;
            }
            case easywsclient::WebSocket::OPEN: {
                if (!triggeredWebsocketJoinedCallback) {
                    triggeredWebsocketJoinedCallback = true;
                    this->getSocketState();
                    SocketDelegate* d = this->delegate;
                    if (d) {
                        d->webSocketDidOpen(this);
                    }
                }

                ws->poll();
                ws->dispatch(callable);
                break;
            }
            default: { break; }
            }
        }

        this->socket = nullptr;
    });

    worker.detach();
}

void EasySocket::close() {
    // Was already closed or never opened.
    if (!this->socket) {
        return;
    }

    this->socket->close();
}

void EasySocket::send(const std::string& message) {
    // Opening a new thread per message might by a little heavy.
    std::thread thread([this, message]() {
        std::lock_guard<std::mutex> guard(this->sendMutex);
        // Grab a copy of the pointer in case it gets NULLed out.
        easywsclient::WebSocket::pointer sock = this->socket;
        if (sock) {
            sock->send(message);
        }
    });
    thread.detach();
}

void EasySocket::handleMessage(const std::string& message) {
    LOG(INFO) << "EasySocket::handleMessage: " << message << std::endl;
    std::thread thread([this, message]() {
        std::lock_guard<std::mutex> guard(this->receiveMutex);
        SocketDelegate* d = this->delegate;
        if (d) {
            d->webSocketDidReceive(this, message);
        }
    });
    thread.detach();
}

SocketState EasySocket::getSocketState() {
    if (!this->socket) {
        return SocketClosed;
    }

    switch (this->socket->getReadyState()) {
    case easywsclient::WebSocket::CLOSING: {
        return SocketClosing;
    }
    case easywsclient::WebSocket::CLOSED: {
        return SocketClosed;
    }
    case easywsclient::WebSocket::CONNECTING: {
        return SocketConnecting;
    }
    case easywsclient::WebSocket::OPEN: {
        return SocketOpen;
    }
    }
}

void EasySocket::setDelegate(SocketDelegate* delegate) {
    this->delegate = delegate;
}

SocketDelegate* EasySocket::getDelegate() {
    return this->delegate;
}

void EasySocket::setURL(const std::string& url) {
    this->url = url;
}
