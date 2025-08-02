#include "models/BSMPricingModel.h"
#include "instruments/Option.h"
#include "core/MathUtils.h"
#include <cmath>
#include <stdexcept>

namespace models {

BSMPricingModel::BSMPricingModel() {
}

double BSMPricingModel::price(const instruments::Option& option, const core::MarketData& data) const {
    double S = data.getClose();
    double K = option.getStrike();
    double T = option.timeToExpiry(data.getTimestamp());
    double r = getRiskFreeRate();
    double sigma = getVolatility(option, data);
    
    if (T <= 0.0) {
        // Expired option - return intrinsic value
        if (option.getOptionType() == instruments::OptionType::Call) {
            return std::max(0.0, S - K);
        } else {
            return std::max(0.0, K - S);
        }
    }
    
    if (option.getOptionType() == instruments::OptionType::Call) {
        return calculateCallPrice(S, K, T, r, sigma);
    } else {
        return calculatePutPrice(S, K, T, r, sigma);
    }
}

Greeks BSMPricingModel::calculateGreeks(const instruments::Option& option, const core::MarketData& data) const {
    double S = data.getClose();
    double K = option.getStrike();
    double T = option.timeToExpiry(data.getTimestamp());
    double r = getRiskFreeRate();
    double sigma = getVolatility(option, data);
    
    if (T <= 0.0) {
        // Expired option - all Greeks are zero
        return Greeks(0.0, 0.0, 0.0, 0.0, 0.0);
    }
    
    bool isCall = (option.getOptionType() == instruments::OptionType::Call);
    
    double delta = calculateDelta(S, K, T, r, sigma, isCall);
    double gamma = calculateGamma(S, K, T, r, sigma);
    double vega = calculateVega(S, K, T, r, sigma);
    double theta = calculateTheta(S, K, T, r, sigma, isCall);
    double rho = calculateRho(S, K, T, r, sigma, isCall);
    
    return Greeks(delta, gamma, vega, theta, rho);
}

std::string BSMPricingModel::getModelName() const {
    return "Black-Scholes-Merton";
}

std::unique_ptr<PricingModel> BSMPricingModel::clone() const {
    return std::make_unique<BSMPricingModel>();
}

double BSMPricingModel::calculateD1(double S, double K, double T, double r, double sigma) const {
    if (sigma <= 0.0 || T <= 0.0) {
        return 0.0;
    }
    return (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
}

double BSMPricingModel::calculateD2(double d1, double sigma, double T) const {
    if (T <= 0.0) {
        return d1;
    }
    return d1 - sigma * std::sqrt(T);
}

double BSMPricingModel::calculateCallPrice(double S, double K, double T, double r, double sigma) const {
    if (sigma <= 0.0) return std::max(0.0, S - K * std::exp(-r * T));
    
    double d1 = calculateD1(S, K, T, r, sigma);
    double d2 = calculateD2(d1, sigma, T);
    
    return S * core::normalCDF(d1) - K * std::exp(-r * T) * core::normalCDF(d2);
}

double BSMPricingModel::calculatePutPrice(double S, double K, double T, double r, double sigma) const {
    if (sigma <= 0.0) return std::max(0.0, K * std::exp(-r * T) - S);
    
    double d1 = calculateD1(S, K, T, r, sigma);
    double d2 = calculateD2(d1, sigma, T);
    
    return K * std::exp(-r * T) * core::normalCDF(-d2) - S * core::normalCDF(-d1);
}

double BSMPricingModel::calculateDelta(double S, double K, double T, double r, double sigma, bool isCall) const {
    if (sigma <= 0.0 || T <= 0.0) return 0.0;
    
    double d1 = calculateD1(S, K, T, r, sigma);
    
    if (isCall) {
        return core::normalCDF(d1);
    } else {
        return core::normalCDF(d1) - 1.0;
    }
}

double BSMPricingModel::calculateGamma(double S, double K, double T, double r, double sigma) const {
    if (sigma <= 0.0 || T <= 0.0 || S <= 0.0) return 0.0;
    
    double d1 = calculateD1(S, K, T, r, sigma);
    return core::normalPDF(d1) / (S * sigma * std::sqrt(T));
}

double BSMPricingModel::calculateVega(double S, double K, double T, double r, double sigma) const {
    if (T <= 0.0 || S <= 0.0) return 0.0;
    
    double d1 = calculateD1(S, K, T, r, sigma);
    return S * core::normalPDF(d1) * std::sqrt(T) / 100.0; // Divide by 100 for 1% vol change
}

double BSMPricingModel::calculateTheta(double S, double K, double T, double r, double sigma, bool isCall) const {
    if (T <= 0.0 || S <= 0.0) return 0.0;
    
    double d1 = calculateD1(S, K, T, r, sigma);
    double d2 = calculateD2(d1, sigma, T);
    
    double term1 = -(S * core::normalPDF(d1) * sigma) / (2.0 * std::sqrt(T));
    
    if (isCall) {
        double term2 = r * K * std::exp(-r * T) * core::normalCDF(d2);
        return (term1 - term2) / 365.0; // Per day
    } else {
        double term2 = r * K * std::exp(-r * T) * core::normalCDF(-d2);
        return (term1 + term2) / 365.0; // Per day
    }
}

double BSMPricingModel::calculateRho(double S, double K, double T, double r, double sigma, bool isCall) const {
    if (T <= 0.0) return 0.0;
    
    double d2 = calculateD2(calculateD1(S, K, T, r, sigma), sigma, T);
    
    if (isCall) {
        return K * T * std::exp(-r * T) * core::normalCDF(d2) / 100.0; // For 1% rate change
    } else {
        return -K * T * std::exp(-r * T) * core::normalCDF(-d2) / 100.0; // For 1% rate change
    }
}

double BSMPricingModel::getVolatility(const instruments::Option& option, const core::MarketData& data) const {
    // First try to get implied volatility from market data
    if (data.hasAdditionalData("implied_volatility")) {
        double impliedVol = data.getAdditionalData("implied_volatility");
        if (impliedVol > 0.0 && impliedVol <= 3.0) { // Sanity check (0-300%)
            return impliedVol;
        }
    }
    
    // Default volatility if none available
    return 0.20; // 20% default
}

double BSMPricingModel::getRiskFreeRate() const {
    return 0.05; // 5% default risk-free rate
}

} // namespace models
