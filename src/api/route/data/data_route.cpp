#include "dqm_gateway/api/route/data/data_route.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace dqm_gateway {

std::optional<int> DataRoute::optional_integer(
    const drogon::HttpRequestPtr& request,
    const char* name) {
    const auto value = request->getParameter(name);
    if (value.empty()) return std::nullopt;
    std::size_t consumed = 0;
    const auto parsed = std::stoi(value, &consumed);
    if (consumed != value.size()) throw std::invalid_argument("not an integer");
    return parsed;
}

std::size_t DataRoute::bounded_size(
    const drogon::HttpRequestPtr& request,
    const char* name,
    std::size_t maximum) {
    const auto value = request->getParameter(name);
    if (value.empty()) return maximum;
    std::size_t consumed = 0;
    const auto parsed = std::stoull(value, &consumed);
    if (consumed != value.size() || parsed == 0) {
        throw std::invalid_argument(std::string(name) + " must be a positive integer");
    }
    return std::min(maximum, static_cast<std::size_t>(parsed));
}

}  // namespace dqm_gateway
