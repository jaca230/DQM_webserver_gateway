add_dqm_gateway_plugin(dqm_gateway_hit_series register_plugin.cpp
  hit_series_json_decoder.cpp
  hit_series_product.cpp
  hit_series_store.cpp
  routes/hit_series_route.cpp)
dqm_gateway_enable_plugin_tests(dqm_gateway_hit_series)
