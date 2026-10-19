#include "dqm_gateway/config/sections/data_source_config.h"

#include "dqm_gateway/config/config_value.h"

#include <stdexcept>
#include <unordered_set>

namespace dqm_gateway {

void DataSourceConfig::parse(const nlohmann::json& value) {
    if (!value.is_array() || value.empty()) {
        throw std::invalid_argument("sources must be a non-empty array");
    }
    std::vector<DataSourceDefinition> parsed;
    std::unordered_set<std::string> names;
    for (const auto& item : value) {
        config_value::validate_keys(item, {"type", "name", "config"}, "sources[]");
        DataSourceDefinition definition{
            item.at("type").get<std::string>(),
            item.at("name").get<std::string>(),
            item.value("config", nlohmann::json::object()),
        };
        if (definition.type.empty() || definition.name.empty() || !definition.config.is_object()) {
            throw std::invalid_argument("Every source requires a type, name, and object config");
        }
        if (!names.insert(definition.name).second) {
            throw std::invalid_argument("Duplicate data source name: " + definition.name);
        }
        parsed.push_back(std::move(definition));
    }
    definitions_ = std::move(parsed);
}

}  // namespace dqm_gateway
