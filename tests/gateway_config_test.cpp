#include "dqm_gateway/config/gateway_config.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TemporaryConfig {
public:
    explicit TemporaryConfig(const std::string& contents) {
        path_ = std::filesystem::temp_directory_path() /
            ("dqm_gateway_config_" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()) + ".json");
        std::ofstream output(path_);
        output << contents;
    }
    ~TemporaryConfig() { std::filesystem::remove(path_); }
    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST_CASE("Gateway configuration reads typed values") {
    TemporaryConfig file(R"({
        "server": {"port": 9000, "threads": 4},
        "sources": [{
            "type": "zmq-publisher",
            "name": "test-publisher",
            "config": {"endpoint": "tcp://127.0.0.1:6000"}
        }],
        "limits": {"request-burst": 12}
    })");
    const auto config = dqm_gateway::GatewayConfig::from_file(file.path());
    CHECK(config.server().port == 9000);
    CHECK(config.server().threads == 4);
    REQUIRE(config.sources().definitions().size() == 1);
    CHECK(config.sources().definitions().front().type == "zmq-publisher");
    CHECK(config.sources().definitions().front().name == "test-publisher");
    CHECK(config.sources().definitions().front().config.at("endpoint") ==
          "tcp://127.0.0.1:6000");
    CHECK(config.limits().request_burst == 12);
}

TEST_CASE("Gateway configuration rejects unknown keys") {
    TemporaryConfig file(R"({"server": {"mystery": true}})");
    CHECK_THROWS_AS(
        dqm_gateway::GatewayConfig::from_file(file.path()),
        std::invalid_argument);
}

