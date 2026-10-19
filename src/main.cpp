#include "dqm_gateway/api/http_api.h"
#include "dqm_gateway/api/live_websocket_controller.h"
#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/api/route/system/register_system_routes.h"
#include "dqm_gateway/api/websocket_hub.h"
#include "dqm_gateway/config/gateway_config.h"
#include "dqm_gateway/input/data_source_factory_registry.h"
#include "dqm_gateway/input/data_source_registry.h"
#include "dqm_gateway/metrics/gateway_metrics.h"
#include "dqm_gateway/plugin/plugin_loader.h"
#include "dqm_gateway/plugin/plugin_registry.h"
#include "dqm_gateway/product/data_product_factory_registry.h"
#include "dqm_gateway/product/data_product_registry.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <drogon/drogon.h>

namespace {

struct CommandLine {
    std::filesystem::path configuration{"config/config.json"};
    std::optional<std::string> address;
    std::optional<std::uint16_t> port;
    std::optional<std::size_t> threads;
    std::optional<std::filesystem::path> plugin_directory;
};

std::size_t positive_integer(const std::string& value, const char* option) {
    std::size_t consumed = 0;
    const auto parsed = std::stoull(value, &consumed);
    if (consumed != value.size() || parsed == 0) {
        throw std::invalid_argument(std::string(option) + " requires a positive integer");
    }
    return static_cast<std::size_t>(parsed);
}

CommandLine command_line(int argc, char** argv) {
    CommandLine result;
    for (int index = 1; index < argc; ++index) {
        const std::string argument(argv[index]);
        if ((argument == "--config" || argument == "-c") && index + 1 < argc) {
            result.configuration = argv[++index];
        } else if (argument == "--address" && index + 1 < argc) {
            result.address = argv[++index];
            if (result.address->empty()) throw std::invalid_argument("--address cannot be empty");
        } else if (argument == "--port" && index + 1 < argc) {
            const auto value = positive_integer(argv[++index], "--port");
            if (value > 65535) throw std::invalid_argument("--port must not exceed 65535");
            result.port = static_cast<std::uint16_t>(value);
        } else if (argument == "--threads" && index + 1 < argc) {
            result.threads = positive_integer(argv[++index], "--threads");
        } else if (argument == "--plugin-dir" && index + 1 < argc) {
            result.plugin_directory = argv[++index];
        } else if (argument == "--help" || argument == "-h") {
            std::cout
                << "Usage: dqm_gateway [OPTIONS]\n\n"
                << "  --config PATH       Configuration file (default: config/config.json)\n"
                << "  --address ADDRESS   Override the configured listen address\n"
                << "  --port PORT         Override the configured HTTP port\n"
                << "  --threads COUNT     Override the configured HTTP worker count\n"
                << "  --plugin-dir PATH   Plugin directory (default: beside executable)\n";
            std::exit(0);
        } else {
            throw std::invalid_argument("Unknown command-line argument: " + argument);
        }
    }
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const auto options = command_line(argc, argv);
        const auto config = dqm_gateway::GatewayConfig::from_file(
            options.configuration);
        const auto address = options.address.value_or(config.server().address);
        const auto port = options.port.value_or(config.server().port);
        const auto threads = options.threads.value_or(config.server().threads);
        const auto plugin_directory = options.plugin_directory.value_or(
            std::filesystem::absolute(argv[0]).parent_path() / "plugins");
        dqm_gateway::GatewayMetrics metrics;
        dqm_gateway::WebsocketHub websocket(
            config.limits().websocket_maximum_rate_hz,
            config.server().allowed_origins,
            metrics);
        dqm_gateway::LiveWebsocketController::configure_hub(websocket);

        // The loader must outlive every factory and object implemented by a plugin.
        dqm_gateway::PluginLoader plugin_loader;
        dqm_gateway::DataProductFactoryRegistry product_factories;
        dqm_gateway::DataSourceFactoryRegistry source_factories;
        dqm_gateway::PluginRegistry plugin_registry(product_factories, source_factories);
        plugin_loader.load_directory(plugin_directory, plugin_registry);

        dqm_gateway::DataProductRegistry products;
        product_factories.create_all(
            {config.store(), config.limits(),
             [&websocket](const dqm_gateway::Product& product) {
                 websocket.publish(product);
             }},
            products);

        dqm_gateway::DataSourceRegistry sources;
        source_factories.create_all(
            config.sources(),
            {[&products, &metrics](dqm_gateway::Product product) {
                 if (!products.ingest(product)) metrics.record_rejected_product();
             },
             metrics},
            sources);

        dqm_gateway::ApiContext api_context(metrics, config.limits());
        dqm_gateway::ApiRouteRegistry routes(api_context);
        products.register_routes(routes);
        dqm_gateway::register_system_routes(routes, sources, products, metrics);
        dqm_gateway::HttpApi api(routes);
        api.register_routes();

        const auto allowed_origins = config.server().allowed_origins;
        drogon::app().registerPostHandlingAdvice(
            [allowed_origins](
                const drogon::HttpRequestPtr& request,
                const drogon::HttpResponsePtr& response) {
                const auto origin = request->getHeader("origin");
                if (!origin.empty() &&
                    std::find(allowed_origins.begin(), allowed_origins.end(), origin) !=
                        allowed_origins.end()) {
                    response->addHeader("Access-Control-Allow-Origin", origin);
                    response->addHeader("Vary", "Origin");
                }
            });

        sources.start_all();
        std::cout << "[dqm_gateway] Loaded " << plugin_loader.loaded_plugins().size()
                  << " plugin(s), started " << config.sources().definitions().size()
                  << " data source(s), and serving http://" << address << ':' << port << '\n';
        drogon::app()
            .addListener(address, port)
            .setThreadNum(threads)
            .run();
        sources.stop_all();
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "[dqm_gateway] " << exception.what() << '\n';
        return 1;
    }
}
