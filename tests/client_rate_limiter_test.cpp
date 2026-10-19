#include "dqm_gateway/api/client_rate_limiter.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Client rate limiter enforces an independent burst per client") {
    dqm_gateway::ClientRateLimiter limiter(1.0, 2);

    CHECK(limiter.allow("client-a"));
    CHECK(limiter.allow("client-a"));
    CHECK_FALSE(limiter.allow("client-a"));
    CHECK(limiter.allow("client-b"));
}
