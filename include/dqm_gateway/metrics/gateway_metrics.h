#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>

namespace dqm_gateway {

class GatewayMetrics {
public:
    void record_publication(std::size_t bytes, std::size_t products) noexcept;
    void record_malformed_publication() noexcept;
    void record_rejected_product() noexcept;
    void record_http_request() noexcept;
    void record_rate_limited_request() noexcept;
    void set_websocket_clients(std::size_t value) noexcept;

    std::string prometheus(std::size_t stored_products, std::size_t stored_bytes) const;

private:
    std::atomic<std::uint64_t> publications_{0};
    std::atomic<std::uint64_t> input_bytes_{0};
    std::atomic<std::uint64_t> products_{0};
    std::atomic<std::uint64_t> malformed_publications_{0};
    std::atomic<std::uint64_t> rejected_products_{0};
    std::atomic<std::uint64_t> http_requests_{0};
    std::atomic<std::uint64_t> rate_limited_requests_{0};
    std::atomic<std::uint64_t> websocket_clients_{0};
};

}  // namespace dqm_gateway

