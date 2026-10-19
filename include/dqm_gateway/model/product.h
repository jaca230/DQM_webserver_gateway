#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace dqm_gateway {

using ProductPayload = std::shared_ptr<const std::vector<std::byte>>;

struct Product {
    std::string id;
    std::string source;
    std::string topic;
    std::string content_type;
    std::string schema;
    std::uint64_t publication_sequence{0};
    std::uint64_t product_sequence{0};
    std::int64_t received_at_unix_ns{0};
    nlohmann::json metadata;
    ProductPayload payload;

    std::size_t size() const noexcept;
};

nlohmann::json product_summary(const Product& product);
std::string make_product_id(
    std::string_view source,
    std::string_view topic,
    std::string_view schema,
    std::size_t product_index);

}  // namespace dqm_gateway
