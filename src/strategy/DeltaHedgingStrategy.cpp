#include "strategy/DeltaHedgingStrategy.h"
#include "instruments/InstrumentFactory.h"
#include <cmath>

namespace VolatilityArbitrage {

DeltaHedgingStrategy::DeltaHedgingStrategy(double targetDelta, double tolerance)
    : targetDelta_(targetDelta), tolerance_(tolerance) {
}

void DeltaHedgingStrategy::applyHedge(Portfolio& portfolio, const core::MarketData& data) const {
    // Calculate current portfolio delta
    double currentDelta = portfolio.calculateDelta(data);
    double deltaGap = currentDelta - targetDelta_;
    
    // Check if hedge is needed
    if (std::abs(deltaGap) <= tolerance_) {
        return; // No hedging needed
    }
    
    // Calculate required hedge quantity (negative of delta gap)
    double hedgeQuantity = -deltaGap;
    
    // Create equity position for hedging (assuming we hedge with the underlying)
    auto hedgeInstrument = instruments::InstrumentFactory::createEquity(
        data.getSymbol(),      // Use the symbol from market data
        1.0                    // Default shares
    );
    
    // Find existing hedge position (equity position)
    size_t hedgePositionIndex = portfolio.getPositionCount(); // Default to new position
    for (size_t i = 0; i < portfolio.getPositionCount(); ++i) {
        const Position& pos = portfolio.getPosition(i);
        if (pos.getInstrument().getType() == instruments::InstrumentType::EQUITY && 
            pos.getInstrument().getSymbol() == data.getSymbol()) {
            hedgePositionIndex = i;
            break;
        }
    }
    
    if (hedgePositionIndex < portfolio.getPositionCount()) {
        // Update existing hedge position
        Position& hedgePosition = portfolio.getPosition(hedgePositionIndex);
        double newQuantity = hedgePosition.getQuantity() + hedgeQuantity;
        
        if (std::abs(newQuantity) < 1e-6) {
            // Close position if quantity becomes very small
            portfolio.removePosition(hedgePositionIndex);
        } else {
            hedgePosition.setQuantity(newQuantity);
        }
    } else {
        // Create new hedge position if hedge quantity is significant
        if (std::abs(hedgeQuantity) >= 1e-6) {
            Position hedgePosition(
                std::move(hedgeInstrument),
                hedgeQuantity,
                data.getClose(),
                data.getTimestamp()
            );
            
            // Set metadata to identify as hedge position
            hedgePosition.setMetadata("is_hedge", 1.0);
            hedgePosition.setMetadata("target_delta", targetDelta_);
            
            portfolio.addPosition(std::move(hedgePosition));
            
            // Update cash position
            portfolio.removeCash(hedgeQuantity * data.getClose());
        }
    }
}

std::unique_ptr<HedgingStrategy> DeltaHedgingStrategy::clone() const {
    return std::make_unique<DeltaHedgingStrategy>(*this);
}

double DeltaHedgingStrategy::getTargetDelta() const {
    return targetDelta_;
}

void DeltaHedgingStrategy::setTargetDelta(double targetDelta) {
    targetDelta_ = targetDelta;
}

double DeltaHedgingStrategy::getTolerance() const {
    return tolerance_;
}

void DeltaHedgingStrategy::setTolerance(double tolerance) {
    tolerance_ = tolerance;
}

} // namespace VolatilityArbitrage
