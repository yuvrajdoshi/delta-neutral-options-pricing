#pragma once

#include "strategy/SignalGenerator.h"

namespace VolatilityArbitrage {

class VolatilitySpreadSignal : public SignalGenerator {
private:
    double entryThreshold_;
    double exitThreshold_;
    
public:
    // Constructors
    VolatilitySpreadSignal(double entryThreshold = 0.1, double exitThreshold = 0.05);
    
    // SignalGenerator interface implementation
    Signal generateSignal(
        const instruments::Instrument& instrument,
        const models::VolatilityModel& model,
        const core::MarketData& data) const override;
    std::unique_ptr<SignalGenerator> clone() const override;
    
    // Accessors and modifiers
    double getEntryThreshold() const;
    void setEntryThreshold(double threshold);
    double getExitThreshold() const;
    void setExitThreshold(double threshold);
};

} // namespace VolatilityArbitrage
