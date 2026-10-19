add_dqm_gateway_plugin(dqm_gateway_waveform register_plugin.cpp
  waveform_json_decoder.cpp
  waveform_product.cpp
  waveform_store.cpp
  routes/waveform_route.cpp)
dqm_gateway_enable_plugin_tests(dqm_gateway_waveform)
