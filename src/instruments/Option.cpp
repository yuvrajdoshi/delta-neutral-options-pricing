#include "instruments/Option.h"
#include "core/MathUtils.h"
#include "core/VolatilityEstimator.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace instruments {

// Option base class implementation
Option::Option(const std::string& underlyingSymbol, const core::DateTime& expiryDate,
               double strike, OptionType type, ExerciseStyle exerciseStyle)
    : Derivative(underlyingSymbol, expiryDate), strike_(strike), type_(type), exerciseStyle_(exerciseStyle) {
    if (strike <= 0.0) {
        throw std::invalid_argument("Strike price must be positive");
    }
}

// Helper method to get volatility with fallback logic
double Option::getVolatilityForPricing(const core::MarketData& data) const {
    // First, try to get implied volatility from market data
    if (data.hasAdditionalData("implied_volatility")) {
        double impliedVol = data.getAdditionalData("implied_volatility");
        if (impliedVol > 0.0 && impliedVol <= 3.0) { // Sanity check (0-300%)
            return impliedVol;
        }
    }
    
    // If no implied vol available, we need historical prices to estimate
    // For now, return a reasonable default
    // In a real implementation, you would pass historical prices to this method
    return 0.20; // 20% default volatility
}

double Option::getStrike() const {
    return strike_;
}

OptionType Option::getOptionType() const {
    return type_;
}

ExerciseStyle Option::getExerciseStyle() const {
    return exerciseStyle_;
}

std::string Option::getSymbol() const {
    std::stringstream ss;
    ss << underlyingSymbol_ << "_";
    ss << (type_ == OptionType::Call ? "C" : "P");
    ss << "_" << static_cast<int>(strike_);
    ss << "_" << expiryDate_.toString("%Y%m%d");
    return ss.str();
}

InstrumentType Option::getType() const {
    return (exerciseStyle_ == ExerciseStyle::European) ? 
           InstrumentType::EUROPEAN_OPTION : InstrumentType::AMERICAN_OPTION;
}

std::vector<double> Option::calculateRiskMetrics(const core::MarketData& data) const {
    std::vector<double> metrics;
    
    // Basic risk metrics for options:
    // 0: Current Value (option price)
    // 1: Delta
    // 2: Gamma  
    // 3: Vega
    // 4: Theta
    // 5: Rho
    
    metrics.push_back(price(data));
    metrics.push_back(delta(data));
    metrics.push_back(gamma(data));
    metrics.push_back(vega(data));
    metrics.push_back(theta(data));
    metrics.push_back(rho(data));
    
    return metrics;
}

