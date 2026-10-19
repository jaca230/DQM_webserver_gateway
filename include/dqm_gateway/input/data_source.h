#pragma once

#include <string_view>

namespace dqm_gateway {

class DataSource {
public:
    virtual ~DataSource() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    [[nodiscard]] virtual bool running() const noexcept = 0;
};

}  // namespace dqm_gateway
