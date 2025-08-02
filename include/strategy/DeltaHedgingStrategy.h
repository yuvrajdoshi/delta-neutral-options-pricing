#pragma once

#include "strategy/HedgingStrategy.h"

namespace VolatilityArbitrage {

class DeltaHedgingStrategy : public HedgingStrategy {
private:
    double targetDelta_;
    double tolerance_;
    
public:
    // Constructors
    DeltaHedgingStrategy(double targetDelta = 0.0, double tolerance = 0.01);
    
    // HedgingStrategy interface implementation
    void applyHedge(Portfolio& portfolio, const core::MarketData& data) const override;
    std::unique_ptr<HedgingStrategy> clone() const override;
    
    // Accessors and modifiers
    double getTargetDelta() const;
    void setTargetDelta(double targetDelta);
    double getTolerance() const;
    void setTolerance(double tolerance);
};

} // namespace VolatilityArbitrage
