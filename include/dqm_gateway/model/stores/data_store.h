#pragma once

#include <cstddef>
#include <string_view>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class DataStore {
public:
    virtual ~DataStore() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual nlohmann::json catalog_json() const = 0;
    [[nodiscard]] virtual std::size_t stored_items() const = 0;
    [[nodiscard]] virtual std::size_t stored_bytes() const = 0;
};

}  // namespace dqm_gateway