// Helper function for Black-Scholes calculations
namespace {
    double blackScholesD1(double S, double K, double T, double r, double sigma) {
        if (T <= 0.0 || sigma <= 0.0) return 0.0;
        return (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    }
    
    double blackScholesD2(double S, double K, double T, double r, double sigma) {
        return blackScholesD1(S, K, T, r, sigma) - sigma * std::sqrt(T);
    }
}

double Option::delta(const core::MarketData& data) const {
    // Get market data parameters
    double S = data.getClose(); // Underlying price
    double K = strike_;
    
    // Default values - in practice these would come from market data or configuration
    double T = timeToExpiry(data.getTimestamp());
    double r = 0.05; // Risk-free rate (5%)
    double sigma = getVolatilityForPricing(data); // Use helper method for volatility
    
    // Check if we have implied volatility in additional data
    if (data.hasAdditionalData("implied_volatility")) {
        sigma = data.getAdditionalData("implied_volatility");
    }
    
    if (T <= 0.0) return 0.0; // Expired option
    
    double d1 = blackScholesD1(S, K, T, r, sigma);
    
    if (type_ == OptionType::Call) {
        return core::normalCDF(d1);
    } else {
        return core::normalCDF(d1) - 1.0;
    }
}

double Option::gamma(const core::MarketData& data) const {
    double S = data.getClose();
    double K = strike_;
    double T = timeToExpiry(data.getTimestamp());
    double r = 0.05;
    double sigma = getVolatilityForPricing(data);
    
    if (T <= 0.0 || sigma <= 0.0) return 0.0;
    
    double d1 = blackScholesD1(S, K, T, r, sigma);
    return core::normalPDF(d1) / (S * sigma * std::sqrt(T));
}

double Option::vega(const core::MarketData& data) const {
    double S = data.getClose();
    double K = strike_;
    double T = timeToExpiry(data.getTimestamp());
    double r = 0.05;
    double sigma = getVolatilityForPricing(data);
    
    if (T <= 0.0) return 0.0;
    
    double d1 = blackScholesD1(S, K, T, r, sigma);
    return S * core::normalPDF(d1) * std::sqrt(T) / 100.0; // Divide by 100 for 1% move
}

double Option::theta(const core::MarketData& data) const {
    double S = data.getClose();
    double K = strike_;
    double T = timeToExpiry(data.getTimestamp());
    double r = 0.05;
    double sigma = getVolatilityForPricing(data);
    
    if (T <= 0.0) return 0.0;
    
    double d1 = blackScholesD1(S, K, T, r, sigma);
    double d2 = blackScholesD2(S, K, T, r, sigma);
    
    double term1 = -(S * core::normalPDF(d1) * sigma) / (2.0 * std::sqrt(T));
    
    if (type_ == OptionType::Call) {
        double term2 = r * K * std::exp(-r * T) * core::normalCDF(d2);
        return (term1 - term2) / 365.0; // Per day
    } else {
        double term2 = r * K * std::exp(-r * T) * core::normalCDF(-d2);
        return (term1 + term2) / 365.0; // Per day
    }
}

double Option::rho(const core::MarketData& data) const {
    double S = data.getClose();
    double K = strike_;
    double T = timeToExpiry(data.getTimestamp());
    double r = 0.05;
    double sigma = getVolatilityForPricing(data);
    
    if (T <= 0.0) return 0.0;
    
    double d2 = blackScholesD2(S, K, T, r, sigma);
    
    if (type_ == OptionType::Call) {
        return K * T * std::exp(-r * T) * core::normalCDF(d2) / 100.0; // For 1% rate change
    } else {
        return -K * T * std::exp(-r * T) * core::normalCDF(-d2) / 100.0; // For 1% rate change
    }
}

// EuropeanOption implementation
EuropeanOption::EuropeanOption(const std::string& underlyingSymbol, const core::DateTime& expiryDate,
                              double strike, OptionType type)
    : Option(underlyingSymbol, expiryDate, strike, type, ExerciseStyle::European) {
}

double EuropeanOption::price(const core::MarketData& data) const {
    double S = data.getClose(); // Underlying price
    double K = strike_;
    double T = timeToExpiry(data.getTimestamp());
    double r = 0.05; // Risk-free rate
    double sigma = getVolatilityForPricing(data); // Use helper method for volatility
    
    // Use implied volatility if available
    if (data.hasAdditionalData("implied_volatility")) {
        sigma = data.getAdditionalData("implied_volatility");
    }
    
    if (T <= 0.0) {
        // Expired option - return intrinsic value
        if (type_ == OptionType::Call) {
            return std::max(0.0, S - K);
        } else {
            return std::max(0.0, K - S);
        }
    }
    
    if (sigma <= 0.0) return 0.0;
    
    // Black-Scholes formula
    double d1 = blackScholesD1(S, K, T, r, sigma);
    double d2 = blackScholesD2(S, K, T, r, sigma);
    
    if (type_ == OptionType::Call) {
        return S * core::normalCDF(d1) - K * std::exp(-r * T) * core::normalCDF(d2);
    } else {
        return K * std::exp(-r * T) * core::normalCDF(-d2) - S * core::normalCDF(-d1);
    }
}

std::unique_ptr<Instrument> EuropeanOption::clone() const {
    return std::make_unique<EuropeanOption>(underlyingSymbol_, expiryDate_, strike_, type_);
}

// AmericanOption implementation
AmericanOption::AmericanOption(const std::string& underlyingSymbol, const core::DateTime& expiryDate,
                              double strike, OptionType type)
    : Option(underlyingSymbol, expiryDate, strike, type, ExerciseStyle::American) {
}

double AmericanOption::price(const core::MarketData& data) const {
    // Simplified American option pricing - using European as approximation
    // In practice, you'd use binomial trees, Monte Carlo, or finite difference methods
    
    double europeanPrice = EuropeanOption(underlyingSymbol_, expiryDate_, strike_, type_).price(data);
    
    // Calculate intrinsic value
    double S = data.getClose();
    double intrinsicValue;
    if (type_ == OptionType::Call) {
        intrinsicValue = std::max(0.0, S - strike_);
    } else {
        intrinsicValue = std::max(0.0, strike_ - S);
    }
    
    // American option value is at least the European value and intrinsic value
    return std::max(europeanPrice, intrinsicValue);
}

std::unique_ptr<Instrument> AmericanOption::clone() const {
    return std::make_unique<AmericanOption>(underlyingSymbol_, expiryDate_, strike_, type_);
}

} // namespace instruments
