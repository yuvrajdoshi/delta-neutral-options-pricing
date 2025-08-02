#include "strategy/VolatilitySpreadSignal.h"
#include "instruments/Option.h"
#include <cmath>

namespace VolatilityArbitrage {

VolatilitySpreadSignal::VolatilitySpreadSignal(double entryThreshold, double exitThreshold)
    : entryThreshold_(entryThreshold), exitThreshold_(exitThreshold) {
}

Signal VolatilitySpreadSignal::generateSignal(
    const instruments::Instrument& instrument,
    const models::VolatilityModel& model,
    const core::MarketData& data) const {
    
    // Try to cast to Option to get implied volatility
    const instruments::Option* option = dynamic_cast<const instruments::Option*>(&instrument);
    if (!option) {
        // Not an option, generate HOLD signal
        return Signal(Signal::Type::HOLD, 0.0, instrument.getSymbol(), data.getTimestamp());
    }
    
    // Get implied volatility from market data
    if (!data.hasAdditionalData("implied_volatility")) {
        // No implied volatility available, generate HOLD signal
        return Signal(Signal::Type::HOLD, 0.0, instrument.getSymbol(), data.getTimestamp());
    }
    
    double impliedVol = data.getAdditionalData("implied_volatility");
    if (impliedVol <= 0.0) {
        // Invalid implied volatility, generate HOLD signal
        return Signal(Signal::Type::HOLD, 0.0, instrument.getSymbol(), data.getTimestamp());
    }
    
    // Get forecasted volatility from model
    double forecastedVol = model.forecast(1); // 1-day ahead forecast
    
    // Calculate volatility spread (implied - forecasted)
    double volSpread = impliedVol - forecastedVol;
    double spreadMagnitude = std::abs(volSpread);
    
    Signal::Type signalType = Signal::Type::HOLD;
    double strength = 0.0;
    
    if (spreadMagnitude >= entryThreshold_) {
        if (volSpread > 0) {
            // Implied vol > forecasted vol -> options are overpriced -> SELL
            signalType = Signal::Type::SELL;
        } else {
            // Implied vol < forecasted vol -> options are underpriced -> BUY
            signalType = Signal::Type::BUY;
        }
        strength = spreadMagnitude;
    } else if (spreadMagnitude <= exitThreshold_) {
        // Close to fair value, consider exit signal
        signalType = Signal::Type::HOLD;
        strength = 0.0;
    }
    
    Signal signal(signalType, strength, instrument.getSymbol(), data.getTimestamp());
    
    // Add metadata
    signal.metadata["implied_vol"] = impliedVol;
    signal.metadata["forecasted_vol"] = forecastedVol;
    signal.metadata["vol_spread"] = volSpread;
    signal.metadata["spread_magnitude"] = spreadMagnitude;
    
    return signal;
}

std::unique_ptr<SignalGenerator> VolatilitySpreadSignal::clone() const {
    return std::make_unique<VolatilitySpreadSignal>(*this);
}

double VolatilitySpreadSignal::getEntryThreshold() const {
    return entryThreshold_;
}

void VolatilitySpreadSignal::setEntryThreshold(double threshold) {
    entryThreshold_ = threshold;
}

double VolatilitySpreadSignal::getExitThreshold() const {
    return exitThreshold_;
}

void VolatilitySpreadSignal::setExitThreshold(double threshold) {
    exitThreshold_ = threshold;
}

} // namespace VolatilityArbitrage
