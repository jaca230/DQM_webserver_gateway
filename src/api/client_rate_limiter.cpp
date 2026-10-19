#include "dqm_gateway/api/client_rate_limiter.h"

#include <algorithm>
#include <stdexcept>

namespace dqm_gateway {

ClientRateLimiter::ClientRateLimiter(double requests_per_second, std::size_t burst)
    : requests_per_second_(requests_per_second), burst_(static_cast<double>(burst)) {
    if (requests_per_second <= 0.0 || burst == 0) {
        throw std::invalid_argument("Rate limiter values must be positive");
    }
}

bool ClientRateLimiter::allow(const std::string& client) {
    const auto now = std::chrono::steady_clock::now();
    std::lock_guard lock(mutex_);
    if (buckets_.size() > 10000) {
        const auto cutoff = now - std::chrono::minutes(10);
        std::erase_if(buckets_, [cutoff](const auto& item) {
            return item.second.updated < cutoff;
        });
    }
    auto [position, inserted] = buckets_.try_emplace(client, Bucket{burst_, now});
    auto& bucket = position->second;
    if (!inserted) {
        const auto elapsed = std::chrono::duration<double>(now - bucket.updated).count();
        bucket.tokens = std::min(burst_, bucket.tokens + elapsed * requests_per_second_);
        bucket.updated = now;
    }
    if (bucket.tokens < 1.0) return false;
    bucket.tokens -= 1.0;
    return true;
}

}  // namespace dqm_gateway

