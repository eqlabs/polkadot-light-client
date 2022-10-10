#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/connection/stream.hpp>
#include <libp2p/host/host.hpp>
#include <libp2p/log/logger.hpp>

#include "network/grandpa/messages.h"
#include "utils/result.h"

namespace plc::core::runner {

class ClientRunner;

} // namespace plc::core::runner

namespace plc::core::network {

class MessageReadWriter;

} // namespace plc::core::network

namespace plc::core::network::grandpa {

class Observer {
public:
    virtual ~Observer() noexcept = default;

    virtual void onMessage(const libp2p::peer::PeerId &peer_id, const Message& message) = 0;
};

class Protocol final: public std::enable_shared_from_this<Protocol> {
    using Stream = libp2p::connection::Stream;
    using PeerId = libp2p::peer::PeerId;
    using PeerInfo = libp2p::peer::PeerInfo;

public:
    Protocol() = delete;
    Protocol(Protocol &&) noexcept = delete;
    Protocol(const Protocol &) = delete;
    Protocol &operator=(Protocol &&) noexcept = delete;
    Protocol &operator=(Protocol const &) = delete;

    Protocol(
        libp2p::Host& host,
        runner::ClientRunner& client_runner,
        std::shared_ptr<Observer> grandpa_observer);

    void start();
    void stop();

private:
    cppcoro::task<void> readHandshake(std::shared_ptr<Stream> stream);
    void read(std::shared_ptr<Stream> stream);
    cppcoro::task<void> readingTask(std::shared_ptr<Stream> stream);
    cppcoro::task<void> incomingStreamTask(std::shared_ptr<Stream> stream);

private:
    static const libp2p::peer::Protocol protocol;

    libp2p::Host& m_host;
    runner::ClientRunner& m_runner;
    std::shared_ptr<Observer> m_observer;
    bool m_is_running = false;

    libp2p::log::Logger m_log = libp2p::log::createLogger("Protocol", "grandpa_protocol");
};

}  // namespace plc::core::network
