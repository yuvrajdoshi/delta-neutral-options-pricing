#pragma once

#include "core/DateTime.h"
#include <string>
#include <map>

namespace VolatilityArbitrage {

class Signal {
public:
    enum class Type { BUY, SELL, HOLD };
    
    // Members
    Type type;
    double strength;
    std::string instrumentId;
    core::DateTime timestamp;
    std::map<std::string, double> metadata;
    
    // Constructors
    Signal();
    Signal(Type type, double strength, const std::string& instrumentId, const core::DateTime& timestamp);
    
    // Methods
    bool isActionable() const;
    std::string toString() const;
};

} // namespace VolatilityArbitrage
