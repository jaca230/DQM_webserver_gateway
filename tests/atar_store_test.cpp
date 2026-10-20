#include "dqm_gateway/product/atar/atar_store.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ATAR store returns only the requested view") {
    dqm_gateway::AtarStore store(4096);
    store.update({
        {"schema", "dqm-atar-view-v1"}, {"view", "occupancy"},
        {"epoch", "run-1"}, {"sequence", 7},
        {"generated-at-unix-ns", 123},
        {"geometry", {{"channels", 256}, {"layers", 8}, {"strips-per-layer", 32}}},
        {"data", {{"counts", {1, 2, 3}}}},
    });
    store.update({{"schema", "dqm-atar-view-v1"}, {"view", "hits-per-event"},
        {"epoch", "run-1"}, {"sequence", 9}, {"data", {{"counts", {4, 5}}}}});

    const auto view = store.view("occupancy");
    REQUIRE(view.at("view") == "occupancy");
    REQUIRE(view.at("sequence") == 7);
    REQUIRE(view.at("data").at("counts") == nlohmann::json({1, 2, 3}));
    REQUIRE_FALSE(view.contains("hits-per-event"));
    CHECK(store.view("hits-per-event").at("sequence") == 9);
    CHECK(store.stored_items() == 2);
    REQUIRE_THROWS_AS(store.view("not-a-view"), std::invalid_argument);
}
