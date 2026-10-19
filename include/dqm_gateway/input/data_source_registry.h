#pragma once

#include "dqm_gateway/input/data_source.h"

#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class DataSourceRegistry final {
public:
    ~DataSourceRegistry();

    void add(std::unique_ptr<DataSource> source);
    void start_all();
    void stop_all();
    [[nodiscard]] bool healthy() const noexcept;
    [[nodiscard]] nlohmann::json status() const;

private:
    std::unordered_map<std::string, std::unique_ptr<DataSource>> sources_;
};

}  // namespace dqm_gateway
