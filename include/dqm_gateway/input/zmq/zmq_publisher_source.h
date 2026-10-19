#pragma once

#include "dqm_gateway/input/data_source.h"
#include "dqm_gateway/input/data_source_factory_registry.h"
#include "dqm_gateway/input/zmq/zmq_publisher_source_config.h"

#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace dqm_gateway {

class GatewayMetrics;

class ZmqPublisherSource final : public DataSource {
public:
    ZmqPublisherSource(
        const DataSourceDefinition& definition,
        const DataSourceDependencies& dependencies);
    ~ZmqPublisherSource() override;

    ZmqPublisherSource(const ZmqPublisherSource&) = delete;
    ZmqPublisherSource& operator=(const ZmqPublisherSource&) = delete;

    [[nodiscard]] std::string_view name() const noexcept override { return name_; }
    void start() override;
    void stop() override;
    [[nodiscard]] bool running() const noexcept override { return running_.load(); }

private:
    void receive_loop();

    std::string name_;
    ZmqPublisherSourceConfig config_;
    std::function<void(Product)> handler_;
    GatewayMetrics& metrics_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};

}  // namespace dqm_gateway
