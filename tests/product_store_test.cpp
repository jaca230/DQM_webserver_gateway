#include "dqm_gateway/product/raw/product_store.h"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace {

dqm_gateway::Product product(std::string id, std::size_t bytes) {
    dqm_gateway::Product result;
    result.id = std::move(id);
    result.payload = std::make_shared<const std::vector<std::byte>>(bytes);
    return result;
}

}  // namespace

TEST_CASE("Product store replaces a product without growing") {
    dqm_gateway::ProductStore store(100, 4);
    REQUIRE(store.put(product("one", 30)));
    REQUIRE(store.put(product("one", 20)));
    CHECK(store.size() == 1);
    CHECK(store.bytes() == 20);
}

TEST_CASE("Product store evicts the oldest latest-value product") {
    dqm_gateway::ProductStore store(50, 2);
    REQUIRE(store.put(product("one", 20)));
    REQUIRE(store.put(product("two", 20)));
    REQUIRE(store.put(product("three", 20)));
    CHECK_FALSE(store.get("one").has_value());
    CHECK(store.get("two").has_value());
    CHECK(store.get("three").has_value());
    CHECK(store.bytes() == 40);
}

