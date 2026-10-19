#include "dqm_gateway/metrics/gateway_metrics.h"

#include <sstream>

namespace dqm_gateway {

void GatewayMetrics::record_publication(std::size_t bytes, std::size_t products) noexcept {
    publications_.fetch_add(1, std::memory_order_relaxed);
    input_bytes_.fetch_add(bytes, std::memory_order_relaxed);
    products_.fetch_add(products, std::memory_order_relaxed);
}

void GatewayMetrics::record_malformed_publication() noexcept {
    malformed_publications_.fetch_add(1, std::memory_order_relaxed);
}

void GatewayMetrics::record_rejected_product() noexcept {
    rejected_products_.fetch_add(1, std::memory_order_relaxed);
}

void GatewayMetrics::record_http_request() noexcept {
    http_requests_.fetch_add(1, std::memory_order_relaxed);
}

void GatewayMetrics::record_rate_limited_request() noexcept {
    rate_limited_requests_.fetch_add(1, std::memory_order_relaxed);
}

void GatewayMetrics::set_websocket_clients(std::size_t value) noexcept {
    websocket_clients_.store(value, std::memory_order_relaxed);
}

std::string GatewayMetrics::prometheus(
    std::size_t stored_products,
    std::size_t stored_bytes) const {
    std::ostringstream output;
    output << "# TYPE dqm_gateway_publications_total counter\n"
           << "dqm_gateway_publications_total " << publications_.load() << '\n'
           << "# TYPE dqm_gateway_input_bytes_total counter\n"
           << "dqm_gateway_input_bytes_total " << input_bytes_.load() << '\n'
           << "# TYPE dqm_gateway_products_total counter\n"
           << "dqm_gateway_products_total " << products_.load() << '\n'
           << "# TYPE dqm_gateway_malformed_publications_total counter\n"
           << "dqm_gateway_malformed_publications_total "
           << malformed_publications_.load() << '\n'
           << "# TYPE dqm_gateway_rejected_products_total counter\n"
           << "dqm_gateway_rejected_products_total " << rejected_products_.load() << '\n'
           << "# TYPE dqm_gateway_http_requests_total counter\n"
           << "dqm_gateway_http_requests_total " << http_requests_.load() << '\n'
           << "# TYPE dqm_gateway_rate_limited_requests_total counter\n"
           << "dqm_gateway_rate_limited_requests_total "
           << rate_limited_requests_.load() << '\n'
           << "# TYPE dqm_gateway_stored_products gauge\n"
           << "dqm_gateway_stored_products " << stored_products << '\n'
           << "# TYPE dqm_gateway_stored_bytes gauge\n"
           << "dqm_gateway_stored_bytes " << stored_bytes << '\n'
           << "# TYPE dqm_gateway_websocket_clients gauge\n"
           << "dqm_gateway_websocket_clients " << websocket_clients_.load() << '\n';
    return output.str();
}

}  // namespace dqm_gateway

