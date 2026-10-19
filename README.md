# dqm_gateway

`dqm_gateway` is a bounded C++ data gateway between online data sources and DQM
clients. Its built-in source consumes `zmq-publisher-v1` multipart
publications, retains only the latest immutable product for each stable ID, and
serves small selected views through asynchronous HTTP and WebSocket APIs.

The gateway does not run reconstruction or ask the publisher to regenerate
data. Gaudi remains responsible for unpacking and analysis; this process owns
web-facing caching, selection, downsampling, throttling, and display baselines.

## Build and test

The first configure downloads pinned Drogon, JsonCpp, nlohmann/json, and Catch2
sources. ZeroMQ and cppzmq are system dependencies.

```sh
./setup_env.sh
./scripts/build.sh --overwrite
ctest --test-dir build --output-on-failure
```

The helper builds `Release` by default; use `--build-type Debug` while
developing.

Run the gateway with:

```sh
./scripts/run.sh
# or
./scripts/run.sh --config /path/to/config.json --port 8080 --threads 4
```

The default configuration creates a ZeroMQ publisher source at
`tcp://127.0.0.1:5555` and serves `http://127.0.0.1:8080`.

To exercise it with the publisher, start the gateway first and then run:

```sh
cd /home/jack/cpp_projects/midas_stuff/midas_publisher
./scripts/run.sh --config config/config.json
```

Then inspect:

```sh
curl http://127.0.0.1:8080/api/v1/health
curl http://127.0.0.1:8080/api/v1/catalog
curl http://127.0.0.1:8080/metrics
```

## Architecture

```text
PluginLoader ── discovers versioned shared modules
    ├── DataSourceFactoryRegistry
    │     └── DataSourceRegistry ── ZMQ today; more inputs can be registered
    │                    │
    │                    ▼
    └── DataProductFactoryRegistry
          └── DataProductRegistry
                ├── raw product ── store/routes
                ├── waveform ───── decoder/store/routes
                ├── hit series ─── decoder/store/routes
                └── histogram ──── decoder/store/routes
                                         │
                                         ▼
ApiRouteRegistry ───────── Drogon HTTP/WebSocket API
```

A `DataProduct` is the primary extension boundary. Each product module owns its
decoder, bounded store, and HTTP route objects, then contributes those routes to
the shared registry. `HttpApi` only activates that registry; it has no knowledge
of concrete products or stores. Routes that expose stored data derive from
`DataRoute`, while process-level health, catalog, and metrics routes live under
`api/route/system`.

Input is independently extensible through `DataSource`, its factory registry,
and the configured `sources` array. Adding a source does not alter a product
module or the HTTP API. Configuration sections are likewise separate typed
classes registered by `GatewayConfig`.

## Plugins

Concrete products and inputs are loadable modules in `build/bin/plugins`. Each
module owns its implementation and exports the same versioned entry point via
`DQM_GATEWAY_PLUGIN`. The loader rejects missing or incompatible entry points
before constructing any runtime objects, and keeps every module loaded until
all plugin-created factories and objects have been destroyed.

A product registers locally in its own `register_plugin.cpp`:

```cpp
void register_plugin(dqm_gateway::PluginRegistry& registry) {
    registry.add_data_product<MyProduct>("my-product");
}

DQM_GATEWAY_PLUGIN(register_plugin)
```

Inputs use the identical pattern with `add_data_source`. Product and source
factory registries share the generic `FactoryRegistry` implementation, so name
validation, duplicate rejection, lookup, and registration ordering are defined
once.

Every plugin folder contains its own `CMakeLists.txt`. CMake discovers plugin
folders under `src/product` and `src/input`, so adding a module does not require
editing the root build file or a central registration list. The executable
searches the `plugins` directory beside itself by default; `--plugin-dir` can
select an installed or development plugin directory explicitly.

These modules are an internal binary extension boundary and must be built
against the same gateway headers and a compatible C++ toolchain. If plugins are
later distributed independently, the registry reference should be replaced by
a narrow C function table before treating the ABI as stable across compilers.

The default source has this shape:

```json
{
  "sources": [
    {
      "type": "zmq-publisher",
      "name": "online-publisher",
      "config": {
        "endpoint": "tcp://127.0.0.1:5555",
        "topics": ["TIME", "DATA", "DQM"]
      }
    }
  ]
}
```

