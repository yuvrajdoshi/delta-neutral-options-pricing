#pragma once

#include "strategy/Signal.h"
#include "instruments/Instrument.h"
#include "models/VolatilityModel.h"
#include "core/MarketData.h"
#include <memory>

namespace VolatilityArbitrage {

class SignalGenerator {
public:
    // Virtual destructor
    virtual ~SignalGenerator() = default;
    
    // Core functionality
    virtual Signal generateSignal(
        const instruments::Instrument& instrument,
        const models::VolatilityModel& model,
        const core::MarketData& data) const = 0;
    
    // Clone pattern
    virtual std::unique_ptr<SignalGenerator> clone() const = 0;
};

} // namespace VolatilityArbitrage
