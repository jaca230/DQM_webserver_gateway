#include "dqm_gateway/input/zmq/zmq_publisher_source_config.h"

#include "dqm_gateway/config/config_value.h"

#include <stdexcept>

namespace dqm_gateway {

ZmqPublisherSourceConfig ZmqPublisherSourceConfig::from_json(
    const nlohmann::json& value) {
    config_value::validate_keys(
        value,
        {"endpoint", "topics", "receive-high-water-mark", "maximum-product-bytes"},
        "sources[].config");
    ZmqPublisherSourceConfig result;
    config_value::read_optional(value, "endpoint", result.endpoint);
    config_value::read_optional(value, "topics", result.topics);
    config_value::read_optional(
        value, "receive-high-water-mark", result.receive_high_water_mark);
    config_value::read_optional(
        value, "maximum-product-bytes", result.maximum_product_bytes);
    if (result.endpoint.empty() || result.receive_high_water_mark <= 0 ||
        result.maximum_product_bytes == 0) {
        throw std::invalid_argument("ZMQ publisher source configuration is invalid");
    }
    return result;
}

}  // namespace dqm_gateway
