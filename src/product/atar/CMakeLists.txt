add_dqm_gateway_plugin(dqm_gateway_atar register_plugin.cpp
  atar_json_decoder.cpp
  atar_product.cpp
  atar_store.cpp
  routes/atar_route.cpp)
dqm_gateway_enable_plugin_tests(dqm_gateway_atar)
