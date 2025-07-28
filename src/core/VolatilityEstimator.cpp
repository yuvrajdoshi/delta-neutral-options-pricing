#include "core/VolatilityEstimator.h"
#include "core/Math.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace core {

double VolatilityEstimator::calculateHistoricalVolatility(
    const TimeSeries& prices, 
    int lookbackDays,
    bool annualized) {
    
    if (prices.size() < 2) {
        throw std::invalid_argument("Need at least 2 price points for volatility calculation");
    }
    
    // Get the last 'lookbackDays' of data
    size_t startIdx = (prices.size() > static_cast<size_t>(lookbackDays)) ? 
                      (prices.size() - lookbackDays) : 0;
    
    TimeSeries recentPrices = prices.getSubseries(startIdx, prices.size() - 1);
    
    // Calculate log returns
    TimeSeries logReturns = recentPrices.logReturn();
    
    if (logReturns.empty()) {
        throw std::runtime_error("Unable to calculate log returns");
    }
    
    // Calculate standard deviation of returns
    double volatility = logReturns.standardDeviation();
    
    // Annualize if requested
    if (annualized) {
        volatility *= std::sqrt(TRADING_DAYS_PER_YEAR);
    }
    
    // Apply bounds
    volatility = std::max(DEFAULT_MIN_VOLATILITY, volatility);
    volatility = std::min(DEFAULT_MAX_VOLATILITY, volatility);
    
    return volatility;
}

double VolatilityEstimator::calculateEWMAVolatility(
    const TimeSeries& prices,
    double lambda,
    bool annualized) {
    
    if (prices.size() < 2) {
        throw std::invalid_argument("Need at least 2 price points for EWMA volatility");
    }
    
    if (lambda < 0.0 || lambda > 1.0) {
        throw std::invalid_argument("Lambda must be between 0 and 1");
    }
    
    // Calculate log returns
    TimeSeries logReturns = prices.logReturn();
    std::vector<double> returns = logReturns.getValues();
    
    if (returns.empty()) {
        throw std::runtime_error("Unable to calculate log returns for EWMA");
    }
    
    // Initialize with first squared return
    double ewmaVariance = returns[0] * returns[0];
    
    // Calculate EWMA variance
    for (size_t i = 1; i < returns.size(); ++i) {
        double returnSquared = returns[i] * returns[i];
        ewmaVariance = lambda * ewmaVariance + (1.0 - lambda) * returnSquared;
    }
    
    double volatility = std::sqrt(ewmaVariance);
    
    // Annualize if requested
    if (annualized) {
        volatility *= std::sqrt(TRADING_DAYS_PER_YEAR);
    }
    
    // Apply bounds
    volatility = std::max(DEFAULT_MIN_VOLATILITY, volatility);
    volatility = std::min(DEFAULT_MAX_VOLATILITY, volatility);
    
    return volatility;
}

double VolatilityEstimator::getVolatility(
    const MarketData& marketData,
    const TimeSeries& historicalPrices,
    VolatilityMethod method,
    int lookbackDays) {
    
    // First, try to get implied volatility from market data
    if (method == VolatilityMethod::IMPLIED_FALLBACK || 
        method == VolatilityMethod::IMPLIED_FALLBACK) {
        
        if (marketData.hasAdditionalData("implied_volatility")) {
            double impliedVol = marketData.getAdditionalData("implied_volatility");
            if (impliedVol > 0 && impliedVol <= DEFAULT_MAX_VOLATILITY) {
                return impliedVol;
            }
        }
        
        // If implied vol not available or invalid, fall back to historical
        if (method == VolatilityMethod::IMPLIED_FALLBACK) {
            method = VolatilityMethod::HISTORICAL;
        }
    }
    
    // Calculate based on chosen method
    switch (method) {
        case VolatilityMethod::HISTORICAL:
            return calculateHistoricalVolatility(historicalPrices, lookbackDays, true);
            
        case VolatilityMethod::EWMA:
            return calculateEWMAVolatility(historicalPrices, 0.94, true);
            
        case VolatilityMethod::GARCH:
            // For now, fall back to EWMA (GARCH would require more complex implementation)
            return calculateEWMAVolatility(historicalPrices, 0.94, true);
            
        default:
            return calculateHistoricalVolatility(historicalPrices, lookbackDays, true);
    }
}

double VolatilityEstimator::estimateImpliedVolatility(
    const std::string& symbol,
    const TimeSeries& underlyingPrices,
    double timeToExpiry,
    VolatilityMethod method) {
    
    // Calculate base volatility
    double baseVolatility;
    
    switch (method) {
        case VolatilityMethod::HISTORICAL:
            baseVolatility = calculateHistoricalVolatility(underlyingPrices, 30, true);
            break;
            
        case VolatilityMethod::EWMA:
            baseVolatility = calculateEWMAVolatility(underlyingPrices, 0.94, true);
            break;
            
        default:
            baseVolatility = calculateHistoricalVolatility(underlyingPrices, 30, true);
            break;
    }
    
    // Adjust for time to expiry (volatility often increases closer to expiry)
    double timeAdjustment = 1.0;
    if (timeToExpiry < 0.1) { // Less than ~36 days
        timeAdjustment = 1.0 + (0.1 - timeToExpiry) * 0.5; // Up to 5% increase
    }
    
    double estimatedVolatility = baseVolatility * timeAdjustment;
    
    // Apply bounds
    estimatedVolatility = std::max(DEFAULT_MIN_VOLATILITY, estimatedVolatility);
    estimatedVolatility = std::min(DEFAULT_MAX_VOLATILITY, estimatedVolatility);
    
    return estimatedVolatility;
}

} // namespace core
