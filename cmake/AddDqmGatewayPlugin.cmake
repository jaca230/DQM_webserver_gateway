function(add_dqm_gateway_plugin TARGET REGISTRATION_SOURCE)
  add_library(${TARGET}_objects OBJECT ${ARGN})
  target_include_directories(${TARGET}_objects
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
  target_link_libraries(${TARGET}_objects PUBLIC dqm_gateway_core)
  target_compile_options(${TARGET}_objects PRIVATE -Wall -Wextra -Wpedantic)

  add_library(${TARGET} MODULE ${REGISTRATION_SOURCE})
  target_link_libraries(${TARGET} PRIVATE ${TARGET}_objects)
  target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
  set_target_properties(${TARGET} PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")
endfunction()

function(dqm_gateway_enable_plugin_tests TARGET)
  set_property(GLOBAL APPEND PROPERTY
    DQM_GATEWAY_TEST_PLUGIN_TARGETS ${TARGET}_objects)
endfunction()