Memory is bounded by `maximum-product-bytes`, `maximum-total-bytes`, and
`maximum-products`. The store evicts the oldest latest-value product when
necessary. It never retains an unbounded event history.

## APIs

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api/v1/health` | Process and configured-source health |
| `GET` | `/api/v1/catalog` | Available products, waveforms, and histograms |
| `GET` | `/api/v1/products/metadata?id=...` | Metadata without the payload |
| `GET` | `/api/v1/products/json?id=...` | A bounded JSON product |
| `GET` | `/api/v1/products/payload?id=...` | Raw bounded payload with `ETag` |
| `GET` | `/api/v1/waveforms?detector=...` | Selected detector traces with optional module/channel filters |
| `GET` | `/api/v1/hit-series?detector=...` | Selected hit times and energies with optional module/channel filters |
| `GET` | `/api/v1/histograms?name=...` | Histogram view and optional baseline |
| `POST` | `/api/v1/histograms/baselines` | Create a non-destructive baseline |
| `GET` | `/metrics` | Prometheus text exposition |
| `WS` | `/api/v1/stream` | Rate-capped latest-product notifications |

WebSocket clients configure their subscription with:

```json
{
  "products": ["publisher.DATA.some-schema.0"],
  "maximum-rate-hz": 2
}
```

Notifications contain metadata, not large payloads. The client fetches only the
view it needs after an update.

## Histogram views and zeroing

The built-in `dqm-histogram-v1` decoder accepts JSON products shaped as:

```json
{
  "epoch": "run-42",
  "definition": "wfd5-integral-v1",
  "generated-at-unix-ns": 1790794319801012545,
  "histograms": [
    {
      "name": "wfd5/integral/ch4",
      "title": "Channel 4 integral",
      "units": "ADC",
      "sequence": 18,
      "edges": [0, 1, 2, 3],
      "counts": [10, 20, 30],
      "sumw2": [10, 20, 30]
    }
  ]
}
```

Create a display baseline without mutating Gaudi:

```sh
curl -X POST -H 'Content-Type: application/json' \
  --data '{"histogram":"wfd5/integral/ch4","name":"operator-zero"}' \
  http://127.0.0.1:8080/api/v1/histograms/baselines
```

Request the accumulated difference and at most 256 output bins:

```sh
curl 'http://127.0.0.1:8080/api/v1/histograms?name=wfd5%2Fintegral%2Fch4&baseline=operator-zero&max-bins=256'
```

Baselines cannot cross histogram definitions or epochs. `sumw2` accumulators
are subtracted before bin aggregation so errors can be reconstructed correctly.
Exact run/configuration boundaries must be identified by the producer; the
gateway does not guess them from arrival time.

## Waveform views

The built-in `dqm-waveform-v1` decoder accepts compact products from the Gaudi
online output. Select only what a plot needs and cap its samples:

```sh
curl 'http://127.0.0.1:8080/api/v1/waveforms?detector=wfd5&module=0&submodule=0&channel=0&max-samples=1024'
```

Sampled detectors include `sample-period-ns`.

## Hit-series views

MuSiP uses the distinct `dqm-hit-series-v1` product with equal-length
`times-ns` and `energies` arrays. Request a bounded time/energy series with:

```sh
curl 'http://127.0.0.1:8080/api/v1/hit-series?detector=musip&module=0&channel=0&max-hits=256'
```

For each detector channel, the gateway keeps a rolling history capped by
`maximum-hits-per-series`; a new epoch replaces the old history. Compact DQM
products may include `generated-at-unix-ns`. Decoded API views preserve it so
clients can distinguish producer time from HTTP fetch time.

The gateway never opens ROOT objects. ROOT archival and expert inspection
remain in the separate nearline pipeline.

## Resource controls

Configuration provides:

- per-client token-bucket request rate and burst;
- maximum response and incoming product sizes;
- bounded product, decoded-view, and baseline counts and retained bytes;
- hard histogram-bin and WebSocket update-rate limits;
- at most 128 product subscriptions per WebSocket;
- exact CORS and WebSocket origin allowlists.

The server binds to loopback by default. If deployed outside the beam-room
network, terminate TLS and authentication at a configured reverse proxy and do
not trust forwarded client-address headers unless trusted-proxy handling is
added explicitly.

The committed `config/config.json` is the normal runtime policy and already
allows the local React development server and MIDAS page on port 8081. Raw
capacity tests use the separately committed `config/benchmark.json`; generated
benchmark artifacts contain results, never gateway configuration.
