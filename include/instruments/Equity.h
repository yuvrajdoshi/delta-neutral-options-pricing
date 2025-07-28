#ifndef EQUITY_H
#define EQUITY_H

#include "instruments/Instrument.h"

namespace instruments {

/**
 * Equity instrument class
 */
class Equity : public Instrument {
private:
    std::string symbol_;
    double shares_;
    
public:
    // Constructors
    Equity(const std::string& symbol, double shares = 1.0);
    
    // Instrument interface implementation
    double price(const core::MarketData& data) const override;
    std::string getSymbol() const override;
    InstrumentType getType() const override;
    std::vector<double> calculateRiskMetrics(const core::MarketData& data) const override;
    std::unique_ptr<Instrument> clone() const override;
    
    // Equity-specific methods
    double getShares() const;
    void setShares(double shares);
};

} // namespace instruments

#endif // EQUITY_H
