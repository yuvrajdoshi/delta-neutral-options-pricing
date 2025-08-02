#pragma once

#include "strategy/Position.h"
#include "core/MarketData.h"
#include <vector>

namespace VolatilityArbitrage {

class Portfolio {
private:
    std::vector<Position> positions_;
    double cash_;
    
public:
    // Constructors
    Portfolio(double initialCash = 0.0);
    
    // Position management
    void addPosition(Position position);
    void removePosition(size_t index);
    void updatePosition(size_t index, double newQuantity);
    Position& getPosition(size_t index);
    const Position& getPosition(size_t index) const;
    size_t getPositionCount() const;
    
    // Cash management
    double getCash() const;
    void addCash(double amount);
    void removeCash(double amount);
    
    // Portfolio valuation
    double getTotalValue(const core::MarketData& data) const;
    double getTotalPnL(const core::MarketData& data) const;
    
    // Risk metrics
    double calculateDelta(const core::MarketData& data) const;
    double calculateGamma(const core::MarketData& data) const;
    double calculateVega(const core::MarketData& data) const;
    double calculateTheta(const core::MarketData& data) const;
};

} // namespace VolatilityArbitrage
