#ifndef DERIVATIVE_H
#define DERIVATIVE_H

#include "instruments/Instrument.h"
#include "core/DateTime.h"

namespace instruments {

/**
 * Abstract base class for derivative instruments
 */
class Derivative : public Instrument {
protected:
    std::string underlyingSymbol_;
    core::DateTime expiryDate_;
    
public:
    // Constructors
    Derivative(const std::string& underlyingSymbol, const core::DateTime& expiryDate);
    
    // Accessors
    std::string getUnderlyingSymbol() const;
    core::DateTime getExpiryDate() const;
    double timeToExpiry(const core::DateTime& currentDate) const;
    
    // Virtual methods for Greeks
    virtual double delta(const core::MarketData& data) const = 0;
    virtual double gamma(const core::MarketData& data) const = 0;
    virtual double vega(const core::MarketData& data) const = 0;
    virtual double theta(const core::MarketData& data) const = 0;
    virtual double rho(const core::MarketData& data) const = 0;
};

} // namespace instruments

#endif // DERIVATIVE_H
