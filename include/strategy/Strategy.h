#pragma once

#include "strategy/BacktestParameters.h"
#include "strategy/Portfolio.h"
#include "core/MarketData.h"
#include <memory>

namespace VolatilityArbitrage {

class Strategy {
public:
    // Virtual destructor
    virtual ~Strategy() = default;
    
    // Core functionality
    virtual void initialize(const BacktestParameters& params) = 0;
    virtual void processBar(const core::MarketData& data) = 0;
    virtual Portfolio getPortfolio() const = 0;
    
    // Clone pattern
    virtual std::unique_ptr<Strategy> clone() const = 0;
};

} // namespace VolatilityArbitrage
