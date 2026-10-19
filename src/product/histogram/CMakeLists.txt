add_dqm_gateway_plugin(dqm_gateway_histogram register_plugin.cpp
  histogram_json_decoder.cpp
  histogram_product.cpp
  histogram_store.cpp
  routes/histogram_baseline_route.cpp
  routes/histogram_route.cpp)
dqm_gateway_enable_plugin_tests(dqm_gateway_histogram)
