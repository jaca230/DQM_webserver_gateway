add_dqm_gateway_plugin(dqm_gateway_raw register_plugin.cpp
  product_store.cpp
  raw_product.cpp
  routes/product_json_route.cpp
  routes/product_metadata_route.cpp
  routes/product_payload_route.cpp)
dqm_gateway_enable_plugin_tests(dqm_gateway_raw)
