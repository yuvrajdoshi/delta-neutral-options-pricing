#pragma once

#include "strategy/Portfolio.h"
#include "core/MarketData.h"
#include <memory>

namespace VolatilityArbitrage {

class HedgingStrategy {
public:
    // Virtual destructor
    virtual ~HedgingStrategy() = default;
    
    // Core functionality
    virtual void applyHedge(Portfolio& portfolio, const core::MarketData& data) const = 0;
    
    // Clone pattern
    virtual std::unique_ptr<HedgingStrategy> clone() const = 0;
};

} // namespace VolatilityArbitrage
