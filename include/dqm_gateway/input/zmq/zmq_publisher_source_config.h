#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class ZmqPublisherSourceConfig final {
public:
    static ZmqPublisherSourceConfig from_json(const nlohmann::json& value);

    std::string endpoint{"tcp://127.0.0.1:5555"};
    std::vector<std::string> topics;
    int receive_high_water_mark{1024};
    std::size_t maximum_product_bytes{64U * 1024U * 1024U};
};

}  // namespace dqm_gateway
