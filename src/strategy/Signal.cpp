#include "strategy/Signal.h"
#include <sstream>

namespace VolatilityArbitrage {

Signal::Signal() 
    : type(Type::HOLD), strength(0.0), instrumentId(""), timestamp() {
}

Signal::Signal(Type type, double strength, const std::string& instrumentId, const core::DateTime& timestamp)
    : type(type), strength(strength), instrumentId(instrumentId), timestamp(timestamp) {
}

bool Signal::isActionable() const {
    return type != Type::HOLD && strength > 0.0;
}

std::string Signal::toString() const {
    std::ostringstream oss;
    std::string typeStr;
    
    switch (type) {
        case Type::BUY:
            typeStr = "BUY";
            break;
        case Type::SELL:
            typeStr = "SELL";
            break;
        case Type::HOLD:
            typeStr = "HOLD";
            break;
    }
    
    oss << "Signal[" << typeStr << ", " << strength << ", " << instrumentId 
        << ", " << timestamp.toString() << "]";
    
    return oss.str();
}

} // namespace VolatilityArbitrage
