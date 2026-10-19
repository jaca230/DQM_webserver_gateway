#pragma once

#include "dqm_gateway/config/config_section.h"

#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace dqm_gateway {

struct DataSourceDefinition {
    std::string type;
    std::string name;
    nlohmann::json config;
};

class DataSourceConfig final : public ConfigSection {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "sources"; }
    void parse(const nlohmann::json& value) override;

    [[nodiscard]] const std::vector<DataSourceDefinition>& definitions() const noexcept {
        return definitions_;
    }

private:
    std::vector<DataSourceDefinition> definitions_;
};

}  // namespace dqm_gateway
