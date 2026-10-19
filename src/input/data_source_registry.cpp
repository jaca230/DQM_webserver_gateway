#include "dqm_gateway/input/data_source_registry.h"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <utility>

namespace dqm_gateway {

DataSourceRegistry::~DataSourceRegistry() {
    stop_all();
}

void DataSourceRegistry::add(std::unique_ptr<DataSource> source) {
    if (!source) throw std::invalid_argument("Cannot register a null data source");
    const std::string name(source->name());
    if (name.empty()) throw std::invalid_argument("Data source name cannot be empty");
    if (!sources_.emplace(name, std::move(source)).second) {
        throw std::invalid_argument("Duplicate data source: " + name);
    }
}

void DataSourceRegistry::start_all() {
    for (const auto& [name, source] : sources_) {
        (void)name;
        source->start();
    }
}

void DataSourceRegistry::stop_all() {
    for (const auto& [name, source] : sources_) {
        (void)name;
        source->stop();
    }
}

bool DataSourceRegistry::healthy() const noexcept {
    if (sources_.empty()) return false;
    for (const auto& [name, source] : sources_) {
        (void)name;
        if (!source->running()) return false;
    }
    return true;
}

nlohmann::json DataSourceRegistry::status() const {
    auto result = nlohmann::json::array();
    for (const auto& [name, source] : sources_) {
        result.push_back({{"name", name}, {"running", source->running()}});
    }
    return result;
}

}  // namespace dqm_gateway
