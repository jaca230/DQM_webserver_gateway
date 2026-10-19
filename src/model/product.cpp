#include "dqm_gateway/model/product.h"

#include <cctype>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string_view>

namespace dqm_gateway {
namespace {

std::string safe_component(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const unsigned char character : value) {
        result.push_back(
            std::isalnum(character) || character == '-' || character == '_' || character == '.'
                ? static_cast<char>(character)
                : '_');
    }
    return result.empty() ? "unspecified" : result;
}

}  // namespace

std::size_t Product::size() const noexcept {
    return payload ? payload->size() : 0;
}

nlohmann::json product_summary(const Product& product) {
    return {
        {"id", product.id},
        {"source", product.source},
        {"topic", product.topic},
        {"content-type", product.content_type},
        {"schema", product.schema},
        {"publication-sequence", product.publication_sequence},
        {"product-sequence", product.product_sequence},
        {"received-at-unix-ns", product.received_at_unix_ns},
        {"size", product.size()},
    };
}

std::string make_product_id(
    std::string_view source,
    std::string_view topic,
    std::string_view schema,
    std::size_t product_index) {
    std::ostringstream stream;
    stream << safe_component(source) << '.' << safe_component(topic) << '.'
           << safe_component(schema) << '.' << product_index;
    return stream.str();
}

}  // namespace dqm_gateway

