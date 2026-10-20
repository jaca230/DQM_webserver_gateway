#pragma once

#include "dqm_gateway/api/route/data/data_route.h"

namespace dqm_gateway {
class AtarStore;
class AtarRoute final : public DataRoute {
public:
    explicit AtarRoute(AtarStore& store);
    void register_route(ApiContext& context) override;
private:
    AtarStore& atar_;
};
}  // namespace dqm_gateway
