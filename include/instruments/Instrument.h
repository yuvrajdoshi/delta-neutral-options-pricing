#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "core/MarketData.h"
#include <vector>
#include <string>
#include <memory>

namespace instruments {

// Enum for instrument types
enum class InstrumentType {
    EQUITY,
    EUROPEAN_OPTION,
    AMERICAN_OPTION,
    FUTURE,
    BOND
};

/**
 * Abstract base class for all financial instruments
 */
class Instrument {
public:
    // Virtual destructor
    virtual ~Instrument() = default;
    
    // Core functionality
    virtual double price(const core::MarketData& data) const = 0;
    virtual std::string getSymbol() const = 0;
    virtual InstrumentType getType() const = 0;
    
    // Risk metrics
    virtual std::vector<double> calculateRiskMetrics(const core::MarketData& data) const = 0;
    
    // Clone pattern
    virtual std::unique_ptr<Instrument> clone() const = 0;
};

} // namespace instruments

#endif // INSTRUMENT_H
