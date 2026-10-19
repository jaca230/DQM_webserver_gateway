#pragma once

#include "dqm_gateway/api/route/api_route.h"

#include <cstddef>
#include <optional>

#include <drogon/HttpRequest.h>

namespace dqm_gateway {

class DataStore;

class DataRoute : public ApiRoute {
public:
    explicit DataRoute(DataStore& store) : store_(store) {}

protected:
    [[nodiscard]] DataStore& store() const noexcept { return store_; }
    static std::optional<int> optional_integer(
        const drogon::HttpRequestPtr& request,
        const char* name);
    static std::size_t bounded_size(
        const drogon::HttpRequestPtr& request,
        const char* name,
        std::size_t maximum);

private:
    DataStore& store_;
};

}  // namespace dqm_gateway
