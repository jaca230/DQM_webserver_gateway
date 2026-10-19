#include "dqm_gateway/product/histogram/histogram_store.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace {

dqm_gateway::HistogramSnapshot snapshot(
    std::string epoch,
    std::vector<double> counts,
    std::vector<double> sumw2 = {}) {
    dqm_gateway::HistogramSnapshot result;
    result.name = "detector/hits";
    result.definition = "bins-v1";
    result.epoch = std::move(epoch);
    result.sequence = 1;
    result.generated_at_unix_ns = 123456789;
    result.edges = {0.0, 1.0, 2.0, 3.0, 4.0};
    result.counts = std::move(counts);
    result.sumw2 = std::move(sumw2);
    return result;
}

}  // namespace

TEST_CASE("Histogram baselines subtract cumulative accumulators") {
    dqm_gateway::HistogramStore histograms(1024 * 1024, 100, 100, 1024 * 1024);
    histograms.update(snapshot("run-1", {1, 2, 3, 4}, {1, 2, 3, 4}));
    REQUIRE(histograms.create_baseline("detector/hits", "manual"));
    auto updated = snapshot("run-1", {3, 5, 8, 10}, {3, 5, 8, 10});
    updated.sequence = 2;
    histograms.update(std::move(updated));

    const auto view = histograms.view("detector/hits", "manual", 4);
    CHECK(view.at("counts") == nlohmann::json::array({2, 3, 5, 6}));
    CHECK(view.at("sumw2") == nlohmann::json::array({2, 3, 5, 6}));
}

TEST_CASE("Histogram baselines cannot cross epochs") {
    dqm_gateway::HistogramStore histograms(1024 * 1024, 100, 100, 1024 * 1024);
    histograms.update(snapshot("run-1", {1, 2, 3, 4}));
    REQUIRE(histograms.create_baseline("detector/hits", "manual"));
    histograms.update(snapshot("run-2", {2, 3, 4, 5}));
    CHECK_THROWS_AS(
        histograms.view("detector/hits", "manual", 4),
        std::logic_error);
}

TEST_CASE("Histogram views aggregate to a requested bin budget") {
    dqm_gateway::HistogramStore histograms(1024 * 1024, 100, 100, 1024 * 1024);
    histograms.update(snapshot("run-1", {1, 2, 3, 4}));
    const auto view = histograms.view("detector/hits", std::nullopt, 2);
    CHECK(view.at("counts") == nlohmann::json::array({3, 7}));
    CHECK(view.at("edges") == nlohmann::json::array({0, 2, 4}));
    CHECK(view.at("binning-factor") == 2);
    CHECK(view.at("generated-at-unix-ns") == 123456789);
}

TEST_CASE("Histogram batches are rejected atomically") {
    dqm_gateway::HistogramStore histograms(1024 * 1024, 100, 100, 1024 * 1024);
    auto valid = snapshot("run-1", {1, 2, 3, 4});
    valid.name = "valid";
    auto invalid = snapshot("run-1", {1, 2, 3, 4});
    invalid.name = "invalid";
    invalid.edges = {0, 1};

    CHECK_THROWS_AS(
        histograms.update(std::vector<dqm_gateway::HistogramSnapshot>{valid, invalid}),
        std::invalid_argument);
    CHECK(histograms.catalog().empty());
}
