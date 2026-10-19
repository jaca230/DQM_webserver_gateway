#include "dqm_gateway/product/waveform/waveform_store.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Waveform store selects and downsamples detector channels") {
    dqm_gateway::WaveformStore store(1024 * 1024, 16, 1024);
    dqm_gateway::WaveformSnapshot waveform;
    waveform.detector = "wfd5";
    waveform.epoch = "run-1";
    waveform.module = 2;
    waveform.submodule = 3;
    waveform.channel = 7;
    waveform.sample_period_ns = 1.0;
    waveform.sequence = 42;
    waveform.generated_at_unix_ns = 123456789;
    waveform.samples = {0, 1, 2, 3, 4, 5, 6, 7};
    store.update({waveform});

    const auto result = store.view("wfd5", 2, 3, 7, 4);
    REQUIRE(result.at("waveforms").size() == 1);
    CHECK(result.at("waveforms").front().at("samples") ==
          nlohmann::json::array({0, 2, 4, 6}));
    CHECK(result.at("waveforms").front().at("sample-period-ns") == 2.0);
    CHECK(result.at("generated-at-unix-ns") == 123456789);
}

TEST_CASE("Waveform batches are rejected atomically") {
    dqm_gateway::WaveformStore store(1024 * 1024, 16, 4);
    dqm_gateway::WaveformSnapshot valid;
    valid.detector = "sampic";
    valid.epoch = "run-1";
    valid.sample_period_ns = 1.0;
    valid.samples = {1, 2};
    auto invalid = valid;
    invalid.channel = 1;
    invalid.samples = {1, 2, 3, 4, 5};

    CHECK_THROWS_AS(store.update({valid, invalid}), std::invalid_argument);
    CHECK(store.catalog().empty());
}
