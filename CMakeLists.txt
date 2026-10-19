cmake_minimum_required(VERSION 3.18)
project(dqm_gateway VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

option(DQM_GATEWAY_BUILD_TESTS "Build unit tests" ON)
set(BUILD_TESTING OFF CACHE BOOL "Do not build dependency test suites" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(BUILD_CTL OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_ORM OFF CACHE BOOL "" FORCE)
set(BUILD_REDIS OFF CACHE BOOL "" FORCE)
set(BUILD_POSTGRESQL OFF CACHE BOOL "" FORCE)
set(BUILD_MYSQL OFF CACHE BOOL "" FORCE)
set(BUILD_SQLITE OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_EXAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  jsoncpp
  GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
  GIT_TAG 1.9.6
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(jsoncpp)
set(JSONCPP_INCLUDE_DIRS "${jsoncpp_SOURCE_DIR}/include" CACHE PATH "" FORCE)
set(JSONCPP_LIBRARIES jsoncpp_static CACHE STRING "" FORCE)
FetchContent_Declare(
  drogon
  GIT_REPOSITORY https://github.com/drogonframework/drogon.git
  GIT_TAG v1.9.13
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(drogon)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(Threads REQUIRED)
find_library(ZMQ_LIBRARY NAMES zmq REQUIRED)
find_path(CPPZMQ_INCLUDE_DIR zmq.hpp REQUIRED)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Some locally installed libzmq builds expose strlcpy through libbsd but do
  # not propagate that dependency when linked by Conda's linker.
  unset(BSD_LIBRARY CACHE)
  find_library(BSD_LIBRARY NAMES bsd
    PATHS "/lib/${CMAKE_LIBRARY_ARCHITECTURE}" "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
    NO_DEFAULT_PATH REQUIRED)
endif()

add_library(dqm_gateway_core SHARED
  src/api/client_rate_limiter.cpp
  src/api/context/api_context.cpp
  src/api/route/api_route_registry.cpp
  src/api/route/data/data_route.cpp
  src/api/route/system/catalog_route.cpp
  src/api/route/system/health_route.cpp
  src/api/route/system/metrics_route.cpp
  src/api/route/system/register_system_routes.cpp
  src/config/config_section_registry.cpp
  src/config/gateway_config.cpp
  src/config/sections/data_source_config.cpp
  src/config/sections/limit_config.cpp
  src/config/sections/server_config.cpp
  src/config/sections/store_config.cpp
  src/input/data_source_factory_registry.cpp
  src/input/data_source_registry.cpp
  src/metrics/gateway_metrics.cpp
  src/model/product.cpp
  src/plugin/plugin_loader.cpp
  src/product/data_product_factory_registry.cpp
  src/product/data_product_registry.cpp
)
target_include_directories(dqm_gateway_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(dqm_gateway_core
  PUBLIC Drogon::Drogon nlohmann_json::nlohmann_json
  PRIVATE Threads::Threads ${CMAKE_DL_LIBS}
)
target_compile_options(dqm_gateway_core PRIVATE -Wall -Wextra -Wpedantic)

include(cmake/AddDqmGatewayPlugin.cmake)
foreach(PLUGIN_ROOT src/product src/input)
  file(GLOB PLUGIN_CONFIGS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_ROOT}/*/CMakeLists.txt")
  foreach(PLUGIN_CONFIG IN LISTS PLUGIN_CONFIGS)
    get_filename_component(PLUGIN_DIRECTORY "${PLUGIN_CONFIG}" DIRECTORY)
    file(RELATIVE_PATH PLUGIN_RELATIVE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/src" "${PLUGIN_DIRECTORY}")
    add_subdirectory(
      "${PLUGIN_DIRECTORY}"
      "${CMAKE_CURRENT_BINARY_DIR}/plugin-build/${PLUGIN_RELATIVE_PATH}")
  endforeach()
endforeach()

add_executable(dqm_gateway
  src/api/http_api.cpp
  src/api/live_websocket_controller.cpp
  src/api/websocket_hub.cpp
  src/main.cpp
)
target_link_libraries(dqm_gateway PRIVATE dqm_gateway_core Drogon::Drogon)
target_compile_options(dqm_gateway PRIVATE -Wall -Wextra -Wpedantic)

if(DQM_GATEWAY_BUILD_TESTS)
  enable_testing()
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(Catch2)
  add_executable(dqm_gateway_tests
    tests/client_rate_limiter_test.cpp
    tests/gateway_config_test.cpp
    tests/hit_series_store_test.cpp
    tests/histogram_store_test.cpp
    tests/product_store_test.cpp
    tests/waveform_store_test.cpp
  )
  get_property(DQM_GATEWAY_TEST_PLUGIN_TARGETS GLOBAL PROPERTY
    DQM_GATEWAY_TEST_PLUGIN_TARGETS)
  target_link_libraries(dqm_gateway_tests PRIVATE
    dqm_gateway_core
    ${DQM_GATEWAY_TEST_PLUGIN_TARGETS}
    Catch2::Catch2WithMain)
  include(Catch)
  catch_discover_tests(dqm_gateway_tests)
endif()
