#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dqm_gateway {

template <typename Factory>
class FactoryRegistry {
public:
    explicit FactoryRegistry(std::string factory_kind)
        : factory_kind_(std::move(factory_kind)) {}

    void add(std::string type, Factory factory) {
        if (type.empty() || !factory) {
            throw std::invalid_argument("Invalid " + factory_kind_ + " factory");
        }
        const auto name = type;
        if (!factories_.emplace(std::move(type), std::move(factory)).second) {
            throw std::invalid_argument("Duplicate " + factory_kind_ + " factory: " + name);
        }
        registration_order_.push_back(name);
    }

protected:
    [[nodiscard]] const Factory* find_factory(const std::string& type) const noexcept {
        const auto found = factories_.find(type);
        return found == factories_.end() ? nullptr : &found->second;
    }

    [[nodiscard]] const Factory& factory(const std::string& type) const {
        const auto* found = find_factory(type);
        if (!found) throw std::invalid_argument("Unknown " + factory_kind_ + " type: " + type);
        return *found;
    }

    [[nodiscard]] const std::vector<std::string>& registration_order() const noexcept {
        return registration_order_;
    }

private:
    std::string factory_kind_;
    std::vector<std::string> registration_order_;
    std::unordered_map<std::string, Factory> factories_;
};

}  // namespace dqm_gateway
