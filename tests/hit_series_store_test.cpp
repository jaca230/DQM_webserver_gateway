#include "dqm_gateway/product/hit_series/hit_series_store.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Hit-series store selects and bounds detector hits") {
    dqm_gateway::HitSeriesStore store(1024 * 1024, 16, 16);
    dqm_gateway::HitSeriesSnapshot series;
    series.detector = "musip";
    series.epoch = "run-1";
    series.module = 1;
    series.channel = 4;
    series.sequence = 42;
    series.generated_at_unix_ns = 123456789;
    series.times_ns = {100, 108, 116, 124};
    series.energies = {12, 15, 9, 20};
    store.update({series});

    const auto result = store.view("musip", 1, 0, 4, 2);
    REQUIRE(result.at("hit-series").size() == 1);
    CHECK(result.at("hit-series").front().at("times-ns") ==
          nlohmann::json::array({100, 116}));
    CHECK(result.at("hit-series").front().at("energies") ==
          nlohmann::json::array({12, 9}));
    CHECK(result.at("generated-at-unix-ns") == 123456789);
}

TEST_CASE("Hit-series batches are rejected atomically") {
    dqm_gateway::HitSeriesStore store(1024 * 1024, 16, 4);
    dqm_gateway::HitSeriesSnapshot valid;
    valid.detector = "musip";
    valid.epoch = "run-1";
    valid.times_ns = {1, 2};
    valid.energies = {3, 4};
    auto invalid = valid;
    invalid.channel = 1;
    invalid.times_ns.push_back(3);

    CHECK_THROWS_AS(store.update({valid, invalid}), std::invalid_argument);
    CHECK(store.catalog().empty());
}

TEST_CASE("Hit-series store retains a bounded rolling history within an epoch") {
    dqm_gateway::HitSeriesStore store(1024 * 1024, 16, 4);
    dqm_gateway::HitSeriesSnapshot first;
    first.detector = "musip";
    first.epoch = "run-1";
    first.channel = 2;
    first.sequence = 1;
    first.generated_at_unix_ns = 100;
    first.times_ns = {10, 20, 30};
    first.energies = {1, 2, 3};
    store.update({first});

    auto second = first;
    second.sequence = 2;
    second.generated_at_unix_ns = 200;
    second.times_ns = {40, 50};
    second.energies = {4, 5};
    store.update({second});

    auto result = store.view("musip", 0, 0, 2, 4);
    CHECK(result.at("generated-at-unix-ns") == 200);
    CHECK(result.at("hit-series").front().at("times-ns") ==
          nlohmann::json::array({20, 30, 40, 50}));
    CHECK(result.at("hit-series").front().at("energies") ==
          nlohmann::json::array({2, 3, 4, 5}));

    second.epoch = "run-2";
    second.sequence = 1;
    second.times_ns = {60};
    second.energies = {6};
    store.update({second});
    result = store.view("musip", 0, 0, 2, 4);
    CHECK(result.at("hit-series").front().at("times-ns") ==
          nlohmann::json::array({60}));
}
