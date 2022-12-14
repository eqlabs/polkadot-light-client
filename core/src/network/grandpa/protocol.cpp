#include "protocol.h"

#include <libp2p/connection/loopback_stream.hpp>

#include "network/common/errors.h"
#include "network/common/roles.h"
#include "network/common/format_peer_id.h"
#include "network/scale/message_reader_writer.h"
#include "runner/client_runner.h"
#include "utils/callback_to_coro.h"

namespace plc::core::network::grandpa {

// TODO: can we use protocol prefix here, i.e. "/{}/grandpa/1" ?
const libp2p::peer::Protocol Protocol::protocol = "/paritytech/grandpa/1";

Protocol::Protocol(
    libp2p::Host& host,
    runner::ClientRunner& client_runner,
    std::shared_ptr<Observer> observer)
    : m_host{host}
    , m_runner{client_runner}
    , m_observer{std::move(observer)} {
}

void Protocol::start() {
    if (m_is_running) {
        m_log->warn("Grandpa protocol is already in a running state.");
        return;
    }

    m_host.setProtocolHandler(protocol, [wp = weak_from_this()](auto &&stream) {
        if (auto self = wp.lock()) {
            if (auto peer_id = stream->remotePeerId()) {
                self->m_log->trace("Handled {} protocol stream from: {}",
                    protocol,
                    peer_id.value());
                self->m_runner.dispatchTask(self->incomingStreamTask(std::forward<decltype(stream)>(stream)));
            } else {
                self->m_log->warn("Handled {} protocol stream from unknown peer",
                    protocol);
            }
        }
    });

    auto stream = std::make_shared<libp2p::connection::LoopbackStream>(m_host.getPeerInfo(), m_runner.getService());
    m_runner.dispatchTask(readingTask(std::move(stream)));
}

void Protocol::stop() {
    if (m_is_running) {
        m_is_running = false;
        m_log->trace("Stopping protocol");
    }
}

cppcoro::task<void> Protocol::readingTask(std::shared_ptr<Stream> stream) {
    auto read_writer = std::make_shared<scale::MessageReadWriter>(std::move(stream));
    auto wp = weak_from_this();
    while (true) {
        auto grandpa_message_res = co_await read_writer->read<Message>();
        auto self = wp.lock();
        if (!self) {
            stream->reset();
            co_return;
        }

        if (!self->m_is_running) {
            self->m_log->trace("Reading task stopped");
            co_return;
        }

        if (!grandpa_message_res) {
            self->m_log->warn("Can't read message from {}: {}",
                stream->remotePeerId().value(),
                grandpa_message_res.error().message());
            stream->reset();
            co_return;
        }

        const auto peer_id = stream->remotePeerId().value();
        self->m_log->debug("Message has received from {}", peer_id);
        self->m_observer->onMessage(peer_id, grandpa_message_res.value());
    }
}

cppcoro::task<> Protocol::incomingStreamTask(std::shared_ptr<Stream> stream) {
    assert(stream->remotePeerId());
    auto read_writer = std::make_shared<scale::MessageReadWriter>(stream);
    const auto logger = m_log;
    const auto handshake_read_res = co_await read_writer->read<Roles>();
    if (!handshake_read_res) {
        logger->trace("Can't read handshake from {}: {}",
            stream->remotePeerId().value(),
            handshake_read_res.error().message());
        stream->reset();
        co_return;
    } else {
        logger->trace("Handshake received from {}", stream->remotePeerId().value());
    }

    Roles roles;
    roles.flags.light = 1;
    const auto handshake_send_res = co_await read_writer->write(roles);
    if (!handshake_send_res) {
        logger->trace("Can't send handshake to {}: {}",
            stream->remotePeerId().value(),
            handshake_send_res.error().message());
        stream->reset();
        co_return;
    } else {
        logger->trace("Handshake send to {}", stream->remotePeerId().value());
    }
}

} // namespace plc::core::network::grandpa
