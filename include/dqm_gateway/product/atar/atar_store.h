#pragma once

#include "dqm_gateway/model/stores/data_store.h"

#include <cstddef>
#include <shared_mutex>
#include <string>

#include <nlohmann/json.hpp>

namespace dqm_gateway {

class AtarStore final : public DataStore {
public:
    explicit AtarStore(std::size_t maximum_bytes);
    void update(nlohmann::json product);
    [[nodiscard]] nlohmann::json view(const std::string& name) const;
    [[nodiscard]] std::string_view name() const noexcept override { return "atar"; }
    [[nodiscard]] nlohmann::json catalog_json() const override;
    [[nodiscard]] std::size_t stored_items() const override;
    [[nodiscard]] std::size_t stored_bytes() const override;

private:
    std::size_t maximum_bytes_;
    std::size_t bytes_{0};
    mutable std::shared_mutex mutex_;
    nlohmann::json product_;
};

}  // namespace dqm_gateway
