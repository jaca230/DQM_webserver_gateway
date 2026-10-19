#pragma once

#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <unordered_map>

namespace dqm_gateway {

class ClientRateLimiter {
public:
    ClientRateLimiter(double requests_per_second, std::size_t burst);
    bool allow(const std::string& client);

private:
    struct Bucket {
        double tokens;
        std::chrono::steady_clock::time_point updated;
    };

    const double requests_per_second_;
    const double burst_;
    std::mutex mutex_;
    std::unordered_map<std::string, Bucket> buckets_;
};

}  // namespace dqm_gateway

