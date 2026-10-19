#include "dqm_gateway/input/zmq/zmq_publisher_source.h"

#include "dqm_gateway/metrics/gateway_metrics.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <zmq.hpp>

namespace dqm_gateway {
namespace {

constexpr std::size_t kMaximumMetadataBytes = 1024U * 1024U;

std::int64_t unix_time_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string frame_string(const zmq::message_t& frame) {
    return {static_cast<const char*>(frame.data()), frame.size()};
}

std::vector<zmq::message_t> receive_message(zmq::socket_t& socket) {
    std::vector<zmq::message_t> frames;
    do {
        zmq::message_t frame;
        const auto received = socket.recv(frame, zmq::recv_flags::none);
        if (!received) return {};
        frames.push_back(std::move(frame));
    } while (socket.get(zmq::sockopt::rcvmore));
    return frames;
}

}  // namespace

ZmqPublisherSource::ZmqPublisherSource(
    const DataSourceDefinition& definition,
    const DataSourceDependencies& dependencies)
    : name_(definition.name),
      config_(ZmqPublisherSourceConfig::from_json(definition.config)),
      handler_(dependencies.product_handler),
      metrics_(dependencies.metrics) {
    if (!handler_) throw std::invalid_argument("ZMQ publisher source requires a handler");
}

ZmqPublisherSource::~ZmqPublisherSource() {
    stop();
}

void ZmqPublisherSource::start() {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) return;
    thread_ = std::thread(&ZmqPublisherSource::receive_loop, this);
}

void ZmqPublisherSource::stop() {
    running_.store(false);
    if (thread_.joinable()) thread_.join();
}

void ZmqPublisherSource::receive_loop() {
    try {
        zmq::context_t context(1);
        zmq::socket_t socket(context, zmq::socket_type::sub);
        socket.set(zmq::sockopt::rcvhwm, config_.receive_high_water_mark);
        socket.set(
            zmq::sockopt::maxmsgsize,
            static_cast<std::int64_t>(config_.maximum_product_bytes));
        if (config_.topics.empty()) {
            socket.set(zmq::sockopt::subscribe, "");
        } else {
            for (const auto& topic : config_.topics) socket.set(zmq::sockopt::subscribe, topic);
        }
        socket.connect(config_.endpoint);

        while (running_.load(std::memory_order_relaxed)) {
            zmq::pollitem_t item{static_cast<void*>(socket), 0, ZMQ_POLLIN, 0};
            zmq::poll(&item, 1, std::chrono::milliseconds(100));
            if (!(item.revents & ZMQ_POLLIN)) continue;

            try {
                auto frames = receive_message(socket);
                if (frames.size() < 2 || frames[1].size() > kMaximumMetadataBytes) {
                    metrics_.record_malformed_publication();
                    continue;
                }
                const auto topic = frame_string(frames[0]);
                const auto envelope = nlohmann::json::parse(frame_string(frames[1]));
                if (envelope.value("protocol", std::string{}) != "zmq-publisher-v1" ||
                    !envelope.contains("products") || !envelope.at("products").is_array() ||
                    envelope.at("products").size() + 2 != frames.size()) {
                    metrics_.record_malformed_publication();
                    continue;
                }

                const auto publication_sequence = envelope.at("sequence").get<std::uint64_t>();
                const auto received_at = unix_time_ns();
                std::size_t bytes = frames[0].size() + frames[1].size();
                const auto& products = envelope.at("products");
                for (std::size_t index = 0; index < products.size(); ++index) {
                    const auto& metadata = products[index];
                    const auto& frame = frames[index + 2];
                    bytes += frame.size();
                    if (frame.size() > config_.maximum_product_bytes ||
                        metadata.value("size", frame.size()) != frame.size()) {
                        metrics_.record_rejected_product();
                        continue;
                    }

                    auto payload = std::make_shared<std::vector<std::byte>>(frame.size());
                    if (!payload->empty()) std::memcpy(payload->data(), frame.data(), frame.size());
                    Product product;
                    product.source = name_ + '@' + config_.endpoint;
                    product.topic = topic;
                    product.content_type = metadata.value(
                        "content-type", std::string{"application/octet-stream"});
                    product.schema = metadata.value("schema", std::string{});
                    product.publication_sequence = publication_sequence;
                    product.product_sequence = metadata.value("sequence", std::uint64_t{0});
                    product.received_at_unix_ns = received_at;
                    product.metadata = metadata;
                    product.payload = std::move(payload);
                    product.id = make_product_id(
                        product.source, product.topic, product.schema, index);
                    handler_(std::move(product));
                }
                metrics_.record_publication(bytes, products.size());
            } catch (const std::exception& exception) {
                metrics_.record_malformed_publication();
                std::cerr << "[source/" << name_ << "] Rejected publication: "
                          << exception.what() << '\n';
            }
        }

        socket.close();
        context.shutdown();
        context.close();
    } catch (const std::exception& exception) {
        std::cerr << "[source/" << name_ << "] Fatal receiver error: "
                  << exception.what() << '\n';
        running_.store(false);
    }
}

}  // namespace dqm_gateway
