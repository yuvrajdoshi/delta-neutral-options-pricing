#include "instruments/Equity.h"
#include <stdexcept>

namespace instruments {

Equity::Equity(const std::string& symbol, double shares) 
    : symbol_(symbol), shares_(shares) {
    if (shares <= 0.0) {
        throw std::invalid_argument("Number of shares must be positive");
    }
}

double Equity::price(const core::MarketData& data) const {
    if (data.getSymbol() != symbol_) {
        throw std::invalid_argument("MarketData symbol does not match equity symbol");
    }
    return data.getClose() * shares_;
}

std::string Equity::getSymbol() const {
    return symbol_;
}

InstrumentType Equity::getType() const {
    return InstrumentType::EQUITY;
}

std::vector<double> Equity::calculateRiskMetrics(const core::MarketData& data) const {
    std::vector<double> metrics;
    
    // Basic risk metrics for equity:
    // 0: Current Value
    // 1: Daily P&L (assuming previous close was similar)
    // 2: Position Delta (always 1.0 for equity)
    // 3: Position Gamma (always 0.0 for equity)
    
    double currentValue = price(data);
    metrics.push_back(currentValue);                           // Current value (close * shares)
    metrics.push_back(currentValue - (shares_ * data.getOpen()));  // Intraday P&L: (close - open) * shares
    metrics.push_back(shares_);                                // Position delta (shares)
    metrics.push_back(0.0);                                   // Position gamma (0 for equity)
    
    return metrics;
}

std::unique_ptr<Instrument> Equity::clone() const {
    return std::make_unique<Equity>(symbol_, shares_);
}

double Equity::getShares() const {
    return shares_;
}

void Equity::setShares(double shares) {
    if (shares <= 0.0) {
        throw std::invalid_argument("Number of shares must be positive");
    }
    shares_ = shares;
}

} // namespace instruments
