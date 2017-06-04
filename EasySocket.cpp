#include "EasySocket.h"
#include "SocketDelegate.h"
#include "easylogging++.h"
#include <iostream>
#include <thread>

// Make sure to implement this constructor if you take out the
// Base class constructor call.
// Otherwise, it'll throw `symbol not found` exceptions when compiling.
EasySocket::EasySocket(const std::string& url, SocketDelegate* delegate)
    : WebSocket(url, delegate)
    , receiveQueue(1) {
    this->state = SocketClosed;
}

void EasySocket::open() {
    easywsclient::WebSocket::pointer socket
        = easywsclient::WebSocket::from_url(this->url);

    if (!socket) {
        this->state = SocketClosed;
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
                this->state = SocketClosed;
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
                this->state = SocketClosing;
                std::lock_guard<std::mutex> guard(this->socketMutex);
                ws->poll();
                ws->dispatch(callable);
                break;
            }
            case easywsclient::WebSocket::CONNECTING: {
                this->state = SocketConnecting;
                std::lock_guard<std::mutex> guard(this->socketMutex);
                ws->poll();
                ws->dispatch(callable);
                break;
            }
            case easywsclient::WebSocket::OPEN: {
                this->state = SocketOpen;
                if (!triggeredWebsocketJoinedCallback) {
                    triggeredWebsocketJoinedCallback = true;
                    this->getSocketState();
                    SocketDelegate* d = this->delegate;
                    if (d) {
                        d->webSocketDidOpen(this);
                    }
                }

                std::lock_guard<std::mutex> guard(this->socketMutex);
                ws->poll();
                ws->dispatch(callable);
                break;
            }
            default: { break; }
            }
        }

        this->state = SocketClosed;
        this->socket = nullptr;
    });

    worker.detach();
}

void EasySocket::close() {
    this->state = SocketClosed;
    // Was already closed or never opened.
    if (!this->socket) {
        return;
    }

    this->socket->close();
}

void EasySocket::send(const std::string& message) {
    std::thread thread([this, message]() {
        std::lock_guard<std::mutex> guard(this->socketMutex);
        // Grab a copy of the pointer in case it gets NULLed out.
        easywsclient::WebSocket::pointer sock = this->socket;
        if (sock && this->state == SocketOpen) {
            sock->send(message);
        }
    });
    thread.detach();
}

void EasySocket::handleMessage(const std::string& message) {
    LOG(INFO) << message + "\n";
    this->receiveQueue.enqueue([this, message]() {
        SocketDelegate* d = this->delegate;
        if (d) {
            d->webSocketDidReceive(this, message);
        }
    });
}

SocketState EasySocket::getSocketState() {
    // easywsclient's State code is seemingly unreliable.
    // So we manage it ourselves.
    return this->state;
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
